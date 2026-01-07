# AI ART JUDGE 🎨🤖

AIがあなたの絵を厳しく審査する！お絵描き＆判定ゲームです。
お題に沿って絵を描くと、AI審査員（Ankobinary氏）があなたの作品を評価し、コメントをくれます。

## 動作環境

- macOS / Linux
- Raylib 5.0+
- Gemini API Key

## インストール方法

### 1. 依存ライブラリのインストール

#### macOS (Homebrew)
```bash
brew install raylib
```

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y build-essential libraylib-dev libcurl4-openssl-dev pkg-config
```

#### Linux (Rocky Linux / CentOS / RHEL 8+)
Raylibが標準リポジトリにないため、ソースからビルドする必要があります。

1. 必要なツールのインストール:
```bash
sudo dnf install -y git make gcc libcurl-devel alsa-lib-devel mesa-libGL-devel libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel
```

2. Raylibのビルドとインストール:
```bash
# ホームディレクトリなど任意の場所で実行
git clone https://github.com/raysan5/raylib.git raylib_src
cd raylib_src/src
make
sudo make install
```

### 2. ビルド
リポジトリをクローンしたディレクトリで `make` を実行します。

```bash
make
```

### 3. APIキーの設定 (重要！)
このゲームは Google Gemini API を使用して画像認識を行います。
環境変数 `GEMINI_API_KEY` にAPIキーを設定してから実行してください。

```bash
export GEMINI_API_KEY="your_api_key_here"
./draw_game
```

## 遊び方

1. タイトル画面で **SPACE** を押してスタート
2. お題が表示されるので、制限時間内に絵を描いてください
   - **左クリックドラッグ**: 描く
   - **マウスホイール**: ブラシサイズ変更
   - **Cキー**: キャンバス消去
3. 時間切れになるとAIが採点します
   - 素晴らしい作品には高得点と賞賛が！
   - 微妙な作品には...厳しいコメントが返ってくるかも？

## 開発者メモ (Developer Notes)

- `src/` : ソースコード
- `assets/` : 画像・フォントリソース
- `Makefile` : ビルド設定 (Mac/Linux自動判別)

### ライセンス
MIT License
