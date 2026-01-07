#ifndef CANVAS_H
#define CANVAS_H

#include "raylib.h"
#include <stdbool.h>

// キャンバスサイズ
#define CANVAS_WIDTH 480
#define CANVAS_HEIGHT 320
#define CANVAS_X 160
#define CANVAS_Y 140

// カラーパレット
#define PALETTE_COLORS 8

// キャンバス構造体
typedef struct {
    RenderTexture2D texture;
    int brush_size;
    Color current_color;
    Vector2 last_pos;
    bool is_drawing;
} Canvas;

// キャンバス初期化
void canvas_init(Canvas* canvas);

// キャンバス終了処理
void canvas_cleanup(Canvas* canvas);

// キャンバスクリア
void canvas_clear(Canvas* canvas);

// キャンバス更新（描画処理）
void canvas_update(Canvas* canvas);

// キャンバス描画
void canvas_draw(const Canvas* canvas);

// カラーパレット描画と選択処理
void canvas_draw_palette(Canvas* canvas);

// キャンバスの画像データを取得（PNG形式）
// 戻り値: PNGデータ（呼び出し側でRL_FREE必要）、サイズはout_sizeに格納
unsigned char* canvas_get_image_data(const Canvas* canvas, int* out_size);

#endif // CANVAS_H
