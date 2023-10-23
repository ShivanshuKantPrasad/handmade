#include <cmath>
#include <cstdint>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <winuser.h>

#include <dsound.h>
#include <x86intrin.h>
#include <xinput.h>

#include "handmade.cpp"
#include "handmade.h"

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

struct win32_sound_output {
  int SamplesPerSecond;
  int Hz;
  uint32_t RunningSampleIndex;
  int WavePeriod;
  int BytesPerSample;
  int SecondaryBufferSize;
  int ToneVolume;
  int LatencySampleCount;
};

global_variable bool Running;
global_variable struct win32_offscreen_buffer Buffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

#define X_INPUT_GET_STATE(name)                                                \
  DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) { return ERROR_DEVICE_NOT_CONNECTED; }
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name)                                                \
  DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) { return ERROR_DEVICE_NOT_CONNECTED; }
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name)                                              \
  HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS,               \
                      LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void Win32LoadXInput(void) {
  HMODULE XInputLibrary = LoadLibrary("xinput1_4.dll");
  if (!XInputLibrary)
    XInputLibrary = LoadLibrary("xinput1_3.dll");

  if (XInputLibrary) {
    XInputGetState =
        (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
    if (!XInputGetState)
      XInputGetState = XInputGetStateStub;

    XInputSetState =
        (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    if (!XInputSetState)
      XInputSetState = XInputSetStateStub;

  } else {
    // TODO: Diagnostic
  }
}

internal void Win32InitDSound(HWND Window, int32_t SamplesPerSecond,
                              int32_t BufferSize) {
  // Load the library
  HMODULE DSoundLibrary = LoadLibrary("dsound.dll");
  if (!DSoundLibrary)
    return;

  direct_sound_create *DirectSoundCreate =
      (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
  LPDIRECTSOUND DirectSound;
  if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {

    WAVEFORMATEX WaveFormat;
    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormat.nChannels = 2;
    WaveFormat.nSamplesPerSec = SamplesPerSecond;
    WaveFormat.wBitsPerSample = 16;
    WaveFormat.nBlockAlign =
        (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
    WaveFormat.nAvgBytesPerSec =
        WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
    WaveFormat.cbSize = 0;

    if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) {

      DSBUFFERDESC BufferDescription;
      BufferDescription.dwSize = sizeof(BufferDescription);
      BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

      LPDIRECTSOUNDBUFFER PrimaryBuffer;
      if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription,
                                                   &PrimaryBuffer, 0))) {
        if (SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat))) {
        } else {
        }
      } else {
      }

    } else {
    }

    DSBUFFERDESC BufferDescription;
    BufferDescription.dwSize = sizeof(BufferDescription);
    BufferDescription.dwFlags = 0;
    BufferDescription.dwBufferBytes = BufferSize;
    BufferDescription.lpwfxFormat = &WaveFormat;
    if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription,
                                                 &GlobalSecondaryBuffer, 0))) {

    } else {
    }

  } else {
    // TODO: Diagnostic
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
      .Width = ClientRect.right - ClientRect.left,
      .Height = ClientRect.bottom - ClientRect.top,
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

  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_KEYUP: {
    uint32_t VKCODE = WParam;
    bool WasDown = ((LParam & (1 << 30)) != 0);
    bool IsDown = ((LParam & (1 << 31)) == 0);
    bool AltKeyWasDown = ((LParam & (1 << 29)) != 0);

    if (WasDown != IsDown) {
      if (AltKeyWasDown && VKCODE == VK_F4) {
        Running = false;
      }
    }

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

void Win32ClearBuffer(win32_sound_output *SoundOutput) {

  VOID *Region1;
  DWORD Region1Size;
  VOID *Region2;
  DWORD Region2Size;
  if (SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize,
                                            &Region1, &Region1Size, &Region2,
                                            &Region2Size, 0))) {
    uint8_t *DestSample = (uint8_t *)Region1;
    for (DWORD ByteIndex = 0; ByteIndex < Region1Size; ByteIndex++) {
      *DestSample++ = 0;
    }
    DestSample = (uint8_t *)Region2;
    for (DWORD ByteIndex = 0; ByteIndex < Region2Size; ByteIndex++) {
      *DestSample++ = 0;
    }
    GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
  }
}

void Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock,
                          DWORD BytesToWrite,
                          struct game_sound_output_buffer *SourceBuffer) {

  VOID *Region1;
  DWORD Region1Size;
  VOID *Region2;
  DWORD Region2Size;

  if (SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite, &Region1,
                                            &Region1Size, &Region2,
                                            &Region2Size, 0))) {
    int16_t *DestSample = (int16_t *)Region1;
    int16_t *SourceSample = SourceBuffer->Samples;
    DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
    DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
    for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount;
         SampleIndex++) {
      *DestSample++ = *SourceSample++;
      *DestSample++ = *SourceSample++;
      SoundOutput->RunningSampleIndex++;
    }

    DestSample = (int16_t *)Region2;
    for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount;
         SampleIndex++) {
      *DestSample++ = *SourceSample++;
      *DestSample++ = *SourceSample++;
      SoundOutput->RunningSampleIndex++;
    }

    GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
  }
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine,
                     int ShowCmd) {
  LARGE_INTEGER PerCountFrequencyResult;
  QueryPerformanceFrequency(&PerCountFrequencyResult);
  int64_t PerfCountFrequency = PerCountFrequencyResult.QuadPart;
  Win32LoadXInput();

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

  win32_sound_output SoundOutput = {};
  SoundOutput.SamplesPerSecond = 48000;
  SoundOutput.Hz = 256;
  SoundOutput.RunningSampleIndex = 0;
  SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.Hz;
  SoundOutput.BytesPerSample = sizeof(int16_t) * 2;
  SoundOutput.SecondaryBufferSize =
      SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
  SoundOutput.ToneVolume = 3000;
  SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 20;

  Win32InitDSound(Window, SoundOutput.SamplesPerSecond,
                  SoundOutput.SecondaryBufferSize);
  Win32ClearBuffer(&SoundOutput);
  GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

  Win32ResizeDIBSection(&Buffer, 1280, 720);

  LARGE_INTEGER LastCounter;
  QueryPerformanceCounter(&LastCounter);

  int64_t LastCycleCount;
  LastCycleCount = __rdtsc();

  int16_t *Samples =
      (int16_t *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize,
                              MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

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

    for (DWORD ControllerIndex; ControllerIndex < XUSER_MAX_COUNT;
         ControllerIndex++) {
      XINPUT_STATE ControllerState;
      if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
        bool DPadUp = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
        bool DPadDown = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
        bool DPadRight = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
        bool DPadLeft = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
        bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
        bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
        bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
        bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
        bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
        bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
        bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
        bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

        int16_t StickX = Pad->sThumbLX;
        int16_t StickY = Pad->sThumbLY;
      }
    }

    HDC DeviceContext = GetDC(Window);
    struct window_dimensions Dimension = Win32WindowDimension(Window);

    DWORD PlayCursor;
    DWORD WriteCursor;
    DWORD TargetCursor;
    DWORD ByteToLock;
    DWORD BytesToWrite;
    bool SoundIsValid = false;
    if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor,
                                                            &WriteCursor))) {
      ByteToLock =
          (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) %
          SoundOutput.SecondaryBufferSize;

      TargetCursor = (PlayCursor + (SoundOutput.LatencySampleCount *
                                    SoundOutput.BytesPerSample)) %
                     SoundOutput.SecondaryBufferSize;
      if (ByteToLock > TargetCursor) {
        BytesToWrite = SoundOutput.SecondaryBufferSize - ByteToLock;
        BytesToWrite += TargetCursor;
      } else {
        BytesToWrite = TargetCursor - ByteToLock;
      }
      SoundIsValid = true;
    }

    game_sound_output_buffer SoundBuffer;
    SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
    SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
    SoundBuffer.Samples = Samples;

    game_offscreen_buffer GameBuffer = {
        .Memory = Buffer.Memory,
        .BytesPerPixel = Buffer.BytesPerPixel,
        .Width = Buffer.Width,
        .Height = Buffer.Height,
    };
    GameUpdateAndRender(&GameBuffer, &SoundBuffer);

    if (SoundIsValid) {
      Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite,
                           &SoundBuffer);
    }

    Win32DisplayWindow(DeviceContext, Dimension.Width, Dimension.Height,
                       Buffer);
    ReleaseDC(Window, DeviceContext);

    int64_t EndCycleCount = __rdtsc();
    LARGE_INTEGER EndCounter;
    QueryPerformanceCounter(&EndCounter);

    int64_t CyclesElapsed = EndCycleCount - LastCycleCount;
    int64_t CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
    float TimeElapsed = (float)CounterElapsed / PerfCountFrequency;
    printf("%fms %.2fFPS - %ld MHz\n", TimeElapsed * 1000, 1 / TimeElapsed,
           CyclesElapsed / 1000000);

    LastCounter = EndCounter;
    LastCycleCount = EndCycleCount;
  }
  return 0;
}
