#pragma once
#include <stdint.h>

struct game_offscreen_buffer {
  void *Memory;
  int BytesPerPixel;
  int Width;
  int Height;
};

struct game_sound_output_buffer {
  int SamplesPerSecond;
  int SampleCount;
  int16_t *Samples;
};

static void GameRender(struct game_offscreen_buffer *Buffer);
static void GameUpdateAndRender(struct game_offscreen_buffer *Buffer,
                                struct game_sound_output_buffer *SoundBuffer);
static void GameOutputSound(struct game_sound_output_buffer *SoundBuffer);
