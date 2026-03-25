#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>

SDL_Window *Window;
SDL_Renderer *renderer;

typedef struct {
  uint8_t V[16];
  unsigned char memory[4096];
  unsigned char stack[48];

} cpu_specs;

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
