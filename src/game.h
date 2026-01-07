#ifndef GAME_H
#define GAME_H

#include "canvas.h"
#include <stdbool.h>

// Game states
typedef enum {
    STATE_TITLE,
    STATE_GET_READY,
    STATE_DRAWING,
    STATE_JUDGING,
    STATE_RESULT,
    STATE_GAMEOVER
} GameState;

// Game data
typedef struct {
    GameState state;
    Canvas canvas;
    int score;
    int current_prompt_index;
    float timer;
    float state_timer;
    bool last_result;
    bool api_error;
    const char* error_message;
} Game;

// Game constants
#define DRAW_TIME 10.0f
#define GET_READY_TIME 2.0f
#define RESULT_TIME 8.0f
#define GAMEOVER_TIME 5.0f

// Game functions
bool game_init(Game* game);
void game_cleanup(Game* game);
void game_update(Game* game);
void game_draw(const Game* game);
void game_reset(Game* game);
void game_next_prompt(Game* game);

// Global texture for character
extern Texture2D ankobinary_tex;

#endif // GAME_H
