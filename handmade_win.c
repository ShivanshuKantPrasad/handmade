#include <stdbool.h>
#include <stdio.h>
#include <windows.h>
#include <winuser.h>

#define local_persist static
#define global_variable static
#define internal static

global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void Win32ResizeDIBSection(int Width, int Height) {

  if (BitmapHandle) {
    DeleteObject(BitmapHandle);
  } else {
    BitmapDeviceContext = CreateCompatibleDC(0);
  }

  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = Width;
  BitmapInfo.bmiHeader.biHeight = Height;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;

  BitmapHandle = CreateDIBSection(BitmapDeviceContext, &BitmapInfo,
                                  DIB_RGB_COLORS, &BitmapMemory, 0, 0);
}

internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width,
                                int Height) {
  StretchDIBits(DeviceContext, X, Y, Width, Height, X, Y, Width, Height,
                BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND Window, UINT Message,
                                         WPARAM WParam, LPARAM LParam) {
  LRESULT Result = 0;

  switch (Message) {

  case WM_SIZE: {
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    int Width = ClientRect.right - ClientRect.left;
    int Height = ClientRect.bottom - ClientRect.top;
    Win32ResizeDIBSection(Width, Height);
  } break;

  case WM_CLOSE: {
    Running = false;
  } break;

  case WM_ACTIVATEAPP: {
  } break;

  case WM_DESTROY: {
    Running = false;
  } break;

  case WM_PAINT: {
    PAINTSTRUCT Paint;
    HDC DeviceContext = BeginPaint(Window, &Paint);
    int X = Paint.rcPaint.left;
    int Y = Paint.rcPaint.top;
    int Width = Paint.rcPaint.right - X;
    int Height = Paint.rcPaint.bottom - Y;
    Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
    EndPaint(Window, &Paint);
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
  WindowClass.lpfnWndProc = Win32MainWindowCallback;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";

  if (!RegisterClass(&WindowClass)) {
    printf("Oh shi- %d\n", GetLastError());
    return 0;
  }

  HWND WindowHandle = CreateWindowEx(
      0, WindowClass.lpszClassName, "Handmade",
      WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
      CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);

  if (!WindowHandle) {
    printf("Oh shi- %d\n", GetLastError());
    return 0;
  }

  Running = true;
  while (Running) {
    MSG Msg;
    BOOL MsgRes = GetMessage(&Msg, 0, 0, 0);
    if (MsgRes > 0) {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
    }
  }
  return 0;
}
