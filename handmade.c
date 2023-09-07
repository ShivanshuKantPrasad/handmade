#include "X11/X.h"
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

int main() {

  Display *display;
  Window window;
  XEvent event;
  int screen;

  display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display");
    exit(1);
  }

  screen = DefaultScreen(display);

  window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10,
                               100, 100, 1, BlackPixel(display, screen),
                               WhitePixel(display, screen));

  Atom del_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
  XSetWMProtocols(display, window, &del_window, 1);

  XMapWindow(display, window);

  while (1) {
    XNextEvent(display, &event);
    switch (event.type) {
    case KeyPress:
    case ClientMessage:
      goto breakout;
    case Expose:
      XFillRectangle(display, window, DefaultGC(display, screen), 10, 10, 10,
                     10);
    }
  }

breakout:
  XDestroyWindow(display, window);
  XCloseDisplay(display);

  return 0;
}
