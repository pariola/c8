#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 8 by 5 (8-bit each by 5 lines)
uint8_t font_set[] = {
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
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// NOTES
// - A hexadecimal digit uses 4 bits
// - Instructions are 2 bytes each (16-bits)
// - You cannot declare variables immediately after a case block

struct Chip8 {
  uint16_t m_index; // Index Register

  uint16_t sp;        // Stack Pointer Register
  uint16_t stack[16]; // Stack

  uint16_t pc; // Program Counter Register

  uint8_t m_delay, m_sound; // Delay Timer Register & Sound Timer Register

  uint8_t v[16];            // V (general) registers V0 - VF (16)
  uint8_t memory[4096];     // whole memory (4096)
  uint8_t graphics[32][64]; // 64 by 32
  uint8_t key_state[16];
};

struct Chip8 *init_cpu() {
  struct Chip8 *ch = calloc(1, sizeof(struct Chip8));
  if (ch == NULL)
    return NULL;

  ch->pc = 0x200;

  memcpy(&ch->memory, font_set, 5 * 16); // load font into memory

  return ch;
}

int load_rom(struct Chip8 *ch, char *filename) {
  FILE *f = fopen(filename, "rb");
  if (f == NULL)
    return 0;

  // Get the file size
  fseek(f, 0, SEEK_END); // end of file
  int fsize = ftell(f);
  fseek(f, 0, SEEK_SET); // back to beginning of file

  // read all byte into memory index 0x200 - ...
  fread(&ch->memory[0x200], fsize, 1, f);
  fclose(f);

  return fsize;
}

void exec(struct Chip8 *ch) {
  u_int16_t opcode = ch->memory[ch->pc] << 8 |
                     ch->memory[ch->pc + 1]; // fetch current instruction

  // printf("%04x\n", opcode); // debug print current instruction

  switch (opcode >> 12) // retrieve first nibble (4-bits)
  {
  case 0x0:
    if (opcode == 0x00E0) // clear display
      for (uint8_t r = 0; r < 32; r++)
        for (uint8_t c = 0; c < 64; c++)
          ch->graphics[r][c] = 0;

    else if (opcode == 0x00EE) {
      ch->sp -= 1;
      ch->pc = ch->stack[ch->sp];
    }
    ch->pc += 2;
    break;

  case 0x1:
    ch->pc = opcode & 0x0FFF; // mask last 3 hex values
    break;

  case 0x2:
    ch->stack[ch->sp] = ch->pc;
    ch->sp += 1;
    ch->pc = opcode & 0x0FFF; // mask last 3 hex values
    break;

  case 0x3: {
    uint8_t x = (opcode & 0x0F00) >> 8;

    if (ch->v[x] == (opcode & 0x00FF))
      ch->pc += 2;

    ch->pc += 2;
    break;
  }

  case 0x4: {
    uint8_t x = (opcode & 0x0F00) >> 8;

    if (ch->v[x] != (opcode & 0x00FF))
      ch->pc += 2;

    ch->pc += 2;
    break;
  }

  case 0x5: {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    if (ch->v[x] == ch->v[y])
      ch->pc += 2;

    ch->pc += 2;
    break;
  }

  case 0x6: {
    uint8_t x = (opcode & 0x0F00) >> 8;
    ch->v[x] = (uint8_t)(opcode & 0x00FF);
    ch->pc += 2;
    break;
  }

  case 0x7: {
    uint8_t x = (opcode & 0x0F00) >> 8;
    ch->v[x] += (uint8_t)(opcode & 0x00FF);
    ch->pc += 2;
    break;
  }

  case 0x8: {
    uint8_t n = opcode & 0x000F; // last nibble
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    if (n == 0)
      ch->v[x] = ch->v[y];
    else if (n == 1)
      ch->v[x] |= ch->v[y];
    else if (n == 2)
      ch->v[x] &= ch->v[y];
    else if (n == 3)
      ch->v[x] ^= ch->v[y];
    else if (n == 4) {
      uint16_t sum = (uint16_t)ch->v[x] + (uint16_t)ch->v[y];
      ch->v[0xF] = sum > 255;
      ch->v[x] = (uint8_t)sum & 0x00FF;
    } else if (n == 5) {
      ch->v[0xF] = ch->v[x] > ch->v[y];
      ch->v[x] -= ch->v[y];
    } else if (n == 6) {
      ch->v[0xF] = ch->v[x] & 1;
      ch->v[x] >>= 1;
    } else if (n == 7) {
      ch->v[0xF] = ch->v[y] > ch->v[x];
      ch->v[x] = ch->v[y] - ch->v[x];
    } else if (n == 0xE) {
      ch->v[0xF] = (ch->v[x] >> 7) == 1;
      ch->v[x] <<= 1;
    }

    ch->pc += 2;
    break;
  }

  case 0x9: {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    if (ch->v[x] != ch->v[y])
      ch->pc += 2;

    ch->pc += 2;
    break;
  }

  // Annn - LD I, addr
  case 0xA:
    ch->m_index = opcode & 0x0FFF; // mask last 3 hex values
    ch->pc += 2;
    break;

  // Bnnn - JP V0, addr
  case 0xB:
    ch->pc = ch->v[0] + opcode & 0x0FFF; // mask last 3 hex values
    break;

  // Cxkk - RND Vx, byte
  case 0xC: {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t kk = ((opcode & 0x00FF)) >> 4;

    ch->v[x] = rand() & kk; // ???
    ch->pc += 2;
    break;
  }

  // Dxyn - DRW Vx, Vy, nibble
  case 0xD: {
    ch->v[0xF] = 0;

    uint8_t n = (opcode & 0x000F);
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    uint8_t vX = ch->v[x];
    uint8_t vY = ch->v[y];

    for (uint8_t row = 0; row < n; row++) {
      uint8_t pixel = ch->memory[ch->m_index + row]; // one row of the pixel

      for (uint8_t col = 0; col < 8; col++) { // 8 columns (fixed size)
        const uint8_t msb = 0x80;             // 0b10000000
        if ((pixel & msb) != 0) {             // true, turn on a pixel
          // previously on - set v[0xF] to 1
          if (ch->graphics[vY + row][vX + col] == 1)
            ch->v[0xF] = 1;

          // sprite pixels are XOR'd with corresponding screen pixels
          ch->graphics[vY + row][vX + col] ^= 1; // 0 -> 1 || 1 -> 0
        }
        pixel <<= 1; // next bit
      }
    }

    ch->pc += 2;
    break;
  }

  case 0xE: {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t kk = ((opcode & 0x00FF));

    // Ex9E - SKP Vx
    if (kk == 0x9E && ch->key_state[x] == 1) // key is down
      ch->pc += 2;

    // ExA1 - SKNP Vx
    else if (kk == 0xA1 && ch->key_state[x] != 1) // key is not down
      ch->pc += 2;

    ch->pc += 2;

    break;
  }

  case 0xF: {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t kk = ((opcode & 0x00FF));

    // --- Timers

    // Fx07 - LD Vx, DT
    if (kk == 0x07)
      ch->v[x] = ch->m_delay;

    // Fx15 - LD DT, Vx
    else if (kk == 0x15)
      ch->m_delay = ch->v[x];

    // Fx18 - LD ST, Vx
    else if (kk == 0x18)
      ch->m_sound = ch->v[x];

    // --- Keyboard

    // Fx0A - LD Vx, K
    else if (kk == 0x0A) {
      uint8_t pressed = 0;

      for (uint16_t i = 0; i < 16; i++) {
        if (ch->key_state[i] == 1) {
          pressed = 1;
          ch->v[x] = i;
          break;
        }
      }

      if (!pressed)
        return;
    }

    // Fx1E - ADD I, Vx
    else if (kk == 0x1E)
      ch->m_index += ch->v[x];

    // Fx29 - LD F, Vx
    else if (kk == 0x29)
      ch->m_index = ch->v[x] * 5; // ???

    // Fx33 - LD B, Vx
    else if (kk == 0x33) {
      uint8_t value = ch->v[x];
      ch->memory[ch->m_index] = value / 100;           // hundreds
      ch->memory[ch->m_index + 1] = (value / 10) % 10; // tens
      ch->memory[ch->m_index + 2] = value % 10;        // ones
    }

    // Fx55 - LD [I], Vx
    else if (kk == 0x55)
      for (size_t i = 0; i <= x; i++)
        ch->memory[ch->m_index + i] = ch->v[i];

    // Fx65 - LD Vx, [I]
    else if (kk == 0x65)
      for (size_t i = 0; i <= x; i++)
        ch->v[i] = ch->memory[ch->m_index + i];

    ch->pc += 2;
    break;
  }
  }

  // decrease delay & sound timers
  // can be moved to a separate thread
  if (ch->m_delay > 0)
    ch->m_delay -= 1;

  if (ch->m_sound > 0)
    ch->m_sound -= 1;
}