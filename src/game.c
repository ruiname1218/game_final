#include "game.h"
#include "llm.h"
#include "prompts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

extern Font g_font;
extern bool g_font_loaded;

static bool used_prompts[PROMPT_COUNT] = {false};
static int used_count = 0;

// Simple text helpers
static void DrawRetroText(const char* text, int x, int y, int fontSize, Color color) {
    if (g_font_loaded) {
        DrawTextEx(g_font, text, (Vector2){(float)x, (float)y}, (float)fontSize, 2, color);
    } else {
        DrawText(text, x, y, fontSize, color);
    }
}

static int MeasureRetroText(const char* text, int fontSize) {
    if (g_font_loaded) {
        return (int)MeasureTextEx(g_font, text, (float)fontSize, 2).x;
    }
    return MeasureText(text, fontSize);
}

bool game_init(Game* game) {
    srand((unsigned int)time(NULL));
    
    if (!llm_init()) {
        game->error_message = llm_get_last_error();
        return false;
    }
    
    canvas_init(&game->canvas);
    
    game->state = STATE_TITLE;
    game->score = 0;
    game->current_prompt_index = 0;
    game->timer = 0;
    game->state_timer = 0;
    game->last_result = false;
    game->api_error = false;
    game->error_message = NULL;
    
    return true;
}

void game_cleanup(Game* game) {
    canvas_cleanup(&game->canvas);
    llm_cleanup();
}

void game_reset(Game* game) {
    game->state = STATE_TITLE;
    game->score = 0;
    game->timer = 0;
    game->state_timer = 0;
    game->last_result = false;
    game->api_error = false;
    
    memset(used_prompts, 0, sizeof(used_prompts));
    used_count = 0;
    
    canvas_clear(&game->canvas);
}

void game_next_prompt(Game* game) {
    if (used_count >= (int)PROMPT_COUNT) {
        memset(used_prompts, 0, sizeof(used_prompts));
        used_count = 0;
    }
    
    int available[PROMPT_COUNT];
    int available_count = 0;
    
    for (int i = 0; i < (int)PROMPT_COUNT; i++) {
        if (!used_prompts[i]) {
            available[available_count++] = i;
        }
    }
    
    if (available_count > 0) {
        int idx = rand() % available_count;
        game->current_prompt_index = available[idx];
        used_prompts[game->current_prompt_index] = true;
        used_count++;
    }
}

void game_update(Game* game) {
    float dt = GetFrameTime();
    
    switch (game->state) {
        case STATE_TITLE:
            if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                game_reset(game);
                game_next_prompt(game);
                game->state = STATE_GET_READY;
                game->state_timer = GET_READY_TIME;
            }
            break;
            
        case STATE_GET_READY:
            game->state_timer -= dt;
            if (game->state_timer <= 0) {
                canvas_clear(&game->canvas);
                game->state = STATE_DRAWING;
                game->timer = DRAW_TIME;
            }
            break;
            
        case STATE_DRAWING:
            canvas_update(&game->canvas);
            game->timer -= dt;
            
            if (game->timer <= 0 || IsKeyPressed(KEY_ENTER)) {
                game->state = STATE_JUDGING;
            }
            break;
            
        case STATE_JUDGING:
            {
                int image_size = 0;
                unsigned char* image_data = canvas_get_image_data(&game->canvas, &image_size);
                
                if (image_data != NULL) {
                    const char* prompt = PROMPTS[game->current_prompt_index];
                    game->last_result = llm_judge_image(image_data, image_size, prompt);
                    game->api_error = false;
                    RL_FREE(image_data);
                } else {
                    game->last_result = false;
                    game->api_error = true;
                    game->error_message = "Failed to get image data";
                }
                
                if (game->last_result) {
                    game->score++;
                }
                
                game->state = STATE_RESULT;
                game->state_timer = RESULT_TIME;
            }
            break;
            
        case STATE_RESULT:
            game->state_timer -= dt;
            if (game->state_timer <= 0) {
                if (game->last_result) {
                    game_next_prompt(game);
                    canvas_clear(&game->canvas);
                    game->state = STATE_GET_READY;
                    game->state_timer = GET_READY_TIME;
                } else {
                    game->state = STATE_GAMEOVER;
                    game->state_timer = GAMEOVER_TIME;
                }
            }
            break;
            
        case STATE_GAMEOVER:
            if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                game_reset(game);
            }
            break;
    }
}

