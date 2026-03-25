#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_rect.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

SDL_Window *Window;
SDL_Renderer *renderer;

typedef struct {
  int width;
  int height;
} display_t;

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

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  if (argc < 2) {
    printf("Usage: %s romfile\n", argv[0]);
    return 1;
  }
  if (!init_sdl())
    exit(EXIT_FAILURE);

  renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);

  SDL_Event event;
  int running = 1;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        running = 0;
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

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

void cleanup() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(Window);
  SDL_Quit();
}

void load_rom(Chip8 *chip) {
  FILE *f = fopen(chip->rom_file, "rb");
  if (!f) {
    printf("Failed to open ROM\n");
    exit(1);
  }

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  rewind(f);

  fread(&chip->memory[0x200], 1, size, f);
  fclose(f);
}

void init_chip8(Chip8 *chip) { chip->pc = 0x200; }
