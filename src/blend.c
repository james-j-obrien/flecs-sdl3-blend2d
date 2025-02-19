#include "blend.h"

#include "sdl.h"

ECS_COMPONENT_DECLARE(Circle);
ECS_COMPONENT_DECLARE(Text);
ECS_COMPONENT_DECLARE(Shape);
ECS_COMPONENT_DECLARE(Renderer);

typedef struct {
  SDL_Texture *texture;

  BLImageCore img;
  BLFontFaceCore face;
  BLFontCore font;
  BLContextCore ctx;
} Renderer;

void BlendStartup(ecs_iter_t *it) {
  ecs_world_t *world = it->world;
  App *app = (App *)ecs_field(it, App, 0);

  SDL_Texture *texture = NULL;

  BLImageCore img;
  BLFontFaceCore face;
  BLFontCore font;

  texture = SDL_CreateTexture(app->renderer, SDL_PIXELFORMAT_ARGB32,
                              SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH,
                              WINDOW_HEIGHT);

  if (!texture) {
    SDL_Log("Couldn't create texture: %s", SDL_GetError());
    app->status = SDL_APP_FAILURE;
    return;
  }
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);

  if (blImageInitAs(&img, WINDOW_WIDTH, WINDOW_HEIGHT, BL_FORMAT_PRGB32) !=
      BL_SUCCESS) {
    SDL_Log("Couldn't create blend2d image");
    app->status = SDL_APP_FAILURE;
    return;
  };

  blFontFaceInit(&face);
  if (blFontFaceCreateFromFile(&face, "assets/Roboto.ttf", 0) != BL_SUCCESS) {
    SDL_Log("Couldn't create font face");
    app->status = SDL_APP_FAILURE;
    return;
  }

  blFontInit(&font);
  if (blFontCreateFromFace(&font, &face, 40) != BL_SUCCESS) {
    SDL_Log("Couldn't create font");
    app->status = SDL_APP_FAILURE;
    return;
  }

  BLContextCore ctx;
  blContextInit(&ctx);

  ecs_singleton_set(world, Renderer, {texture, img, face, font, ctx});
}

void BlendRenderStart(ecs_iter_t *it) {
  Renderer *renderer = (Renderer *)ecs_field(it, Renderer, 0);

  blContextBegin(&renderer->ctx, &renderer->img, NULL);
  blContextClearAll(&renderer->ctx);
}

void BlendRenderEnd(ecs_iter_t *it) {
  Renderer *renderer = (Renderer *)ecs_field(it, Renderer, 0);
  App *app = (App *)ecs_field(it, App, 1);

  blContextEnd(&renderer->ctx);

  BLImageData data;
  blImageGetData(&renderer->img, &data);

  SDL_UpdateTexture(renderer->texture, NULL, data.pixelData, data.stride);
  SDL_RenderTexture(app->renderer, renderer->texture, NULL, NULL);

  blContextReset(&renderer->ctx);
}

void RenderCircle(ecs_iter_t *it) {
  Renderer *renderer = (Renderer *)ecs_field(it, Renderer, 0);
  const Shape *shape = ecs_field(it, Shape, 1);
  const Circle *circle = ecs_field(it, Circle, 2);

  for (int i = 0; i < it->count; i++) {
    BLCircle obj = {shape[i].x + shape[i].offset_x,
                    shape[i].y + shape[i].offset_y, circle[i].radius};
    blContextFillGeometryRgba32(&renderer->ctx, BL_GEOMETRY_TYPE_CIRCLE, &obj,
                                shape[i].color);
  }
}

void RenderText(ecs_iter_t *it) {
  Renderer *renderer = (Renderer *)ecs_field(it, Renderer, 0);
  const Shape *shape = ecs_field(it, Shape, 1);
  const Text *text = ecs_field(it, Text, 2);

  for (int i = 0; i < it->count; i++) {
    BLPoint point = {shape[i].x + shape[i].offset_x,
                     shape[i].y + shape[i].offset_y};
    blContextFillUtf8TextDRgba32(&renderer->ctx, &point, &renderer->font,
                                 text[i].chars, SIZE_MAX, shape[i].color);
  }
}

void BlendImport(ecs_world_t *world) {
  ECS_MODULE(world, Blend);

  ECS_COMPONENT_DEFINE(world, Circle);
  ECS_COMPONENT_DEFINE(world, Text);
  ECS_COMPONENT_DEFINE(world, Shape);
  ECS_COMPONENT_DEFINE(world, Renderer);

  ECS_SYSTEM(world, BlendStartup, EcsOnStart, sdl.App($));
  ECS_SYSTEM(world, BlendRenderStart, EcsPostUpdate, Renderer($));
  ECS_SYSTEM(world, RenderCircle, EcsPreStore, Renderer($), Shape, Circle);
  ECS_SYSTEM(world, RenderText, EcsPreStore, Renderer($), Shape, Text);
  ECS_SYSTEM(world, BlendRenderEnd, EcsOnStore, Renderer($), sdl.App($));
}
