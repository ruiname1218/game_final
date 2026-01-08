#ifndef CANVAS_H
#define CANVAS_H

#include "raylib.h"
#include <stdbool.h>

#define CANVAS_WIDTH 480
#define CANVAS_HEIGHT 320
#define CANVAS_X 160
#define CANVAS_Y 140

#define PALETTE_COLORS 8

typedef struct {
    RenderTexture2D texture;
    int brush_size;
    Color current_color;
    Vector2 last_pos;
    bool is_drawing;
} Canvas;

void canvas_init(Canvas* canvas);

void canvas_cleanup(Canvas* canvas);

void canvas_clear(Canvas* canvas);

void canvas_update(Canvas* canvas);

void canvas_draw(const Canvas* canvas);

void canvas_draw_palette(Canvas* canvas);

unsigned char* canvas_get_image_data(const Canvas* canvas, int* out_size);

#endif // CANVAS_H
