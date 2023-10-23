#include "handmade.h"
#include <math.h>
#include <stdint.h>

static void GameRender(struct game_offscreen_buffer *Buffer) {
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

static void GameOutputSound(struct game_sound_output_buffer *SoundBuffer) {
  static float tSine;
  int16_t ToneVolume = 3000;
  int ToneHz = 256;
  int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;
  int16_t *SampleOut = SoundBuffer->Samples;
  for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount;
       SampleIndex++) {
    float SineValue = sinf(tSine);
    int16_t SampleValue = (int16_t)(SineValue * ToneVolume);
    *SampleOut++ = SampleValue;
    *SampleOut++ = SampleValue;

    tSine += 2.0f * M_PI / (float)WavePeriod;
  }
}

static void GameUpdateAndRender(struct game_offscreen_buffer *Buffer,
                                struct game_sound_output_buffer *SoundBuffer) {
  // TODO: Allow sample offsets here for more robust platform options
  GameOutputSound(SoundBuffer);
  GameRender(Buffer);
}
