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
  App *app = (App *)ecs_singleton_get(world, App);

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

  size_t size;
  void *file = SDL_LoadFile("assets/Roboto.ttf", &size);
  if (!file) {
    SDL_Log("Couldn't load font");
    app->status = SDL_APP_FAILURE;
    return;
  }

  BLArrayCore bytes;
  if (blArrayInit(&bytes, BL_OBJECT_TYPE_ARRAY_UINT8) != BL_SUCCESS) {
    SDL_Log("Failed to create byte array");
    app->status = SDL_APP_FAILURE;
    return;
  }

  if (blArrayAppendData(&bytes, file, size) != BL_SUCCESS) {
    SDL_Log("Failed to append data to byte array");
    app->status = SDL_APP_FAILURE;
    return;
  }

  BLFontDataCore data;
  blFontDataInit(&data);
  if (blFontDataCreateFromDataArray(&data, &bytes) != BL_SUCCESS) {
    SDL_Log("Failed to create font data");
    app->status = SDL_APP_FAILURE;
    return;
  }

  blFontFaceInit(&face);
  if (blFontFaceCreateFromData(&face, &data, 0) != BL_SUCCESS) {
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

  blFontDataDestroy(&data);
  blArrayDestroy(&bytes);
  SDL_free(file);

  BLContextCore ctx;
  blContextInit(&ctx);

  ecs_singleton_set(world, Renderer, {texture, img, face, font, ctx});
}

void BlendRenderStart(ecs_iter_t *it) {
  ecs_world_t *world = it->world;
  Renderer *renderer = (Renderer *)ecs_singleton_get(world, Renderer);
  App *app = ecs_get_mut(world, ecs_id(App), App);

  SDL_SetRenderDrawColor(app->renderer, 0, 0, 0,
                         SDL_ALPHA_OPAQUE); /* black, full alpha */
  SDL_RenderClear(app->renderer);           /* start with a blank canvas. */

  blContextBegin(&renderer->ctx, &renderer->img, NULL);
  blContextClearAll(&renderer->ctx);

  blContextFillAllRgba32(&renderer->ctx, 0x10000000u);
}

void BlendRenderEnd(ecs_iter_t *it) {
  ecs_world_t *world = it->world;
  Renderer *renderer = (Renderer *)ecs_singleton_get(world, Renderer);
  App *app = (App *)ecs_singleton_get(world, App);

  BLImageData data;
  blImageGetData(&renderer->img, &data);

  SDL_UpdateTexture(renderer->texture, NULL, data.pixelData, data.stride);
  SDL_RenderTexture(app->renderer, renderer->texture, NULL, NULL);

  blContextReset(&renderer->ctx);

  SDL_RenderPresent(app->renderer);
}

void RenderCircle(ecs_iter_t *it) {
  Renderer *renderer = (Renderer *)ecs_singleton_get(it->world, Renderer);
  const Shape *shape = ecs_field(it, Shape, 0);
  const Circle *circle = ecs_field(it, Circle, 1);

  for (int i = 0; i < it->count; i++) {
    BLCircle obj = {shape[i].x + shape[i].offset_x,
                    shape[i].y + shape[i].offset_y, circle[i].radius};
    blContextFillGeometryRgba32(&renderer->ctx, BL_GEOMETRY_TYPE_CIRCLE, &obj,
                                shape[i].color);
  }
}

void RenderText(ecs_iter_t *it) {
  Renderer *renderer = (Renderer *)ecs_singleton_get(it->world, Renderer);
  const Shape *shape = ecs_field(it, Shape, 0);
  const Text *text = ecs_field(it, Text, 1);

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

  ECS_SYSTEM(world, BlendStartup, EcsOnStart);
  ECS_SYSTEM(world, BlendRenderStart, EcsPostUpdate);
  ECS_SYSTEM(world, RenderCircle, EcsPreStore, Shape, Circle);
  ECS_SYSTEM(world, RenderText, EcsPreStore, Shape, Text);
  ECS_SYSTEM(world, BlendRenderEnd, EcsOnStore);
}
