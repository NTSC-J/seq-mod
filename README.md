# seq-mod
## 概要
UNIXの 'seq' コマンドに似た機能を提供する、Linux向けのキャラクタデバイスです。
実用性とかはありません。
open(2), close(2), read(2), write(2), ioctl(2)が実装されています。

## 使い方(基本)
    % make
    # insmod seq.ko
    % echo 5 > /dev/seq
    % cat /dev/seq
    1
    2
    3
    4
    5
    % echo 5 10 > /dev/seq
    % cat /dev/seq
    5
    6
    7
    8
    9
    10
    % echo 5 2 20 > /dev/seq
    % cat /dev/seq
    5
    7
    9
    11
    13
    15
    17
    19
    # rmmod seq

## 使い方(ioctl)
ioctl(2)を経由して、デリミタの文字を切り替えたりできます。

    % echo 10 > /dev/seq
    % cd ioctl
    % make
    % ./seqctl set delimiter ' '
    % cat /dev/seq
    1 2 3 4 5 6 7 8 9 10 
    % ./seqctl set delimiter ,
    % cat /dev/seq
    1,2,3,4,5,6,7,8,9,10,
    % ./seqctl set delimiter '
    '
    % cat /dev/seq
    1
    2
    3
    4
    5
    6
    7
    8
    9
    10

## ライセンス
GNU GPL version 3 or later

