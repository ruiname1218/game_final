#include "canvas.h"
#include <stdlib.h>
#include <math.h>



void canvas_init(Canvas* canvas) {
    canvas->texture = LoadRenderTexture(CANVAS_WIDTH, CANVAS_HEIGHT);
    canvas->brush_size = 8;
    canvas->current_color = (Color){0, 0, 255, 255};
    canvas->last_pos = (Vector2){0, 0};
    canvas->is_drawing = false;
    
    BeginTextureMode(canvas->texture);
    ClearBackground(WHITE);
    EndTextureMode();
}

void canvas_cleanup(Canvas* canvas) {
    UnloadRenderTexture(canvas->texture);
}

void canvas_clear(Canvas* canvas) {
    BeginTextureMode(canvas->texture);
    ClearBackground(WHITE);
    EndTextureMode();
}

void canvas_update(Canvas* canvas) {
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        canvas->brush_size += (int)wheel * 2;
        if (canvas->brush_size < 2) canvas->brush_size = 2;
        if (canvas->brush_size > 40) canvas->brush_size = 40;
    }
    
    if (IsKeyPressed(KEY_C)) {
        canvas_clear(canvas);
    }
    
    Vector2 mouse = GetMousePosition();
    
    bool in_canvas = (mouse.x >= CANVAS_X && mouse.x < CANVAS_X + CANVAS_WIDTH &&
                      mouse.y >= CANVAS_Y && mouse.y < CANVAS_Y + CANVAS_HEIGHT);
    
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && in_canvas) {
        float canvas_x = mouse.x - CANVAS_X;
        float canvas_y = mouse.y - CANVAS_Y;
        
        BeginTextureMode(canvas->texture);
        
        if (canvas->is_drawing) {
            float last_x = canvas->last_pos.x - CANVAS_X;
            float last_y = canvas->last_pos.y - CANVAS_Y;
            
            DrawLineEx(
                (Vector2){last_x, last_y},
                (Vector2){canvas_x, canvas_y},
                (float)canvas->brush_size,
                canvas->current_color
            );
        }
        
        DrawCircle((int)canvas_x, (int)canvas_y, (float)canvas->brush_size / 2, canvas->current_color);
        
        EndTextureMode();
        
        canvas->last_pos = mouse;
        canvas->is_drawing = true;
    } else {
        canvas->is_drawing = false;
    }
}

void canvas_draw(const Canvas* canvas) {
    DrawRectangle(CANVAS_X - 3, CANVAS_Y - 3, CANVAS_WIDTH + 6, CANVAS_HEIGHT + 6, DARKGRAY);
    DrawRectangle(CANVAS_X - 1, CANVAS_Y - 1, CANVAS_WIDTH + 2, CANVAS_HEIGHT + 2, WHITE);
    
    DrawTextureRec(
        canvas->texture.texture,
        (Rectangle){0, 0, (float)CANVAS_WIDTH, (float)-CANVAS_HEIGHT},
        (Vector2){CANVAS_X, CANVAS_Y},
        WHITE
    );
    
    Vector2 mouse = GetMousePosition();
    bool in_canvas = (mouse.x >= CANVAS_X && mouse.x < CANVAS_X + CANVAS_WIDTH &&
                      mouse.y >= CANVAS_Y && mouse.y < CANVAS_Y + CANVAS_HEIGHT);
    
    if (in_canvas) {
        DrawCircleLines((int)mouse.x, (int)mouse.y, (float)canvas->brush_size / 2, canvas->current_color);
    }
}

void canvas_draw_palette(Canvas* canvas) {
    (void)canvas;
    const char* text = TextFormat("Brush: %d  [Wheel] Size  [C] Clear", canvas->brush_size);
    int y = CANVAS_Y + CANVAS_HEIGHT + 30;
    DrawText(text, CANVAS_X + 2, y + 2, 16, BLACK);
    DrawText(text, CANVAS_X, y, 16, WHITE);
}

unsigned char* canvas_get_image_data(const Canvas* canvas, int* out_size) {
    Image image = LoadImageFromTexture(canvas->texture.texture);
    ImageFlipVertical(&image);
    unsigned char* png_data = ExportImageToMemory(image, ".png", out_size);
    UnloadImage(image);
    return png_data;
}
