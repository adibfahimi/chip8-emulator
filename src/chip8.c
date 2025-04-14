#include "chip8.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define X(opcode) ((opcode & 0x0F00) >> 8)
#define Y(opcode) ((opcode & 0x00F0) >> 4)
#define NN(opcode) (opcode & 0x00FF)
#define NNN(opcode) (opcode & 0x0FFF)

const uint8_t FONTSET[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

uint8_t *read_file(char *file_name, int *file_size) {
  FILE *fp = fopen(file_name, "rb");
  if (!fp) {
    perror("Error opening file");
    return NULL;
  }

  fseek(fp, 0L, SEEK_END);
  *file_size = ftell(fp);
  fseek(fp, 0L, SEEK_SET);

  uint8_t *buffer = malloc(*file_size);
  if (!buffer) {
    perror("Error allocating memory");
    fclose(fp);
    return NULL;
  }

  size_t result = fread(buffer, 1, *file_size, fp);
  if (result != *file_size) {
    perror("Error reading file");
    free(buffer);
    fclose(fp);
    return NULL;
  }

  fclose(fp);
  return buffer;
}

Chip8 *chip8_init(char *rom_file_name) {
  Chip8 *chip8 = malloc(sizeof(Chip8));
  if (!chip8) {
    perror("Error allocating Chip8 struct");
    return NULL;
  }

  chip8->i = 0;
  chip8->pc = 0x200;
  chip8->delay_timer = 0;
  chip8->sound_timer = 0;
  chip8->sp = 0;

  chip8->memory = malloc(4096);
  if (!chip8->memory) {
    perror("Error allocating memory");
    free(chip8);
    return NULL;
  }

  memset(chip8->memory, 0, 4096);
  memset(chip8->gfx, 0, sizeof(chip8->gfx));
  memset(chip8->stack, 0, sizeof(chip8->stack));
  memset(chip8->key, 0, sizeof(chip8->key));
  memset(chip8->v, 0, sizeof(chip8->v));

  memcpy(&chip8->memory[0x50], FONTSET, sizeof(FONTSET));

  int file_size = 0;
  uint8_t *rom_bytes = read_file(rom_file_name, &file_size);
  if (!rom_bytes) {
    perror("Error reading ROM file");
    free(chip8->memory);
    free(chip8);
    return NULL;
  }

  if (file_size > (4096 - 512)) {
    perror("ROM file too large");
    free(rom_bytes);
    free(chip8->memory);
    free(chip8);
    return NULL;
  }

  memcpy(&chip8->memory[0x200], rom_bytes, file_size);
  free(rom_bytes);

  return chip8;
}

void chip8_emulate(Chip8 *chip8) {
  uint16_t opcode =
      chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];

  uint16_t x, y;
  uint16_t vx;

  switch (opcode & 0xF000) {
  case 0x0000:
    switch (opcode & 0x00FF) {
    case 0x00E0:
      memset(chip8->gfx, 0, 64 * 32);
      chip8->pc += 2;
      break;

    case 0x00EE:
      if (chip8->sp == 0) {
        fprintf(stderr, "Stack underflow error\n");
        chip8->pc += 2;
        break;
      }
      chip8->sp -= 1;
      chip8->pc = chip8->stack[chip8->sp];
      chip8->pc += 2;
      break;

    case 0x0000:
      chip8->pc = NNN(opcode);
      break;

    default:
      perror("Unknown opcode");
    }
    break;

  case 0xA000:
    chip8->i = NNN(opcode);
    chip8->pc += 2;
    break;

  case 0xC000: {
    uint8_t X = (opcode & 0x0F00) >> 8;
    uint8_t NN = opcode & 0x00FF;
    chip8->v[X] = (uint8_t)(GetRandomValue(0, 255)) & NN;
    chip8->pc += 2;
  } break;

  case 0xD000:
    x = chip8->v[X(opcode)];
    int y = chip8->v[Y(opcode)];
    int height = opcode & 0x000F;
    int pixel;

    chip8->v[0xF] = 0;

    for (int yline = 0; yline < height; yline++) {
      int mem_idx = chip8->i + yline;
      if (mem_idx < 0 || mem_idx >= 4096) {
        fprintf(stderr, "Memory access out of bounds\n");
        continue;
      }
      pixel = chip8->memory[mem_idx];
      for (int xline = 0; xline < 8; xline++) {
        int px = (x + xline) % 64;
        int py = (y + yline) % 32;
        int gfx_idx = px + (py * 64);
        if (gfx_idx < 0 || gfx_idx >= (64 * 32)) {
          fprintf(stderr, "Graphics array access out of bounds\n");
          continue;
        }
        if ((pixel & (0x80 >> xline)) != 0) {
          if (chip8->gfx[gfx_idx] == 1) {
            chip8->v[0xF] = 1;
          }
          chip8->gfx[gfx_idx] ^= 1;
        }
      }
    }

    chip8->draw_flag = true;
    chip8->pc += 2;
    break;

  case 0x1000:
    chip8->pc = opcode & 0x0FFF;
    break;
  case 0x2000:
    if (chip8->sp >= 16) {
      fprintf(stderr, "Stack overflow error\n");
      chip8->pc += 2;
      break;
    }
    chip8->stack[chip8->sp] = chip8->pc;
    chip8->sp += 1;
    chip8->pc = NNN(opcode);
    break;
  case 0x3000:
    if (chip8->v[X(opcode)] == NN(opcode)) {
      chip8->pc += 4;
    } else {
      chip8->pc += 2;
    }
    break;
  case 0x4000:
    if (chip8->v[X(opcode)] != NN(opcode)) {
      chip8->pc += 4;
    } else {
      chip8->pc += 2;
    }
    break;
  case 0x5000:
    if (chip8->v[X(opcode)] == chip8->v[Y(opcode)]) {
      chip8->pc += 4;
    } else {
      chip8->pc += 2;
    }
    break;
  case 0x6000:
    chip8->v[X(opcode)] = NN(opcode);
    chip8->pc += 2;
    break;

  case 0x7000:
    x = X(opcode);
    chip8->v[x] = chip8->v[x] += NN(opcode);
    chip8->pc += 2;
    break;

  case 0x8000:
    switch (opcode & 0x000F) {
    case 0x0000:
      chip8->v[X(opcode)] = chip8->v[Y(opcode)];
      chip8->pc += 2;
      break;

    case 0x0001:
      chip8->v[X(opcode)] |= chip8->v[Y(opcode)];
      chip8->pc += 2;
      break;

    case 0x0002:
      chip8->v[X(opcode)] &= chip8->v[Y(opcode)];
      chip8->pc += 2;
      break;

    case 0x0003:
      chip8->v[X(opcode)] ^= chip8->v[Y(opcode)];
      chip8->pc += 2;
      break;

    case 0x0004:
      x = X(opcode);
      y = Y(opcode);
      uint16_t sum = chip8->v[x] + chip8->v[y];
      if (sum > 0xFF) {
        chip8->v[0xF] = 1;
      } else {
        chip8->v[0xF] = 0;
      }
      chip8->v[x] = (sum & 0xFF);
      chip8->pc += 2;
      break;

    case 0x0005:
      x = X(opcode);
      y = Y(opcode);
      chip8->v[0xF] = chip8->v[x] >= chip8->v[y] ? 1 : 0;
      chip8->v[x] -= chip8->v[y];
      chip8->pc += 2;
      break;

    case 0x0006:
      x = X(opcode);
      chip8->v[0xF] = chip8->v[x] & 0x1;
      chip8->v[x] >>= 1;
      chip8->pc += 2;
      break;

    case 0x0007:
      x = X(opcode);
      uint16_t y = Y(opcode);
      if (chip8->v[y] > chip8->v[x]) {
        chip8->v[0xF] = 1;
      } else {
        chip8->v[0xF] = 0;
      }
      chip8->v[x] = chip8->v[y] - chip8->v[x];
      chip8->pc += 2;
      break;

    case 0x000E:
      x = X(opcode);
      chip8->v[0xF] = chip8->v[x] >> 7;
      chip8->v[x] <<= 1;
      chip8->pc += 2;
      break;

    default:
      perror("Unknown opcode");
    }
    break;

  case 0x9000:
    if (chip8->v[X(opcode)] != chip8->v[Y(opcode)]) {
      chip8->pc += 4;
    } else {
      chip8->pc += 2;
    }
    break;

  case 0xB000:
    chip8->pc = NNN(opcode) + chip8->v[0];
    break;

  case 0xE000:
    switch (opcode & 0x00FF) {
    case 0x009E:
      vx = chip8->v[X(opcode)];
      if (vx >= 16) {
        perror("Key index out of bounds");
      }
      if (chip8->key[vx] != 0) {
        chip8->pc += 4;
      } else {
        chip8->pc += 2;
      }
      break;

    case 0x00A1:
      vx = chip8->v[X(opcode)];
      if (vx >= 16) {
        perror("Key index out of bounds");
      }
      if (chip8->key[vx] == 0) {
        chip8->pc += 4;
      } else {
        chip8->pc += 2;
      }
      break;

    default:
      perror("Unknown opcode");
    }
    break;

  case 0xF000:
    switch (opcode & 0x00FF) {
    case 0x0007:
      chip8->v[X(opcode)] = chip8->delay_timer;
      chip8->pc += 2;
      break;
    case 0x000A:
      bool key_press = false;

      for (int i = 0; i < 16; i++) {
        if (chip8->key[i] != 0) {
          chip8->v[X(opcode)] = i;
          key_press = true;
        }
      }

      if (!key_press)
        return;

      chip8->pc += 2;
      break;

    case 0x0015:
      chip8->delay_timer = chip8->v[X(opcode)];
      chip8->pc += 2;
      break;

    case 0x0018:
      chip8->sound_timer = chip8->v[X(opcode)];
      chip8->pc += 2;
      break;

    case 0x001E:
      chip8->i += chip8->v[X(opcode)];
      chip8->pc += 2;
      break;

    case 0x0029:
      chip8->i = chip8->v[X(opcode)] * 0x5;
      chip8->pc += 2;
      break;

    case 0x0033:
      x = X(opcode);
      chip8->memory[chip8->i] = chip8->v[x] / 100;
      chip8->memory[(chip8->i + 1)] = (chip8->v[x] / 10) % 10;
      chip8->memory[(chip8->i + 2)] = chip8->v[x] % 10;
      chip8->pc += 2;
      break;

    case 0x0055:
      x = X(opcode);
      for (int i = 0; i <= x; i++) {
        chip8->memory[(chip8->i + i)] = chip8->v[i];
      }
      chip8->i += x + 1;
      chip8->pc += 2;
      break;

    case 0x0065:
      x = X(opcode);
      for (int i = 0; i <= x; i++) {
        chip8->v[i] = chip8->memory[(chip8->i + i)];
      }
      chip8->i += x + 1;
      chip8->pc += 2;
      break;

    default:
      perror("Unknown opcode");
    }

    break;

  default:
    perror("Unknown opcode");
  }
}

void chip8_deinit(Chip8 *chip8) {
  if (chip8) {
    if (chip8->memory) {
      free(chip8->memory);
    }
    free(chip8);
  }
}
