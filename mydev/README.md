# mydev — シンプルな文字デバイスドライバ集

Linux カーネルモジュールとしての文字デバイスの最小例です。  
用途に応じて2種類の実装があります。

- `mydev_log.c`: 書き込まれた内容を `dmesg` にログ出力するだけ
- `mydev_buf.c`: カーネル内に 4096 バイトのバッファを持ち、書いた内容を読み戻せる

作成されるデバイスノードは `/dev/mydev` です。


## ビルド

このディレクトリで `make` を実行すると `mydev_log.ko` が生成されます。

`mydev_buf.ko` を使いたい場合は以下のように `Makefile` を編集し、対象オブジェクトを切り替えます。

```make
obj-m += mydev_log.o   # ← コメントアウトする
# obj-m += mydev_buf.o # ← コメントアウトを外す
```


## クリーンアップ

```bash
make clean
```

## 動作確認

ビルドすると `mydev_log.ko` または `mydev_buf.ko` ができます。ロードすると `/dev/mydev` が作成されます（udev 稼働前提）。

### mydev_log の動作確認

```bash
sudo insmod mydev_log.ko
sudo chmod 666 /dev/mydev
echo "hello mydev" > /dev/mydev
sudo dmesg | tail -n 5
sudo rmmod mydev_log
```

### mydev_buf の動作確認  

```bash
sudo insmod mydev_buf.ko
sudo chmod 666 /dev/mydev
echo "hello mydev" > /dev/mydev
cat /dev/mydev
sudo rmmod mydev_buf
```

## 使い方

対応するモジュールをロードして、使用します。  
使用後は、`rmmod` でアンロードします。  

```bash
sudo insmod mydev_log.ko
sudo insmod mydev_buf.ko
```

ファイルの権限でエラーが出る場合、以下を実行します。  

```bash
chmod 666 /dev/mydev
```

### mydev_log（ログ出力）

- open/release 時と write 時に `pr_info` でカーネルログへ出力します。
- read は常に EOF（0 バイト）を返します。

例:

```bash
echo "hello kernel" > /dev/mydev
dmesg | tail -n 5   # "mydev: write 'hello kernel' (len=...)" が出る
```

### mydev_buf（バッファ保持）

- `data_buf[4096]` に最後に書いたデータを上書き保存します（`write` は最大 4096 バイトを取り込み、返り値はユーザ視点で全量成功 `cnt`）。
- `read` はファイルオフセット `*ppos` に従って返し、読み切ると EOF になります。
- クリティカルセクションは `mutex` で保護しています。

例:

```bash
echo -n "abc" > /dev/mydev
sudo cat /dev/mydev   # => abc
```
