#include "canvas.h"
#include <stdlib.h>
#include <math.h>



void canvas_init(Canvas* canvas) {
    canvas->texture = LoadRenderTexture(CANVAS_WIDTH, CANVAS_HEIGHT);
    canvas->brush_size = 8;
    canvas->current_color = (Color){0, 0, 255, 255};  // Pure blue (same as background)
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
    // Brush size change (mouse wheel)
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        canvas->brush_size += (int)wheel * 2;
        if (canvas->brush_size < 2) canvas->brush_size = 2;
        if (canvas->brush_size > 40) canvas->brush_size = 40;
    }
    
    // Clear with C key
    if (IsKeyPressed(KEY_C)) {
        canvas_clear(canvas);
    }
    
    Vector2 mouse = GetMousePosition();
    
    // Check if mouse is inside canvas
    bool in_canvas = (mouse.x >= CANVAS_X && mouse.x < CANVAS_X + CANVAS_WIDTH &&
                      mouse.y >= CANVAS_Y && mouse.y < CANVAS_Y + CANVAS_HEIGHT);
    
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && in_canvas) {
        // Convert to canvas coordinates (direct mapping, no Y inversion needed for drawing)
        float canvas_x = mouse.x - CANVAS_X;
        float canvas_y = mouse.y - CANVAS_Y;
        
        BeginTextureMode(canvas->texture);
        
        if (canvas->is_drawing) {
            // Draw line from last position to current
            float last_x = canvas->last_pos.x - CANVAS_X;
            float last_y = canvas->last_pos.y - CANVAS_Y;
            
            DrawLineEx(
                (Vector2){last_x, last_y},
                (Vector2){canvas_x, canvas_y},
                (float)canvas->brush_size,
                canvas->current_color
            );
        }
        
        // Draw circle at current position
        DrawCircle((int)canvas_x, (int)canvas_y, (float)canvas->brush_size / 2, canvas->current_color);
        
        EndTextureMode();
        
        canvas->last_pos = mouse;
        canvas->is_drawing = true;
    } else {
        canvas->is_drawing = false;
    }
}

void canvas_draw(const Canvas* canvas) {
    // Canvas border
    DrawRectangle(CANVAS_X - 3, CANVAS_Y - 3, CANVAS_WIDTH + 6, CANVAS_HEIGHT + 6, DARKGRAY);
    DrawRectangle(CANVAS_X - 1, CANVAS_Y - 1, CANVAS_WIDTH + 2, CANVAS_HEIGHT + 2, WHITE);
    
    // Draw RenderTexture (flip Y for correct display)
    DrawTextureRec(
        canvas->texture.texture,
        (Rectangle){0, 0, (float)CANVAS_WIDTH, (float)-CANVAS_HEIGHT},
        (Vector2){CANVAS_X, CANVAS_Y},
        WHITE
    );
    
    // Brush preview
    Vector2 mouse = GetMousePosition();
    bool in_canvas = (mouse.x >= CANVAS_X && mouse.x < CANVAS_X + CANVAS_WIDTH &&
                      mouse.y >= CANVAS_Y && mouse.y < CANVAS_Y + CANVAS_HEIGHT);
    
    if (in_canvas) {
        DrawCircleLines((int)mouse.x, (int)mouse.y, (float)canvas->brush_size / 2, canvas->current_color);
    }
}

void canvas_draw_palette(Canvas* canvas) {
    (void)canvas;  // No palette - blue only
    // Just show brush size
    const char* text = TextFormat("Brush: %d  [Wheel] Size  [C] Clear", canvas->brush_size);
    int y = CANVAS_Y + CANVAS_HEIGHT + 30; // Move down to avoid frame overlap
    DrawText(text, CANVAS_X + 2, y + 2, 16, BLACK); // Shadow
    DrawText(text, CANVAS_X, y, 16, WHITE);
}

unsigned char* canvas_get_image_data(const Canvas* canvas, int* out_size) {
    Image image = LoadImageFromTexture(canvas->texture.texture);
    ImageFlipVertical(&image);
    unsigned char* png_data = ExportImageToMemory(image, ".png", out_size);
    UnloadImage(image);
    return png_data;
}
