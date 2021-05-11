#include <stdio.h>
#include <stdlib.h>

#include "citrapad.h"
#include "joystick.h"

#include <SDL.h>

int main(void) {
  if (SDL_Init(SDL_INIT_JOYSTICK)) {
    fputs("ERROR: Error initializing SDL.", stderr);
    fprintf(stderr, "ERROR: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  joystick_init();

  citrapad_init();

  citrapad_loop();

  citrapad_quit();

  joystick_quit();

  SDL_Quit();

  return 0;
}
