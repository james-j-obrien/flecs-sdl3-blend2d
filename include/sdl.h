#ifndef APP_H
#define APP_H

#include "flecs.h"
#include <SDL3/SDL.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;

  SDL_AppResult status;
} App;

extern ECS_COMPONENT_DECLARE(App);

void SdlImport(ecs_world_t *world);

#endif