#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_rect.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MEMORY 4096
#define REGISTERS 16
#define STACK 4096
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define KEYCOUNT 16
#define FREQUENCY 60

uint8_t fontset[80] = {
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
SDL_Window *Window;
SDL_Renderer *renderer;

// typedef struct {
//   int width;
//   int height;
// } display_t;

typedef struct {
  uint8_t memory[4096]; // RAM
  uint8_t V[16];        // Registers V0-VF
  uint16_t I;           // Index register
  uint16_t pc;          // Program counter

  uint16_t stack[16]; // Stack
  uint8_t sp;         // Stack pointer

  uint8_t delay_timer;
  uint8_t sound_timer;

  uint8_t display[64 * 32]; // Screen pixels
  uint8_t keypad[16];       // Input
  const char *rom_file;
} Chip8;

void init_chip8(Chip8 *chip);
void load_rom(Chip8 *c);
bool init_sdl(void);
void cleanup();
void init_chip8(Chip8 *chip);
void emulate(Chip8 *chip, uint16_t opcode);

// void render_display(Chip8 *chip);
uint16_t fetch(Chip8 *chip);

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s romfile\n", argv[0]);
    return 1;
  }

  if (!init_sdl())
    exit(EXIT_FAILURE);

  // renderer for display
  renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);

  // for event handling
  SDL_Event event;

  Chip8 chip;
  chip.rom_file = argv[1];

  init_chip8(&chip);
  load_rom(&chip);

  int running = 1;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        running = 0;
    }
    fetch(&chip);

    // emulate(&chip);
    // render_display(&chip);

    SDL_RenderPresent(renderer);
    SDL_Delay(1000 / FREQUENCY); // frequency set to 60Hz
  }
  cleanup();
  return EXIT_SUCCESS;
}

bool init_sdl(void) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("could not initialize SDL! %s\n", SDL_GetError());
    return false;
  }
  Window = SDL_CreateWindow("silicon8", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 1000, 1000, 0);
  if (!Window) {
    printf("could not create a window : %s", SDL_GetError());
    return false;
  }
  return true;
}

// do memory cleanup here only if later required
void cleanup() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(Window);
  SDL_Quit();
}

// edge cases still left and probably this section is causing segfaults
void load_rom(Chip8 *chip) {
  FILE *f = fopen(chip->rom_file, "rb");
  if (!f) {
    printf("Failed to open ROM\n");
    exit(1);
  }

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  rewind(f);

  if (size > (4096 - 0x200)) {
    printf("ROM too large!\n");
    exit(1);
  }

  fread(&chip->memory[0x200], 1, size, f);
  fclose(f);
}

// memcpy is the better method (probably) implement later
void init_chip8(Chip8 *chip) {
  chip->pc = 0x200;
  chip->I = 0;
  chip->sp = 0;

  for (int i = 0; i < 4096; i++)
    chip->memory[i] = 0;

  for (int i = 0; i < 16; i++) {
    chip->V[i] = 0;
    chip->stack[i] = 0;
    chip->keypad[i] = 0;
  }

  for (int i = 0; i < 64 * 32; i++)
    chip->display[i] = 0;

  chip->delay_timer = 0;
  chip->sound_timer = 0;

  // LOAD FONTSET
  for (int i = 0; i < 80; i++) {
    chip->memory[0x50 + i] = fontset[i];
  }
}

uint16_t fetch(Chip8 *chip) {
  if (chip->pc >= 4094) {
    printf("PC out of bounds!\n");
    exit(1);
  }

  // each opcode is combination of two 8bit instructions
  uint16_t opcode = (chip->memory[chip->pc] << 8) | chip->memory[chip->pc + 1];
  chip->pc += 2;
  printf("Opcode: %04X\n", opcode);
  return opcode;
}

