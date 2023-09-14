#include <X11/Xlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

  XSelectInput(display, window, KeyPressMask | ExposureMask);

  XMapWindow(display, window);

  unsigned char *image = (unsigned char *)malloc(width * height * 4);
  int stride = width * 4;

  uint8_t *pixel = image;
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
