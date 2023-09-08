#include <stdio.h>
#include <windows.h>
#include <winuser.h>

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message, WPARAM WParam,
                                    LPARAM LParam) {
  LRESULT Result = 0;

  switch (Message) {

    /* case WM_SIZE: { */
    /*   printf("WM_SIZE\n"); */
    /* } break; */

    /* case WM_CLOSE: { */
    /*   printf("WM_CLOSE\n"); */
    /* } break; */

    /* case WM_ACTIVATEAPP: { */
    /*   printf("WM_ACTIVATEAPP\n"); */
    /* } break; */

  case WM_DESTROY: {
    printf("WM_DESTROY\n");
    PostQuitMessage(0);
  } break;

  default: {
    Result = DefWindowProc(Window, Message, WParam, LParam);
  } break;
  }
  return Result;
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine,
                     int ShowCmd) {

  // Create a Window Class
  WNDCLASS WindowClass = {};
  WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";

  if (!RegisterClass(&WindowClass)) {
    printf("Oh shi- %d\n", GetLastError());
  }

  HWND WindowHandle = CreateWindowEx(
      0, WindowClass.lpszClassName, "Handmade", WS_VISIBLE, CW_USEDEFAULT,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);

  if (!WindowHandle) {
    printf("Oh shi- %d\n", GetLastError());
  }

  MSG Msg;
  while (GetMessage(&Msg, 0, 0, 0)) {
    TranslateMessage(&Msg);
    DispatchMessage(&Msg);
  }
  return 0;
}
