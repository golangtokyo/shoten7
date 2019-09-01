= 準標準パッケージ（golang.org/x）の早めぐり

#@# textlint-disable

== はじめに

#@# textlint-enable

バックエンドエンジニアをGoを利用している@<code>{@budougumi0617}@<fn>{bd617_twitter}です。
本章では2019年9月現在@<code>{golang.org/x}配下にある準公式パッケージについて紹介したいと思います。



//footnote[bd617_twitter][@<href>{https://twitter.com/budougumi0617}]

== 本章の概要・ゴール
本章では、Goの準公式パッケージとして配信されているリポジトリにどんなものリポジトリがあるのかを紹介します。
リポジトリの中にはGoの今後のバージョンのために試験的に作成されたパッケージから、実務での利用に耐えうるパッケージまでさまざまなコードが含まれています。
中には実際に実務でGoを開発する際にほぼ確実に利用するであろうコマンドラインツールも含まれています。
このような準公式パッケージを学習することで、本章では次のようなことを学ぶことをゴールとします。

 * 準公式パッケージで提供される各種ツールを知ることで日々の開発の生産性を上げる
 * 準公式パッケージで提供されるコードを利用することでコーディングの品質を上げる（車輪の再発明を避ける）
 * 今後リリースされるGoのバージョンに取り込まれるであろう、新しいGoの仕組みを先取りして学習する

まず、@<hd>{abst}では@<code>{golang.org/x}配下にあるパッケージの位置づけを簡単に述べます。
@<hd>{summary}では、@<code>{golang.org/x}配下にあるすべてのパッケージの簡単な紹介を述べます。
最後に@<hd>{detail}で実践的な実装を含んだパッケージの詳細とサンプルコードを記載します。

=={abst} @<code>{golang.org/x}配下にあるパッケージとは
@<code>{golang/x}パッケージという準公式パッケージがあることをご存知でしょうか。
準公式パッケージは公式パッケージに比べて互換性（@<i>{compatibility requirements}）の基準が低いなどの制約はありますが、便利なものばかりです。
また、Goに大きな新機能が追加される前の試験的な実装もこちらで公開されます。
例を挙げると、Go1.7で@<code>{context}パッケージが追加される前は@<code>{golang.org/x/net/context}パッケージの@<code>{context.Context}でコンテキスト情報を扱っていました。
最近ではGo Modules導入前の@<code>{vgo}、まだ@<code>{xerrors}なども準公式パッケージです。


TODO: もうちょっと練って書く

=={summary} @<code>{golang.org/x}で提供されているパッケージ一覧
それでは、早速@<code>{golang.org/x}から提供されているパッケージを見ていきましょう。
サブパッケージまでひとつひとつ紹介していくと途方のない数になってしまうため、ここでは@@<i>{GitHub}上のリポジトリ単位で紹介していきます。


=== @<code>{golang.org/x/arch}パッケージ
 * https://godoc.org/golang.org/x/arch
 * https://github.com/golang/arch

マシンアーキテクチャ依存のコードが同梱されている。Goのツールチェインで使われている。

=== @<code>{golang.org/x/benchmarks}パッケージ
 * https://godoc.org/golang.org/x/benchmarks
 * https://github.com/golang/benchmarks


ベンチマークを実行するツールがいくつか含まれているリポジトリ。
ベンチマークの流れはドライバーで実装されている。ツール作る際の共通化の参考になりそう
https://github.com/golang/benchmarks/blob/master//driver/driver.go

GCについての知見？
https://twitter.com/budougumi0617/status/1163561278335406080

=== @<code>{golang.org/x/blog}パッケージ
 * https://godoc.org/golang.org/x/blog
 * https://github.com/golang/blog

Go Blogのソースが置かれているリポジトリ。
正直Go Blogは検索性があまり高くない。ブログ内容を検索したいとき、手元でgrepしたり、GitHub上で検索できる。
 * https://blog.golang.org

=== @<code>{golang.org/x/build}パッケージ
 * https://godoc.org/golang.org/x/build
 * https://github.com/golang/build