void emulate(Chip8 *chip, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  uint8_t n = (opcode & 0x000F);
  uint8_t nn = (opcode & 0x00FF);
  uint16_t nnn = (opcode & 0x0FFF);
  switch (opcode & 0xF000) {
  case 0x0000:
    switch (opcode) {
    case 0x00E0: // CLS: Clear the screen 0x00E0
      memset(chip->display, 0, sizeof(chip->display));
      break;
    case 0x00EE: // RET: Return from subroutine
      if (chip->sp > 0)
        chip->pc = chip->stack[--chip->sp];
      break;
    }
    break;
  case 0x1000: // JP addr: Jump to address NNN
    chip->pc = nnn;
    break;
  case 0x2000: // CALL addr
    chip->stack[chip->sp++] = chip->pc;
    chip->pc = nnn;
    break;
  case 0x6000: // LD Vx, byte: Set VX to NN
    chip->V[x] = nn;
    break;
  case 0x7000: // ADD Vx, byte: Add NN to VX (no carry)
    chip->V[x] += nn;
    break;
  case 0xA000: // LD I, addr: Set Index register to NNN
    chip->I = nnn;
    break;
  case 0xB000: // JP V0, addr
    chip->pc = nnn + chip->V[0];
    break;
  case 0xC000: // RND Vx, byte
    chip->V[x] = (rand() % 256) & nn;
    break;
  case 0x3000: // Skip next instruction if Vx == NN
    if (chip->V[x] == nn) {
      chip->pc += 2;
    }
    break;

  case 0x4000: // Skip if Vx != NN
    if (chip->V[x] != nn) {
      chip->pc += 2;
    }
    break;

  case 0x5000: // Skip if Vx == Vy
    if (chip->V[x] == chip->V[y]) {
      chip->pc += 2;
    }
    break;

  case 0x9000: // Skip if Vx != Vy
    if (chip->V[x] != chip->V[y]) {
      chip->pc += 2;
    }
    break;
  case 0x8000:
    if (n == 0x0) { // 8XY0 → Vx = Vy
      chip->V[x] = chip->V[y];
    }

    else if (n == 0x1) { // 8XY1 → Vx = Vx OR Vy
      chip->V[x] |= chip->V[y];
    }

    else if (n == 0x2) { // 8XY2 → Vx = Vx AND Vy
      chip->V[x] &= chip->V[y];
    }

    else if (n == 0x3) { // 8XY3 → Vx = Vx XOR Vy
      chip->V[x] ^= chip->V[y];
    }

    else if (n == 0x4) { // 8XY4 → ADD with carry
      uint16_t sum = chip->V[x] + chip->V[y];
      chip->V[0xF] = (sum > 255);
      chip->V[x] = sum & 0xFF;
    }

    else if (n == 0x5) { // 8XY5 → Vx = Vx - Vy
      chip->V[0xF] = (chip->V[x] > chip->V[y]);
      chip->V[x] -= chip->V[y];
    }

    else if (n == 0x6) { // 8XY6 → SHIFT RIGHT
      chip->V[0xF] = chip->V[x] & 0x1;
      chip->V[x] >>= 1;
    }

    else if (n == 0x7) { // 8XY7 → Vx = Vy - Vx
      chip->V[0xF] = (chip->V[y] > chip->V[x]);
      chip->V[x] = chip->V[y] - chip->V[x];
    }

    else if (n == 0xE) { // 8XYE → SHIFT LEFT
      chip->V[0xF] = (chip->V[x] & 0x80) >> 7;
      chip->V[x] <<= 1;
    }
    break;

  case 0xE000:
    if ((opcode & 0xF0FF) == 0xE09E) { // SKP Vx
      if (chip->keypad[chip->V[x]]) {
        chip->pc += 2;
      }
    }

    else if ((opcode & 0xF0FF) == 0xE0A1) { // SKNP Vx
      if (!chip->keypad[chip->V[x]]) {
        chip->pc += 2;
      }
    }
    break;

  case 0xF000:
    if ((opcode & 0xF0FF) == 0xF007) { // LD Vx, DT
      chip->V[x] = chip->delay_timer;
    }

    else if ((opcode & 0xF0FF) == 0xF00A) { // LD Vx, K
      int key_pressed = 0;
      for (int i = 0; i < 16; i++) {
        if (chip->keypad[i]) {
          chip->V[x] = i;
          key_pressed = 1;
        }
      }
      if (!key_pressed) {
        chip->pc -= 2;
      }
    }

    else if ((opcode & 0xF0FF) == 0xF015) { // LD DT, Vx
      chip->delay_timer = chip->V[x];
    }

    else if ((opcode & 0xF0FF) == 0xF018) { // LD ST, Vx
      chip->sound_timer = chip->V[x];
    }

    else if ((opcode & 0xF0FF) == 0xF01E) { // ADD I, Vx
      chip->I += chip->V[x];
    }

    else if ((opcode & 0xF0FF) == 0xF029) { // LD F, Vx
      chip->I = 0x50 + (chip->V[x] * 5);
    }

    else if ((opcode & 0xF0FF) == 0xF033) { // LD B, Vx
      uint8_t value = chip->V[x];
      chip->memory[chip->I] = value / 100;
      chip->memory[chip->I + 1] = (value / 10) % 10;
      chip->memory[chip->I + 2] = value % 10;
    }

    else if ((opcode & 0xF0FF) == 0xF055) { // LD [I], Vx
      for (int i = 0; i <= x; i++) {
        chip->memory[chip->I + i] = chip->V[i];
      }
    }

    else if ((opcode & 0xF0FF) == 0xF065) { // LD Vx, [I]
      for (int i = 0; i <= x; i++) {
        chip->V[i] = chip->memory[chip->I + i];
      }
    }
    break;

  case 0xD000: // DRW Vx, Vy, nibble: Draw sprite
    // draw_sprite(chip, x, y, n);
    break;
  }
}
