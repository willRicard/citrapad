#include "joystick.h"

#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

static SDL_Joystick *joystick = NULL;
static int axis_x = 3;
static int axis_y = 4;

void joystick_init() {
  if (SDL_NumJoysticks() == 0) {
    fputs("ERROR: Joystick not found.", stderr);
    exit(EXIT_FAILURE);
  }

  joystick = SDL_JoystickOpen(0);
  if (joystick == NULL) {
    fputs("ERROR: Error accessing joystick.", stderr);
    fprintf(stderr, "ERROR: %s\n", SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE);
  }
}

void joystick_quit() {
	SDL_JoystickClose(joystick);
}

void joystick_update(uint8_t *is_active, uint16_t *x, uint16_t *y) {
  SDL_JoystickUpdate();
  *is_active = !SDL_JoystickGetButton(joystick, 9);
  *x = ((1 << 14) + SDL_JoystickGetAxis(joystick, axis_x)) >> 1;
  *y = ((1 << 14) + SDL_JoystickGetAxis(joystick, axis_y)) >> 1;
}
