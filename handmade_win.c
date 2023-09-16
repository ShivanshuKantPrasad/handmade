#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <winuser.h>

#define local_persist static
#define global_variable static
#define internal static

struct win32_offscreen_buffer {
  BITMAPINFO BitmapInfo;
  void *Memory;
  int Width;
  int Height;
  int BytesPerPixel;
  int Pitch;
};

global_variable bool Running;
global_variable struct win32_offscreen_buffer Buffer;

internal void Win32Render(struct win32_offscreen_buffer *Buffer) {
  uint8_t *Row = (uint8_t *)Buffer->Memory;
  for (int y = 0; y < Buffer->BitmapInfo.bmiHeader.biHeight; ++y) {
    uint8_t *Pixel = (uint8_t *)Row;
    for (int x = 0; x < Buffer->BitmapInfo.bmiHeader.biWidth; ++x) {

      // Blue
      *Pixel = 0x00;
      ++Pixel;

      // Green
      *Pixel = x % 255;
      ++Pixel;

      // Red
      *Pixel = y % 255;
      ++Pixel;

      // Padding
      *Pixel = 0x00;
      ++Pixel;
    }
    Row += Buffer->Pitch;
  }
}

internal void Win32ResizeDIBSection(struct win32_offscreen_buffer *Buffer,
                                    int Width, int Height) {

  if (Buffer->Memory) {
    VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
  }

  Buffer->BitmapInfo.bmiHeader.biSize = sizeof(Buffer->BitmapInfo.bmiHeader);
  Buffer->BitmapInfo.bmiHeader.biWidth = Width;
  Buffer->BitmapInfo.bmiHeader.biHeight = Height;
  Buffer->BitmapInfo.bmiHeader.biPlanes = 1;
  Buffer->BitmapInfo.bmiHeader.biBitCount = 32;
  Buffer->BitmapInfo.bmiHeader.biCompression = BI_RGB;

  Buffer->Width = Width;
  Buffer->Height = Height;
  Buffer->BytesPerPixel = 4;
  Buffer->Pitch = Width * Buffer->BytesPerPixel;

  Buffer->Memory = VirtualAlloc(0, Buffer->BytesPerPixel * Width * Height,
                                MEM_COMMIT, PAGE_READWRITE);
}

struct window_dimensions {
  int Width;
  int Height;
};

internal struct window_dimensions Win32WindowDimension(HWND Window) {
  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  struct window_dimensions Dimension = {
      .Height = ClientRect.bottom - ClientRect.top,
      .Width = ClientRect.right - ClientRect.left,
  };
  return Dimension;
}

internal void Win32DisplayWindow(HDC DeviceContext, int WindowWidth,
                                 int WindowHeight,
                                 struct win32_offscreen_buffer Buffer) {

  StretchDIBits(DeviceContext, 0, 0, WindowWidth, WindowHeight, 0, 0,
                Buffer.Width, Buffer.Height, Buffer.Memory, &Buffer.BitmapInfo,
                DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND Window, UINT Message,
                                         WPARAM WParam, LPARAM LParam) {
  LRESULT Result = 0;

  switch (Message) {

  case WM_SIZE: {
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
    struct window_dimensions Dimension = Win32WindowDimension(Window);
    Win32DisplayWindow(DeviceContext, Dimension.Width, Dimension.Height,
                       Buffer);
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

  HWND Window = CreateWindowEx(0, WindowClass.lpszClassName, "Handmade",
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0,
                               0, Instance, 0);

  if (!Window) {
    printf("Oh shi- %d\n", GetLastError());
    return 0;
  }

  Win32ResizeDIBSection(&Buffer, 1280, 720);

  Running = true;
  while (Running) {
    MSG Msg;

    while (PeekMessage(&Msg, 0, 0, 0, PM_REMOVE)) {
      if (Msg.message == WM_QUIT) {
        Running = false;
      }
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
    }
    HDC DeviceContext = GetDC(Window);
    struct window_dimensions Dimension = Win32WindowDimension(Window);
    Win32Render(&Buffer);
    Win32DisplayWindow(DeviceContext, Dimension.Width, Dimension.Height,
                       Buffer);
    ReleaseDC(Window, DeviceContext);
  }
  return 0;
}
