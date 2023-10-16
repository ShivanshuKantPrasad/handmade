#include "handmade.h"
#include <stdint.h>

static void Win32Render(struct game_offscreen_buffer *Buffer) {
  uint8_t *Row = (uint8_t *)Buffer->Memory;
  for (int y = 0; y < Buffer->Height; ++y) {
    uint8_t *Pixel = (uint8_t *)Row;
    for (int x = 0; x < Buffer->Width; ++x) {

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
    Row += Buffer->Width * Buffer->BytesPerPixel;
  }
}
