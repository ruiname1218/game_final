# お絵描き判定ゲーム (Draw & Guess)

Raylib + C言語で作成されたお絵描き判定ゲームです。Gemini Vision APIを使用してLLMが絵を判定します。

## 必要な依存関係

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential libraylib-dev libcurl4-openssl-dev libcjson-dev
```

### Fedora
```bash
sudo dnf install raylib-devel libcurl-devel cjson-devel
```

### Arch Linux
```bash
sudo pacman -S raylib curl cjson
```

## ビルド

```bash
make
```

## 実行

Gemini API Keyを環境変数に設定してから実行:

```bash
export GEMINI_API_KEY="your-api-key-here"
./draw_game
```

## 遊び方

1. スペースキーでゲーム開始
2. 画面に表示されたお題を10秒以内に描く
3. マウス左ボタンでドラッグして描画
4. マウスホイールでブラシサイズ変更
5. 画面下部のカラーパレットで色変更
6. Cキーでキャンバスクリア
7. LLMが正解と判定すれば続行、不正解でゲームオーバー
8. 最終スコアに応じて称号が決まる

## 称号一覧

| スコア | 称号 |
|--------|------|
| 0 | 才能なし |
| 1-2 | 凡人 |
| 3-4 | 小学生レベル |
| 5-6 | なかなかやるな |
| 7-9 | 芸術家の卵 |
| 10+ | 画伯 |
