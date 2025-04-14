#include "bamboo.h"
#include "chip8.h"
#include "input.h"
#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int CUBE_SIZE = 10;
const int SCREEN_WIDTH = 64 * CUBE_SIZE;
const int SCREEN_HEIGHT = 32 * CUBE_SIZE;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file name>\n", argv[0]);
    return EXIT_FAILURE;
  }

  Chip8 *chip8 = chip8_init(argv[1]);
  if (chip8 == NULL) {
    fprintf(stderr, "Failed to initialize Chip8.\n");
    return EXIT_FAILURE;
  }

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "chip8 emulator");
  InitAudioDevice();
  SetTargetFPS(60);

  Wave wave = LoadWaveFromMemory(".wav", bamboo_wav, bamboo_wav_len);
  Sound sound = LoadSoundFromWave(wave);
  UnloadWave(wave);
  SetSoundVolume(sound, 0.5f);

  int cycles_per_frame = 10;

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    update_input_keys(chip8);

    for (int i = 0; i < cycles_per_frame; i++) {
      chip8_emulate(chip8);
    }

    if (chip8->delay_timer > 0) {
      chip8->delay_timer -= 1;
    }

    if (chip8->sound_timer > 0) {
      if (chip8->sound_timer == 1) {
        PlaySound(sound);
      }
      chip8->sound_timer -= 1;
    }

    if (chip8->draw_flag) {
      for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
          if (chip8->gfx[y * 64 + x] == 1) {
            DrawRectangle(x * CUBE_SIZE, y * CUBE_SIZE, CUBE_SIZE, CUBE_SIZE,
                          WHITE);
          }
        }
      }
      chip8->draw_flag = false;
    }

    EndDrawing();
  }

  UnloadSound(sound);
  CloseAudioDevice();
  CloseWindow();
  chip8_deinit(chip8);
  return EXIT_SUCCESS;
}
