#include "input.h"
#include "chip8.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>

void update_input_keys(Chip8 *chip8) {
  memset(chip8->key, 0, sizeof(chip8->key));

  // Standard CHIP-8 keypad mapping:
  // 1 2 3 C   => 1 2 3 4
  // 4 5 6 D   => Q W E R
  // 7 8 9 E   => A S D F
  // A 0 B F   => Z X C V

  if (IsKeyDown(KEY_ONE))
    chip8->key[0x1] = 1;
  if (IsKeyDown(KEY_TWO))
    chip8->key[0x2] = 1;
  if (IsKeyDown(KEY_THREE))
    chip8->key[0x3] = 1;
  if (IsKeyDown(KEY_FOUR))
    chip8->key[0xC] = 1;

  if (IsKeyDown(KEY_Q))
    chip8->key[0x4] = 1;
  if (IsKeyDown(KEY_W))
    chip8->key[0x5] = 1;
  if (IsKeyDown(KEY_E))
    chip8->key[0x6] = 1;
  if (IsKeyDown(KEY_R))
    chip8->key[0xD] = 1;

  if (IsKeyDown(KEY_A))
    chip8->key[0x7] = 1;
  if (IsKeyDown(KEY_S))
    chip8->key[0x8] = 1;
  if (IsKeyDown(KEY_D))
    chip8->key[0x9] = 1;
  if (IsKeyDown(KEY_F))
    chip8->key[0xE] = 1;

  if (IsKeyDown(KEY_Z))
    chip8->key[0xA] = 1;
  if (IsKeyDown(KEY_X))
    chip8->key[0x0] = 1;
  if (IsKeyDown(KEY_C))
    chip8->key[0xB] = 1;
  if (IsKeyDown(KEY_V))
    chip8->key[0xF] = 1;
}