=== @<code>{golang.org/x/crypto}パッケージ
 * https://godoc.org/golang.org/x/crypto
 * https://github.com/golang/crypto

標準パッケージに含まれていない暗号形式の実装が含まれている。
暗号化は自分で実装せず提供されているライブラリを使いましょう。

=== @<code>{golang.org/x/debug}パッケージ
 * @<href>{https://godoc.org/golang.org/x/debug}
 * @<href>{https://github.com/golang/debug}


debugツールであるviewcoreコマンドの実装が入っている。
コアファイルの解析方法を知りたいときに役立つのかもしれない。
正直delveで良いと思う。

viewcoreコマンドの挙動は以下のように試すことができる。
macOS Mojaveではうまくコアダンプファイルを作れなかったのでDockerでLinux環境を用意することをオススメします。
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


コアファイルの取得方法はここ。
https://golang.org/doc/gdb

=== @<code>{golang.org/x/exp}パッケージ
https://godoc.org/golang.org/x/exp
https://github.com/golang/exp

試験的に実装されたコードが内包されています。
例を挙げると、x/xerrorsも元はこのリポジトリで実装されていました。
定期的にチェックしておくと、Goの新しい機能を先取りできるかも？

//cmd{
$ git log --oneline wrap.go
3ee3066 xerrors: remove useless condition checking in Is
3850056 xerrors: fix Is panics if target is uncomparable
1f06c39 xerrors: fix crash in As
a5947ff xerrors: require that error be type assertable to As's target
334af84 xerrors: check for nil pointers in As's target
4e09f5a xerrors: make As match on assignability
20feca1 xerrors: copied files from x/exp/errors
//}

=== @<code>{golang.org/x/image}パッケージ
 * https://godoc.org/golang.org/x/image
 * https://github.com/golang/image

標準パッケージでサポートしていない形式の実装も入っている。
フォントもサポートしていたり、drawサブパッケージが便利。
https://qiita.com/tebakane/items/b7a47379659d42364c8d
https://text.baldanders.info/golang/resize-image/

=== @<code>{golang.org/x/lint}パッケージ
 * https://godoc.org/golang.org/x/lint
 * https://github.com/golang/lint

golintコマンドの実装

=== @<code>{golang.org/x/mobile}パッケージ
 * https://godoc.org/golang.org/x/mobile
 * https://github.com/golang/mobile

gomobileの実装
GoでiOSアプリやAndroidアプリを作ることができる

=== @<code>{golang.org/x/mod}パッケージ
 * https://godoc.org/golang.org/x/mod
 * https://github.com/golang/mod

go modulesに関するコードだけど今も最新仕様に追従できているのかは謎。
ちゃんと更新履歴を確認しておく必要がある。

=== @<code>{golang.org/x/net}パッケージ
 * https://godoc.org/golang.org/x/net
 * https://github.com/golang/net

ネットワーク関連のexpな実装が多く含まれている。http2、websocketとか。 
ちょっとサンプルコード書きたいなー。
もしくはgRPCとかkubernetsで使われているかな？

WebSocketについてはgorillaパッケージなどのほうが優れていることが言及されている。
golang.org/x/net/websocket
https://godoc.org/golang.org/x/net/websocket


LimitListenerなどがある。
https://heartbeats.jp/hbblog/2015/10/golang-limitlistener.html

テストの参考になりそう。
https://github.com/golang/net/blob/master/nettest/conntest.go

=== @<code>{golang.org/x/oauth2}パッケージ
 * https://godoc.org/golang.org/x/oauth2
 * https://github.com/golang/oauth2

OAUth2.0を利用するためのリポジトリ。メジャーな企業の設定も入っている。appengineらへんはちょっと特殊な仕様になっているはず。
RoundTripperらへんの実装の参考になる。

=== @<code>{golang.org/x/perf}パッケージ
 * https://godoc.org/golang.org/x/perf
 * https://github.com/golang/perf

=== @<code>{golang.org/x/sync}パッケージ
 * https://godoc.org/golang.org/x/sync
 * https://github.com/golang/sync

`sync`パッケージの補完的なパッケージ。`syncmap.Map`は標準パッケージに取り込まれている。

=== @<code>{golang.org/x/text}パッケージ
 * https://godoc.org/golang.org/x/text
 * https://github.com/golang/text

`text`パッケージの補完的なパッケージ

golang.org/x/text/transformパッケージを使う #golang
https://tech.mercari.com/entry/2017/12/05/154907

=== @<code>{golang.org/x/time}パッケージ
 * https://godoc.org/golang.org/x/time
 * https://github.com/golang/time

`time`パッケージに補足的なパッケージです。現在提供されているのは`time/rate`パッケージのみ。

=== @<code>{golang.org/x/tools}パッケージ
 * https://godoc.org/golang.org/x/tools
 * https://github.com/golang/tools

20以上のツールや静的解析で利用する型などが含まれている。

=== @<code>{golang.org/x/tour}パッケージ
 * https://godoc.org/golang.org/x/tour
 * https://github.com/golang/tour

A Tour of GoのWebサイトの実装。
誤字や内容の修正が必要な場合はこのリポジトリで。

=== @<code>{golang.org/x/vgo}パッケージ
 * https://godoc.org/golang.org/x/vgo
 * https://github.com/golang/vgo

Go1.10のころに発表されたGoのModules思想のプロトタイプである`vgo`コマンドの実装。

https://research.swtch.com/vgo

=== @<code>{golang.org/x/website}パッケージ
 * https://godoc.org/golang.org/x/website
 * https://github.com/golang/website

Goの公式サイトのソースが管理されているリポジトリ。
https://golang.org

https://google.golang.org/ から各godocへリダイレクトする簡易Webサーバなどの実装もある。



=== @<code>{golang.org/x/xerrors}パッケージ
 * https://godoc.org/golang.org/x/xerrors
 * https://github.com/golang/xerrors

新しいエラーの仕組み。

=={detail} @<code>{golang.org/x}で提供されているパッケージを使ったコーディング
ここまでで2019年9月現在存在する@<code>{golang.org/x}配下のパッケージを俯瞰的に確認しました。
本節では、実際に業務やOSSでも利用できる実用的なパッケージの利用方法を紹介します。

=== @<code>{golang.org/x/oauth2}

@<code>{golang.org/x/oauth2}パッケージはGoでOAuth2.0形式の認証認可を扱うためのパッケージです。
@<code>{GitHub}や@<code>{Google}、@<code>{PayPal}など各社の認可エンドポイントのURLが定義された構造体オブジェクトも含まれています。

//list[github_endpoint][GitHubのOAuth2.0用のエンドポイント]{
// Copyright 2014 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// Package github provides constants for using OAuth2 to access Github.
package github // import "golang.org/x/oauth2/github"

import (
	"golang.org/x/oauth2"
)

// Endpoint is Github's OAuth 2.0 endpoint.
var Endpoint = oauth2.Endpoint{
	AuthURL:  "https://github.com/login/oauth/authorize",
	TokenURL: "https://github.com/login/oauth/access_token",
}
//}

実際に使うときは@<code>{TokenSource}構造体を起点に実装を行ないます。

TODO: コードを貼る


oauth2パッケージの設計は利用者に対してとても親切な点があります。
それは、利用者がどこまでoauth2パッケージをどのレベルで利用したいか選択できる点です。
一番簡単な方法は、@<code>{TokenSource}オブジェクトからクライアントを生成する方法です。
TODO: コードを貼る

また、トークン情報を別の用途で利用する場合はTokenオブジェクトを生成します。これも自動的にトークンリフレッシュが行われます。

TODO: コードを貼る

最後はRoundTripperインターフェイスを実装したTransportオブジェクトを利用する方法です。
こちらをHttpClientに用いることでOAuth2以外の機能を付与した状態でClientを利用できます。
TODO: コードを貼る



TODO: 実用系のパッケージを使ったサンプルコードを時間があるかぎり書いていく。


== まとめ

TODO: 準公式パッケージを使ってよいGoライフを的な感じでまとめる。
