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
    case 0x00E0: // CLS: Clear the screen
      memset(chip->display, 0, sizeof(chip->display));
      break;
    case 0x00EE: // RET: Return from subroutine
      chip->pc = chip->stack[--chip->sp];
      break;
    }
    break;
  case 0x1000: // JP addr: Jump to address NNN
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
  case 0xD000: // DRW Vx, Vy, nibble: Draw sprite
    // draw_sprite(chip, x, y, n);
    break;
  }
}
