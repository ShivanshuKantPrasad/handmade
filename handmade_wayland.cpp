#include "xdg-shell-client-protocol.h"
#include <cstdint>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct wl_compositor *comp;
struct wl_surface *surface;
struct wl_buffer *buff;
struct wl_shm *shm;
struct xdg_wm_base *sh;
struct xdg_toplevel *top;
struct wl_seat *wl_seat;
struct wl_keyboard *wl_keyboard;
struct wl_pointer *wl_pointer;
struct wl_touch *wl_touch;

uint8_t *pixel;
uint16_t w = 200;
uint16_t h = 100;
uint8_t c = 0;
uint8_t cls = 0;

int32_t alc_shm(uint64_t sz) {
  char name[8];
  name[0] = '/';
  name[7] = 0;
  for (uint8_t i = 1; i < 6; i++) {
    name[i] = (rand() & 23) + 'a';
  }

  printf("%s", name);

  int32_t fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL,
                        S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH);

  shm_unlink(name);
  ftruncate(fd, sz);

  return fd;
}

void resize() {
  int32_t fd = alc_shm(w * h * 4);
  pixel =
      (uint8_t *)mmap(0, w * h * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, w * h * 4);
  buff =
      wl_shm_pool_create_buffer(pool, 0, w, h, w * 4, WL_SHM_FORMAT_ARGB8888);
  wl_shm_pool_destroy(pool);
}

void draw() {
  c++;
  memset(pixel, c, w * h * 4);

  wl_surface_attach(surface, buff, 0, 0);
  wl_surface_damage(surface, 0, 0, w, h);
  wl_surface_commit(surface);
}

void frame_new(void *data, struct wl_callback *wl_callback,
               uint32_t callback_data);

wl_callback_listener wl_callback_listener = {.done = frame_new};

void frame_new(void *data, struct wl_callback *wl_callback,
               uint32_t callback_data) {
  wl_callback_destroy(wl_callback);
  wl_callback = wl_surface_frame(surface);
  wl_callback_add_listener(wl_callback, &wl_callback_listener, 0);
  draw();
}

void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                           uint32_t serial) {
  xdg_surface_ack_configure(xdg_surface, serial);
  if (!pixel) {
    resize();
  }
  draw();
}

struct xdg_surface_listener xdg_surface_listener = {.configure =
                                                        xdg_surface_configure};

void xdg_toplevel_listener_configure(void *data,
                                     struct xdg_toplevel *xdg_toplevel,
                                     int32_t width, int32_t height,
                                     struct wl_array *states) {
  if (!width && !height)
    return;

  if (w != width || h != height) {
    munmap(pixel, w * h * 4);
    w = width;
    h = height;
    resize();
  }
}

void xdg_toplevel_listener_close(void *data,
                                 struct xdg_toplevel *xdg_toplevel) {
  cls = 1;
}

void configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel,
                      int32_t width, int32_t height) {}

void wm_capabilities(void *data, struct xdg_toplevel *xdg_toplevel,
                     struct wl_array *capabilities) {}

struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_listener_configure,
    .close = xdg_toplevel_listener_close,
    /* .configure_bounds = configure_bounds, */
    /* .wm_capabilities = wm_capabilities, */
};

void sh_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}

struct xdg_wm_base_listener sh_listener = {
    .ping = sh_ping,
};

void keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format,
            int32_t fd, uint32_t size) {}

void enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
           struct wl_surface *surface, struct wl_array *keys) {}

void leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
           struct wl_surface *surface) {}

void key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
         uint32_t time, uint32_t key, uint32_t state) {}

void modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
               uint32_t mods_depressed, uint32_t mods_latched,
               uint32_t mods_locked, uint32_t group) {}

void repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate,
                 int32_t delay) {}

struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = keymap,
    .enter = enter,
    .leave = leave,
    .key = key,
    .modifiers = modifiers,
    .repeat_info = repeat_info,
};

static void wl_seat_capabilities(void *data, struct wl_seat *wl_seat,
                                 uint32_t capabilities) {
  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD && !wl_keyboard) {
    wl_keyboard = wl_seat_get_keyboard(wl_seat);
    wl_keyboard_add_listener(wl_keyboard, &wl_keyboard_listener, 0);
  }
}

static void wl_seat_name(void *data, struct wl_seat *wl_seat,
                         const char *name) {
  fprintf(stderr, "seat name: %s\n", name);
}

static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = wl_seat_capabilities,
    .name = wl_seat_name,
};

void reg_glob(void *data, struct wl_registry *reg, uint32_t name,
              const char *interface, uint32_t v) {
  if (!strcmp(interface, wl_compositor_interface.name)) {
    comp = (wl_compositor *)wl_registry_bind(reg, name,
                                             &wl_compositor_interface, 4);
  } else if (!strcmp(interface, wl_shm_interface.name)) {
    shm = (wl_shm *)wl_registry_bind(reg, name, &wl_shm_interface, 1);
  } else if (!strcmp(interface, xdg_wm_base_interface.name)) {
    sh = (xdg_wm_base *)wl_registry_bind(reg, name, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(sh, &sh_listener, 0);
  } else if (!strcmp(interface, wl_seat_interface.name)) {
    wl_seat =
        (struct wl_seat *)wl_registry_bind(reg, name, &wl_seat_interface, 7);
    wl_seat_add_listener(wl_seat, &wl_seat_listener, 0);
  }
}

void reg_glob_rem(void *data, struct wl_registry *reg, uint32_t name) {}

struct wl_registry_listener reg_list = {.global = reg_glob,
                                        .global_remove = reg_glob_rem};

int main() {
  struct wl_display *disp = wl_display_connect(0);
  struct wl_registry *reg = wl_display_get_registry(disp);
  wl_registry_add_listener(reg, &reg_list, 0);
  wl_display_roundtrip(disp);

  surface = wl_compositor_create_surface(comp);
  struct wl_callback *cb = wl_surface_frame(surface);
  wl_callback_add_listener(cb, &wl_callback_listener, 0);

  xdg_surface *xdg_surface = xdg_wm_base_get_xdg_surface(sh, surface);
  xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, 0);

  top = xdg_surface_get_toplevel(xdg_surface);
  xdg_toplevel_add_listener(top, &xdg_toplevel_listener, 0);
  xdg_toplevel_set_title(top, "Handmade Wayland");
  wl_surface_commit(surface);

  while (wl_display_dispatch(disp)) {
    if (cls)
      break;
  }

  if (buff) {
    wl_buffer_destroy(buff);
  }
  xdg_toplevel_destroy(top);
  xdg_surface_destroy(xdg_surface);
  wl_surface_destroy(surface);
  wl_display_disconnect(disp);
  return 0;
}
