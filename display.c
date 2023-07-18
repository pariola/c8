#include "cpu.c"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 1024
#define HEIGHT 512

// mapped to match the original layout
int keymap[16] = {
    SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_A,
    SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C,
    SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V,
};

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

int create_window() {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  }

  /* Creates a SDL window */
  window =
      SDL_CreateWindow("chip8",                /* Title of the SDL window */
                       SDL_WINDOWPOS_CENTERED, /* Position x of the window */
                       SDL_WINDOWPOS_CENTERED, /* Position y of the window */
                       WIDTH,  /* Width of the window in pixels */
                       HEIGHT, /* Height of the window in pixels */
                       0);     /* Additional flag(s) */

  /* Checks if window has been created; if not, exits program */
  if (window == NULL) {
    fprintf(stderr, "SDL window failed to initialise: %s\n", SDL_GetError());
    return 1;
  }

  renderer = SDL_CreateRenderer(window, -1, 0);
  if (renderer == NULL) {
    fprintf(stderr, "SDL renderer failed to initialise: %s\n", SDL_GetError());
    return 1;
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_STATIC, 64, 32);
  if (texture == NULL) {
    fprintf(stderr, "SDL texture failed to initialise: %s\n", SDL_GetError());
    return 1;
  }

  return 0;
}

void close_window() {
  if (window == NULL)
    return;

  // Frees memory
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  /* Shuts down all SDL subsystems */
  SDL_Quit();
}

int listen_events(struct Chip8 *ch) {
  SDL_Event event;

  while (SDL_PollEvent(&event) > 0) {
    switch (event.type) {
    case SDL_QUIT:
      return 0;
    case SDL_KEYDOWN:
      for (uint8_t i = 0; i < 16; i++)
        if (event.key.keysym.scancode == keymap[i])
          ch->key_state[i] = 1;
      break;
    case SDL_KEYUP:
      for (uint8_t i = 0; i < 16; i++)
        if (event.key.keysym.scancode == keymap[i])
          ch->key_state[i] = 0;
      break;
    }
  }

  return 1;
}

void build_texture(struct Chip8 *ch) {
  SDL_RenderClear(renderer); // clear renderer first

  uint32_t pixels[64 * 32]; // requires flattend array

  for (size_t r = 0; r < 32; r++)
    for (size_t c = 0; c < 64; c++)
      pixels[r * 64 + c] = ch->graphics[r][c] == 1 ? 0xFFFFFFFF : 0x00000000;

  SDL_UpdateTexture(texture, NULL, pixels,
                    64 * sizeof(Uint32)); // ??? what is pitch?

  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}