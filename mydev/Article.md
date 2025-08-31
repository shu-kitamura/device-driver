# デバイスドライバ開発に入門してみた  

## 概要

今年からデバイスドライバ関係の業務をアサインされました。  
理解を深めるため、簡単なデバイスドライバを実装してみました。  
今回実装したのは、キャラクタデバイスです。また、最小の実装を試しただけで、実際のハードの制御はしていません。  

他にもブロックデバイス、ネットワークデバイスがありますが、今回は触れません。  

## 実装したデバイスドライバ

以下の2種類のデバイスドライバを実装しました。

- `mydev_log.c` - デバイスに書き込まれたデータをカーネルログに出力します。  
    動作例
    ```bash
    echo "hello kernel" > /dev/mydev
    dmesg | tail -n 5  # mydev: write 'hello kernel' (len=...) が出力される
    ```
- `mydev_buf.c` - デバイスに書き込まれたデータをバッファに保持して、後から読み戻せます。  
    動作例
    ```bash
    echo -n "abc" >/dev/mydev
    sudo cat /dev/mydev # abc が出力される
    ```

上記の動作例を実行する場合、権限の問題で以下を実行する必要があるかもしれません。  
```
chmod 666 /dev/mydev
```

## 実装した関数

以下の6種類の関数を実装することで、デバイスドライバとして動作させることができました。  

- init  
   ドライバがロードされるときに実行される関数  
    - `alloc_chrdev_region`(関数) で major/minor 番号を確保する。  
        カーネルはここで確保した番号でデバイスを認識しています。  
    - `class_create`, `device_create`(関数) で udev 経由で /dev/mydev が作成されるようにする。  
    - `cdev_init`/`cdev_add`(関数) で cdev を初期化して file_operations を関連付け  
        `file_operations` は open/read/write/release されたときに実行する関数を登録しておく構造体です。  
        以下のような構造体では open されたときに `mydev_open` が実行されます。  
        ```c
        static const struct file_operations fops = {
            .owner   = THIS_MODULE,
            .open    = mydev_open,
            .release = mydev_release,
            .read    = mydev_read,
            .write   = mydev_write,
        };
        ```
- exit
    ドライバがアンロードされるときに実行される関数  
    デバイスの削除や init 時に確保した番号の解放を行っています。  
- open
    デバイスファイルが開かれたときに実行される関数
- release
    デバイスファイルが閉じられたときに実行される関数
- read
    デバイスファイルが read されたときに実行される関数  
    例えば以下が実行されたときに動く  
    ```bash
    cat /dev/mydev
    ```
- write
    デバイスファイルが write されたときに実行される関数
    例えば以下が実行されたときに動く
    ```bash
    echo "hello" > /dev/mydev
    ```
