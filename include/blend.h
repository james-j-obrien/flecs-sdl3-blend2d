#ifndef RENDER_H
#define RENDER_H

#include "flecs.h"
#include <SDL3/SDL.h>
#include <blend2d.h>

typedef struct {
  double radius;
} Circle;

typedef struct {
  char *chars;
} Text;

typedef struct {
  double x, y;
  uint32_t color;
} Shape;

extern ECS_COMPONENT_DECLARE(Circle);
extern ECS_COMPONENT_DECLARE(Text);
extern ECS_COMPONENT_DECLARE(Shape);

void BlendImport(ecs_world_t *world);

#endif