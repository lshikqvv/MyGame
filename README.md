# MyGames

## Porker: under construction

## Build

```bash
make clean && make
```

## CUI gameplay (5-card draw, one exchange, rematch enabled)

### 1. Start server

```bash
./build/bin/Server
```

### 2. Start 2 clients in separate terminals

```bash
./build/bin/Client
```

### 3. クライアントは起動後に `POKER` を選択してゲーム開始します（現在は `POKER` のみ）

### 4. 交換時は `1-5` をスペース区切りで入力、交換しない場合は `0`

### 5. 結果表示後、`Play again? (y/n)` で再戦可否を選べます（両者 `y` のとき次ラウンド開始）

## GUI client (same protocol as CUI client)

Build GUI client:

```bash
make gui
```

Start server, then run GUI client:

```bash
./build/bin/GuiClient --clients 2
```

- `--clients N` で同一プロセスから複数ウィンドウを起動できます（デフォルトは `2`）。
- GUIは起動直後にホーム画面を表示し、ゲーム開始/終了の導線をホームに集約しています。
- ゲーム中の交換対象はカード画像をクリックして選択します（チェックボックス/入力式は使用しません）。
- 再戦ボタンは結果表示後のみ表示されます。終了選択後はホームに戻り、ホームで終了を選ぶとプロセスが終了します。
- 右側に幅広のログ専用ペイン、中央にプレイ領域（カード/結果）を配置しています。
- 設定タブで日本語/英語の表示切替ができます。
- 1つをGUI、もう1つをCUIクライアントで接続することも可能です。

## Resource layout (multi-game ready)

- `resources/common/i18n/gui_strings.json`: GUI共通文字列（日英）
- `resources/poker/gui_config.json`: Poker用GUI設定（画像パス、スート/ランク順、バナー）
- `resources/poker/images/`: Poker用画像配置先

設定とリソースは上記ディレクトリを唯一の定義源として使用します。
ハードコードやフォールバック参照は行いません。
