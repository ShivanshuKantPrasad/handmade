#include "pulse/channelmap.h"
#include "pulse/context.h"
#include "pulse/def.h"
#include "pulse/thread-mainloop.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <math.h>
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void context_state_cb(pa_context *context, void *mainloop) {
  pa_threaded_mainloop_signal(mainloop, 0);
}

void stream_state_cb(pa_stream *s, void *mainloop) {
  pa_threaded_mainloop_signal(mainloop, 0);
}

void stream_success_cb(pa_stream *stream, int success, void *userdata) {
  return;
}

void stream_write_cb(pa_stream *stream, size_t requested_bytes,
                     void *userdata) {

  int bytes_remaining = requested_bytes;

  while (bytes_remaining > 0) {
    uint16_t *buffer = NULL;
    size_t bytes_to_fill = 44100;
    size_t i;

    if (bytes_to_fill > bytes_remaining)
      bytes_to_fill = bytes_remaining;

    pa_stream_begin_write(stream, (void **)&buffer, &bytes_to_fill);

    for (i = 0; i < bytes_to_fill; i += 2) {
      int16_t value = (i % 100) * 100;
      buffer[i] = value;
      buffer[i + 1] = value;
    }

    pa_stream_write(stream, buffer, bytes_to_fill, NULL, 0LL, PA_SEEK_RELATIVE);
    bytes_remaining -= bytes_to_fill;
  }
}

int main() {

  int width = 400;
  int height = 400;

  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display");
    exit(1);
  }

  Window window = XCreateSimpleWindow(display, XDefaultRootWindow(display), 0,
                                      0, width, height, 1, 0, 0);

  XWindowAttributes wa;
  XGetWindowAttributes(display, window, &wa);

  GC gc = XCreateGC(display, window, 0, NULL);

  Atom del_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &del_window, 1);

  Atom wm_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
  Atom wm_dialog = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
  XChangeProperty(display, window, wm_type, XA_ATOM, 32, PropModeAppend,
                  (unsigned char *)&wm_dialog, 1);

  /* Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False); */
  /* Atom wm_floating = XInternAtom(display, "_NET_WM_STATE_MODAL", False); */
  /* XChangeProperty(display, window, wm_state, XA_ATOM, 32, PropModeAppend, */
  /*                 (unsigned char*) &wm_floating, 1); */

  XSelectInput(display, window, KeyPressMask | ExposureMask);

  XMapWindow(display, window);

  /* pa_simple *s; */
  pa_sample_spec ss;

  ss.format = PA_SAMPLE_S16NE;
  ss.channels = 2;
  ss.rate = 44100;

  /* int bytes_per_sample = ss.channels * sizeof(int16_t); */
  /* int frequency = 256; */
  /* int wave_period = ss.rate / frequency; */
  /* int duration = 5; */
  /* int cycles = duration * frequency; */
  /* int no_sample = wave_period * cycles; */
  /* int16_t *buffer = malloc(no_sample * bytes_per_sample); */
  /**/
  /* for (int i = 0; i < no_sample; i++) { */
  /**/
  /*   float t = 2.0f * 3.14f * (float)i / (float)wave_period; */
  /*   float SineValue = sinf(t); */
  /*   int16_t value = SineValue * 12000; */
  /*   buffer[2 * i] = value; */
  /*   buffer[2 * i + 1] = value; */
  /* } */
  /**/
  /* s = pa_simple_new(NULL, "Handmade", PA_STREAM_PLAYBACK, NULL, "Music", &ss,
   */
  /*                   NULL, NULL, NULL); */
  /**/
  /* pa_simple_write(s, buffer, no_sample * bytes_per_sample, NULL); */

  pa_threaded_mainloop *mainloop;
  pa_mainloop_api *mainloop_api;
  pa_context *context;
  pa_stream *stream;

  mainloop = pa_threaded_mainloop_new();
  assert(mainloop);
  mainloop_api = pa_threaded_mainloop_get_api(mainloop);
  context = pa_context_new(mainloop_api, "pcm-playback");
  assert(context);

  pa_context_set_state_callback(context, &context_state_cb, mainloop);

  pa_threaded_mainloop_lock(mainloop);

  assert(pa_threaded_mainloop_start(mainloop) == 0);
  assert(pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) == 0);

  for (;;) {
    pa_context_state_t context_state = pa_context_get_state(context);
    assert(PA_CONTEXT_IS_GOOD(context_state));
    if (context_state == PA_CONTEXT_READY)
      break;
    pa_threaded_mainloop_wait(mainloop);
  }

  pa_channel_map map;
  pa_channel_map_init_stereo(&map);

  stream = pa_stream_new(context, "Playback", &ss, &map);
  pa_stream_set_state_callback(stream, stream_state_cb, mainloop);
  pa_stream_set_write_callback(stream, stream_write_cb, mainloop);

  pa_buffer_attr buffer_attr;
  buffer_attr.maxlength = (uint32_t)-1;
  buffer_attr.tlength = (uint32_t)-1;
  buffer_attr.prebuf = (uint32_t)-1;
  buffer_attr.minreq = (uint32_t)-1;

  pa_stream_flags_t stream_flags;
  stream_flags = PA_STREAM_START_CORKED | PA_STREAM_INTERPOLATE_TIMING |
                 PA_STREAM_NOT_MONOTONIC | PA_STREAM_AUTO_TIMING_UPDATE |
                 PA_STREAM_ADJUST_LATENCY;

  assert(pa_stream_connect_playback(stream, NULL, &buffer_attr, stream_flags,
                                    NULL, NULL) == 0);

  for (;;) {
    pa_stream_state_t stream_state = pa_stream_get_state(stream);
    assert(PA_STREAM_IS_GOOD(stream_state));
    if (stream_state == PA_STREAM_READY)
      break;
    pa_threaded_mainloop_wait(mainloop);
  }

  pa_threaded_mainloop_unlock(mainloop);

  pa_stream_cork(stream, 0, stream_success_cb, mainloop);

  char *image = (char *)malloc(width * height * 4);
  int stride = width * 4;

  uint8_t *pixel = (uint8_t *)image;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {

      *pixel = 255; // blue
      pixel++;

      *pixel = 255; // green
      pixel++;

      *pixel = 0; // red
      pixel++;

      *pixel = 0; // padding
      pixel++;
    }
  }

  XImage *ximage = XCreateImage(display, wa.visual, wa.depth, ZPixmap, 0, image,
                                width, height, 32, 0);

  while (1) {
    XEvent event;
    XNextEvent(display, &event);

    switch (event.type) {

    case KeyPress: {
      fprintf(stderr, "Key Pressed %d\n", event.xkey.keycode);
    } break;

    case ClientMessage: {
      if ((Atom)event.xclient.data.l[0] == del_window) {
        fprintf(stderr, "WM_DELETE_WINDOW\n");
        goto breakout;
      }
    } break;

    case Expose: {
      XPutImage(display, window, gc, ximage, 0, 0, 0, 0, width, height);
    } break;
    }
  }

breakout:
  XDestroyWindow(display, window);
  XCloseDisplay(display);

  return 0;
}