void game_draw(const Game* game) {
    switch (game->state) {
        case STATE_TITLE:
            {
                // Character in Title (Center)
                if (ankobinary_tex.id > 0) {
                     float scale = 0.6f;
                     int tex_w = ankobinary_tex.width * scale;
                     // int tex_h = ankobinary_tex.height * scale;
                     DrawTextureEx(ankobinary_tex, (Vector2){(SCREEN_WIDTH - tex_w)/2, 160}, 0.0f, scale, WHITE);
                }

                // Big title
                const char* title = "AI ART JUDGE";
                int tw = MeasureRetroText(title, 48);
                // Shadow
                DrawRetroText(title, (SCREEN_WIDTH - tw) / 2 + 5, 80 + 5, 48, (Color){0,0,0,128});
                DrawRetroText(title, (SCREEN_WIDTH - tw) / 2, 80, 48, GOLD);
                
                // Simple instruction with shadow
                const char* start = "PRESS SPACE";
                int sw = MeasureRetroText(start, 24);
                Color col = ((int)(GetTime() * 3) % 2 == 0) ? WHITE : YELLOW;
                
                // Shadow
                DrawRetroText(start, (SCREEN_WIDTH - sw) / 2 + 3, 500 + 3, 24, BLACK);
                // Main text
                DrawRetroText(start, (SCREEN_WIDTH - sw) / 2, 500, 24, col);
            }
            break;
            
        case STATE_GET_READY:
            {
                // Just the prompt, big and centered
                const char* prompt = PROMPTS[game->current_prompt_index];
                int tw = MeasureRetroText(prompt, 64);
                DrawRetroText(prompt, (SCREEN_WIDTH - tw) / 2, 270, 64, YELLOW);
            }
            break;
            
        case STATE_DRAWING:
            {
                // Timer - with pop background
                int timer_sec = (int)game->timer + 1;
                Color tc = (timer_sec <= 3) ? RED : WHITE;
                const char* timer_str = TextFormat("%d", timer_sec);
                int ttw = MeasureRetroText(timer_str, 40);
                
                // Timer Background (Circle)
                DrawCircle(SCREEN_WIDTH / 2, 35, 30, (Color){255, 100, 100, 255}); // Pop Red Circle
                DrawCircleLines(SCREEN_WIDTH / 2, 35, 30, WHITE);
                DrawRetroText(timer_str, (SCREEN_WIDTH - ttw) / 2, 15, 40, tc);

                // Canvas Frame (Rounded Style like reference)
                int frame_pad = 20;
                int frame_x = CANVAS_X - frame_pad;
                int frame_y = CANVAS_Y - frame_pad;
                int frame_w = CANVAS_WIDTH + frame_pad * 2;
                int frame_h = CANVAS_HEIGHT + frame_pad * 2;
                
                // Yellow Outer Frame (Background/Offset)
                // Drawing it slightly larger and offset to create the layered look
                DrawRectangleRounded((Rectangle){(float)frame_x + 10, (float)frame_y + 10, (float)frame_w, (float)frame_h}, 0.1f, 10, (Color){255, 220, 80, 255}); // Pastel Yellow

                // Dark Inner Frame
                DrawRectangleRounded((Rectangle){(float)frame_x, (float)frame_y, (float)frame_w, (float)frame_h}, 0.1f, 10, (Color){40, 44, 52, 255}); // Dark Grey/Black

                // Inner Border (White line inside dark frame? Optional, keeping it simple inside)
                // Just the canvas background will contrast with the dark frame
                
                // Draw a small border around canvas to separate it from dark frame if needed, 
                // but standard canvas is white so it should pop against dark grey.
                
                // Canvas
                canvas_draw(&game->canvas);
                canvas_draw_palette((Canvas*)&game->canvas);
                
                // Current Prompt (Bottom)
                const char* prompt = PROMPTS[game->current_prompt_index];
                char draw_text[64];
                snprintf(draw_text, sizeof(draw_text), "DRAW: %s", prompt);
                int pw = MeasureRetroText(draw_text, 20);
                
                // Prompt Badge
                DrawRectangle(20, 15, pw + 20, 30, (Color){50, 200, 255, 255}); // Cyan Badge
                DrawRectangleLines(20, 15, pw + 20, 30, WHITE);
                DrawRetroText(draw_text, 30, 22, 20, WHITE);
            }
            break;
            
        case STATE_JUDGING:
            {
                const char* judging = "JUDGING...";
                int jw = MeasureRetroText(judging, 48);
                DrawRetroText(judging, (SCREEN_WIDTH - jw) / 2, 270, 48, YELLOW);
            }
            break;
            
        case STATE_RESULT:
            {
                // AI Response Bubble
                const char* full_response = llm_get_last_response();
                if (full_response && strlen(full_response) > 0) {
                    // Truncate to first sentence
                    char response[256];
                    strncpy(response, full_response, sizeof(response) - 1);
                    response[sizeof(response) - 1] = '\0';
                    
                    char* dot = strchr(response, '.');
                    if (dot) {
                        *(dot + 1) = '\0'; // Include the dot
                    }
                    // Also stop at newline if any
                    char* newline = strchr(response, '\n');
                    if (newline && (!dot || newline < dot)) {
                        *newline = '\0';
                    }

                    // Character Display
                    if (ankobinary_tex.id > 0) {
                        float scale = 0.2f; // Reduced scale to 0.2 to fit better
                        // Draw scaled character on the left
                        DrawTextureEx(ankobinary_tex, (Vector2){20, 50}, 0.0f, scale, WHITE);
                    }

                    // Bubble Shadow (Yellow Offset)
                    DrawRectangle(140, 60, SCREEN_WIDTH - 180, 100, (Color){255, 220, 80, 255});
                    
                    // Bubble Body (Opaque White)
                    DrawRectangle(130, 50, SCREEN_WIDTH - 180, 100, WHITE);
                    DrawRectangleLines(130, 50, SCREEN_WIDTH - 180, 100, BLACK);
                    
                    // Bubble Tail (Triangle pointing LEFT to character)
                    Vector2 v1 = {130, 80};
                    Vector2 v2 = {130, 120};
                    Vector2 v3 = {110, 100}; // Pointing left
                    
                    DrawTriangle(v1, v3, v2, WHITE); // Reordered vertices for CCW winding
                    DrawLineEx(v1, v3, 1, BLACK); 
                    DrawLineEx(v3, v2, 1, BLACK);
                    // Cover the right line (vertical) to merge with box
                    DrawLineEx((Vector2){130, 82}, (Vector2){130, 118}, 2, WHITE); 

                    
                    // Text rendering
                    int y = 70;
                    const char* p = response;
                    char line[128];
                    int line_idx = 0;
                    
                    while (*p) {
                        line[line_idx++] = *p++;
                        line[line_idx] = '\0';
                        
                        // Slightly wider text area (Adjusted X)
                        if (MeasureRetroText(line, 20) > SCREEN_WIDTH - 220 || *p == '\0') {
                            DrawRetroText(line, 150, y, 20, BLACK);
                            y += 25;
                            line_idx = 0;
                        }
                    }
                }

                if (game->last_result) {
                    const char* correct = "CORRECT!";
                    int cw = MeasureRetroText(correct, 64);
                    DrawRetroText(correct, (SCREEN_WIDTH - cw) / 2, 250, 64, GREEN);
                    
                    const char* score = TextFormat("%d", game->score);
                    int scw = MeasureRetroText(score, 48);
                    DrawRetroText(score, (SCREEN_WIDTH - scw) / 2, 340, 48, WHITE);
                } else {
                    const char* wrong = "WRONG!";
                    int ww = MeasureRetroText(wrong, 64);
                    DrawRetroText(wrong, (SCREEN_WIDTH - ww) / 2, 270, 64, RED);
                }
            }
            break;
            
        case STATE_GAMEOVER:
            {
                // Game over text (top)
                const char* go = "GAME OVER";
                int gow = MeasureRetroText(go, 48);
                DrawRetroText(go, (SCREEN_WIDTH - gow) / 2, 80, 48, RED);
                
                // Score
                const char* score = TextFormat("SCORE: %d", game->score);
                int scw = MeasureRetroText(score, 36);
                DrawRetroText(score, (SCREEN_WIDTH - scw) / 2, 140, 36, WHITE);
                
                // Title (rank)
                const Title* title = get_title(game->score);
                int titw = MeasureRetroText(title->title, 40);
                DrawRetroText(title->title, (SCREEN_WIDTH - titw) / 2, 190, 40, GOLD);

                // AI Response Bubble (below score)
                const char* full_response = llm_get_last_response();
                if (full_response && strlen(full_response) > 0) {
                    // Truncate to first sentence
                    char response[256];
                    strncpy(response, full_response, sizeof(response) - 1);
                    response[sizeof(response) - 1] = '\0';
                    
                    char* dot = strchr(response, '.');
                    if (dot) {
                        *(dot + 1) = '\0';
                    }
                    char* newline = strchr(response, '\n');
                    if (newline && (!dot || newline < dot)) {
                        *newline = '\0';
                    }

                    // Bubble Body
                    int box_y = 250;
                    int box_h = 100;
                    
                    // Character Display
                    if (ankobinary_tex.id > 0) {
                        float scale = 0.2f;
                        DrawTextureEx(ankobinary_tex, (Vector2){20, (float)box_y}, 0.0f, scale, WHITE);
                    }

                    // Bubble Shadow (Yellow Offset)
                    DrawRectangle(140, box_y + 10, SCREEN_WIDTH - 180, box_h, (Color){255, 220, 80, 255});
                    
                    // Bubble Body (Opaque White)
                    DrawRectangle(130, box_y, SCREEN_WIDTH - 180, box_h, WHITE);
                    DrawRectangleLines(130, box_y, SCREEN_WIDTH - 180, box_h, BLACK);

                    // Bubble Tail (Triangle pointing LEFT)
                    Vector2 v1 = {130, (float)box_y + 30};
                    Vector2 v2 = {130, (float)box_y + 70};
                    Vector2 v3 = {110, (float)box_y + 50};
                    DrawTriangle(v1, v3, v2, WHITE);
                    DrawLineEx(v1, v3, 1, BLACK); // Tail border
                    DrawLineEx(v3, v2, 1, BLACK);
                    // Cover the vertical line
                    DrawLineEx((Vector2){130, v1.y+2}, (Vector2){130, v2.y-2}, 2, WHITE); 
                    
                    int y = box_y + 20;
                    const char* p = response;
                    char line[128];
                    int line_idx = 0;
                    
                    while (*p) {
                        line[line_idx++] = *p++;
                        line[line_idx] = '\0';
                        
                        // Slightly wider text area (Adjusted X)
                        if (MeasureRetroText(line, 20) > SCREEN_WIDTH - 220 || *p == '\0') {
                            DrawRetroText(line, 150, y, 20, BLACK);
                            y += 25;
                            line_idx = 0;
                        }
                    }
                }

                // Retry
                const char* retry = "PRESS SPACE";
                int rw = MeasureRetroText(retry, 24);
                Color col = ((int)(GetTime() * 3) % 2 == 0) ? WHITE : YELLOW;
                
                // Shadow
                DrawRetroText(retry, (SCREEN_WIDTH - rw) / 2 + 3, 450 + 3, 24, BLACK);
                DrawRetroText(retry, (SCREEN_WIDTH - rw) / 2, 450, 24, col);
            }
            break;
    }
}
