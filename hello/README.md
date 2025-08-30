# hello: Linux カーネルモジュール (Hello World)

シンプルな Linux カーネルモジュールのサンプルです。
以下のメッセージをカーネルログへ出力します。

- `insmod` 時に `Hello world!!!`
- `rmmod` 時に `GoodBye world!!!`

## 動作環境

以下の環境で動作を確認できました。  

- カーネル : 5.15.0-1084-raspi
- OS : Ubuntu 22.04
- カーネルヘッダがインストール済み
	- 例: Debian/Ubuntu: `sudo apt-get install build-essential linux-headers-"$(uname -r)"`

## ファイル

- `hello.c`: 本体
- `Makefile`: カーネルモジュール用ビルドルール

## ビルド

このディレクトリで `make` を実行すると `hello.ko` が生成されます。

## ロード/アンロード

以下のコマンドで動作確認ができます。  

```shell
sudo insmod hello.ko
sudo rmmod hello
sudo dmesg | tail -n 5
```

出力例

```
$ sudo dmesg | tail -n 5
...
[ 1872.435903] Hello world!!!
[ 1877.249524] GoodBye world!!!
```

モジュール情報
```
$ modinfo hello.ko
filename:       /home/shusei/device-driver/hello/hello.ko
license:        GPL
srcversion:     9B1B15814AD9D563E45EB96
depends:
name:           hello
vermagic:       5.15.0-1084-raspi SMP preempt mod_unload modversions aarch64
```

## クリーンアップ

```
make clean
```


## 注意

- モジュールのロード/アンロードには管理者権限が必要です。
- 実機のカーネルバージョンに合致したヘッダが必要です。

