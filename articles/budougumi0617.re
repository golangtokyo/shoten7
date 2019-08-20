= 動作確認用

#@# textlint-disable

## はじめに

freee株式会社でバックエンドエンジニアをしている@budougumi0617@<fn>{bd617_twitter}です。

#@# textlint-enable

//footnote[bd617_twitter][@<href>{https://twitter.com/budougumi0617}]

== 本章の目的・ゴール

== @<code>{golang.org/x}パッケージについて
@<code>{golang/x}パッケージという準公式パッケージがあることをご存知でしょうか。
例を挙げると、Go1.7で@<code>{context}パッケージが追加される前は@<code>{golang.org/x/net/context}パッケージの@<code>{context.Context}でコンテキスト情報を扱っていました。
最近ではGo Modules導入前の@<code>{vgo}、まだ@<code>{xerrors}なども準公式パッケージです。
準公式パッケージは公式パッケージに比べて互換性（@<i>{compatibility requirements}）の基準が低いなどの制約はありますが、便利なものばかりです。
本章では


//list[clash][無限ループで完了しないコード]{
package main

import "fmt"

func goroutineA(c1 chan int, c2 chan int) {
    _ = <-c2
    c1 <- 1
    return
}

func goroutineB(c1 chan int, c2 chan int) {
    fmt.Println("c1", <-c1)
    c2 <- 2
    return
}

func main() {
    c1 := make(chan int)
    c2 := make(chan int)

    go goroutineA(c1,c2)
    go goroutineB(c1,c2)

    for {
    }
}
//}

//cmd{
$ docker run --rm --name sandbox -it golang:1.12.9-buster /bin/bash
root@d5d112c8c215:/go/src# go get golang.org/x/debug/cmd/viewcore
root@d5d112c8c215:/go/src# go build -gcflags "-N -l" main.go
root@d5d112c8c215:/go/src# GOTRACEBACK=crash ./main &
[1] 786
root@d5d112c8c215:/go/src# kill -ABRT 786
root@d5d112c8c215:/go/src# SIGABRT: abort
PC=0x48739a m=0 sigcode=0

goroutine 1 [running]:
main.main()
	/go/src/main.go:22 +0xba fp=0xc000074f98 sp=0xc000074f58 pc=0x48739a
...

[1]+  Aborted                 (core dumped) GOTRACEBACK=crash ./main
root@d5d112c8c215:/go/src# viewcore core goroutines
G stacksize=800
  000000c000009918 000000c000009938 runtime.usleep+61
  000000c000009938 000000c0000099b8 runtime.sighandler+976
  000000c0000099b8 000000c000009a68 runtime.sigtrampgo+498
  000000c00002c758 000000c00002c798 main.main+190
  000000c00002c798 000000c00002c7e0 runtime.main+524
...
//}

== その他のパッケージ


