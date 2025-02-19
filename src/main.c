#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <blend2d.h>

#include <errno.h>
#include <math.h>
#include <stdio.h>

#include "blend.h"
#include "flecs.h"
#include "sdl.h"

void Move(ecs_iter_t *it) {
  Shape *shape = (Shape *)ecs_field(it, Shape, 0);
  Uint64 ticks = SDL_GetTicks();
  double offset = sin(ticks / 500.0f) * 100 + 40;

  for (int i = 0; i < it->count; i++) {
    shape[i].offset_y = offset;
  }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  ecs_world_t *world = ecs_init_w_args(argc, argv);
  *appstate = world;

  ECS_IMPORT(world, Sdl);
  ECS_IMPORT(world, Blend);

  ecs_entity_t circle = ecs_entity(world, {.name = "White Circle"});
  ecs_set(world, circle, Circle, {50.0});
  ecs_set(world, circle, Shape,
          {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0, 0, 0xFFFFFFFFu});

  ecs_entity_t text = ecs_entity(world, {.name = "Blue Text"});
  ecs_set(world, text, Text, {"Hello world!"});
  ecs_set(world, text, Shape,
          {WINDOW_WIDTH / 2 - 110, WINDOW_HEIGHT / 4, 0, 0, 0xFFBBBB00u});

  ECS_SYSTEM(world, Move, EcsOnUpdate, blend.Shape);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS; /* end the program, reporting success to the OS.
                             */
  }
  return SDL_APP_CONTINUE; /* carry on with the program! */
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  ecs_world_t *world = appstate;
  const App *app = ecs_singleton_get(world, App);
  if (app->status != SDL_APP_CONTINUE) {
    return app->status;
  }

  ecs_progress(world, 0);

  app = ecs_singleton_get(world, App);
  return app->status;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  ecs_world_t *world = appstate;
  ecs_fini(world);
}
