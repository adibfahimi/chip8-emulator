#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t *memory;
  uint8_t v[16];
  uint16_t i;
  uint16_t pc;
  uint8_t gfx[64 * 32];
  bool draw_flag;
  uint8_t delay_timer;
  uint8_t sound_timer;
  uint16_t stack[16];
  uint8_t sp;
  uint8_t key[16];
} Chip8;

uint8_t *read_file(char *file_name, int *file_size);
Chip8 *chip8_init(char *rom_file_name);
void chip8_emulate(Chip8 *chip8);
void chip8_deinit(Chip8 *chip8);

#endif
