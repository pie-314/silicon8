#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>

SDL_Window *Window;
SDL_Renderer *renderer;

typedef struct {
  int width;
  int height;
} Screen;

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
} Chip8;

void init_chip8(Chip8 *chip); // initalize all cpu vals
void init_sdl(Screen *s);     // initalization of screen

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  Window = SDL_CreateWindow("silicon8", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 1000, 1000, 0);

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

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(Window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
