#include "raylib.h"
#include "game.h"
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FONT_PATH "assets/PressStart2P.ttf"

Font g_font = {0};
bool g_font_loaded = false;

static Texture2D flower1_tex = {0};
static Texture2D flower2_tex = {0};
static Texture2D bg_tex = {0};
Texture2D ankobinary_tex = {0};

void DrawBackground(void) {
    if (bg_tex.id > 0) {
        Rectangle source = {0, 0, (float)bg_tex.width, (float)bg_tex.height};
        Rectangle dest = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        DrawTexturePro(bg_tex, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        ClearBackground((Color){0, 0, 255, 255});
    }
    
    float scale = 0.15f;
    
    if (flower1_tex.id > 0) {
        DrawTextureEx(flower1_tex, (Vector2){10, (SCREEN_HEIGHT - flower1_tex.height * scale) / 2}, 0.0f, scale, WHITE);
    }
    
    if (flower2_tex.id > 0) {
        DrawTextureEx(flower2_tex, (Vector2){SCREEN_WIDTH - flower2_tex.width * scale - 10, (SCREEN_HEIGHT - flower2_tex.height * scale) / 2}, 0.0f, scale, WHITE);
    }
}

int main(void) {
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "AI ART JUDGE");
    SetTargetFPS(60);
    
    g_font = LoadFontEx(FONT_PATH, 48, NULL, 0);
    if (g_font.texture.id > 0) {
        g_font_loaded = true;
        SetTextureFilter(g_font.texture, TEXTURE_FILTER_POINT);
    }
    
    flower1_tex = LoadTexture("assets/flower1.png");
    flower2_tex = LoadTexture("assets/flower2.png");
    bg_tex = LoadTexture("assets/bg2.png");
    ankobinary_tex = LoadTexture("assets/ankobinary.png");
    
    Game game = {0};
    bool init_success = game_init(&game);
    
    while (!WindowShouldClose()) {
        if (init_success) {
            game_update(&game);
        }
        
        BeginDrawing();
        DrawBackground();
        
        if (init_success) {
            game_draw(&game);
        } else {
            DrawText("Error: Failed to initialize game", 100, 250, 24, RED);
            if (game.error_message) {
                DrawText(game.error_message, 100, 290, 20, MAROON);
            }
            DrawText("Please check GEMINI_API_KEY", 100, 340, 18, WHITE);
        }
        
        EndDrawing();
    }
    
    if (init_success) {
        game_cleanup(&game);
    }
    if (g_font_loaded) {
        UnloadFont(g_font);
    }
    
    if (flower1_tex.id > 0) UnloadTexture(flower1_tex);
    if (flower2_tex.id > 0) UnloadTexture(flower2_tex);
    if (bg_tex.id > 0) UnloadTexture(bg_tex);
    if (ankobinary_tex.id > 0) UnloadTexture(ankobinary_tex);
    
    CloseWindow();
    return 0;
}
