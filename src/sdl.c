#include "sdl.h"

ECS_COMPONENT_DECLARE(App);

void SdlStartup(ecs_iter_t *it) {
  ecs_world_t *world = it->world;
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;

  App *app = (App *)ecs_singleton_get(world, App);

  SDL_SetAppMetadata("flecs-sdl3-blend2d", "1.0",
                     "com.example.flecs-sdl3-blend2d");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    app->status = SDL_APP_FAILURE;
    return;
  }

  if (!SDL_CreateWindowAndRenderer("flecs-sdl3-blend2d", WINDOW_WIDTH,
                                   WINDOW_HEIGHT, 0, &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    app->status = SDL_APP_FAILURE;
    return;
  }

  ecs_singleton_set(world, App, {window, renderer, SDL_APP_CONTINUE});
}

void SdlImport(ecs_world_t *world) {
  ECS_MODULE(world, Sdl);

  ECS_COMPONENT_DEFINE(world, App);
  ecs_singleton_set(world, App, {.status = SDL_APP_CONTINUE});

  ECS_SYSTEM(world, SdlStartup, EcsOnStart);
}