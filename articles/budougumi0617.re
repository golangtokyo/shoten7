= 準標準パッケージ（golang.org/x）の早めぐり

#@# textlint-disable

== はじめに

#@# textlint-enable

freee株式会社でバックエンドエンジニアをしている@<tt>{@budougumi0617}@<fn>{bd617_twitter}です。
本章では2019年9月現在@<code>{golang.org/x}配下にある準標準パッケージについて紹介します。



//footnote[bd617_twitter][@<href>{https://twitter.com/budougumi0617}]

== 本章の概要・ゴール
本章では、Goの準標準パッケージとして配信されているリポジトリにどんなものリポジトリがあるのかを紹介します。
リポジトリの中にはGoの今後のバージョンのために試験的に作成されたパッケージから、実務での利用に耐えうるパッケージまでさまざまなコードが含まれています。
中には実際に実務でGoを開発する際にほぼ確実に利用するであろうコマンドラインツールも含まれています。
このような準標準パッケージを学習することで、本章では次のようなことを学ぶことをゴールとします。

 * 準標準パッケージで提供される各種ツールを知ることで日々の開発の生産性を上げる
 * 準標準パッケージで提供されるコードを利用することでコーディングの品質を上げる（車輪の再発明を避ける）
 * 今後リリースされるGoのバージョンに取り込まれるであろう、新しいGoの仕組みを先取りして学習する

まず、@<hd>{abst}では@<code>{golang.org/x}配下にあるパッケージの位置付けを簡単に述べます。
@<hd>{summary}では、@<code>{golang.org/x}配下にあるすべてのパッケージの簡単な紹介を述べます。
最後に@<hd>{detail}で実践的な実装を含んだパッケージの詳細とサンプルコードを記載します。

=={abst} golang.org/x配下にあるパッケージとは
@<code>{golang.org/x}という@<code>{import}パスから始まる準標準パッケージがあることをご存じでしょうか。
準標準パッケージは標準パッケージに比べて互換性（@<i>{compatibility requirements}）の基準が低いなどの制約はありますが、便利なパッケージばかりです。
また、Goに大きな新機能が追加される前の試験的な実装もこちらで公開されます。
例を挙げると、Go1.7で@<code>{context}パッケージが追加される前は@<code>{golang.org/x/net/context}パッケージの@<code>{context.Context}でコンテキスト情報を扱う実装が開始されていました。

#@# textlint-disable

最近ではGo Modules導入前の@<tt>{vgo}コマンドや新しいエラーハンドリングのアプローチとして公開された@<code>{golang.org/x/xerrors}パッケージなども@<code>{golang.org/x}の下で公開されている準標準パッケージです。

#@# textlint-enable

=={summary} golang.org/xで提供されているパッケージ一覧
それでは、さっそく@<code>{golang.org/x}配下から提供されているパッケージを見ていきましょう。
サブパッケージまでひとつひとつ紹介していくと途方のない数になってしまうため、ここでは@<tt>{GitHub}上のリポジトリ単位で紹介していきます。また以降の文中では@<code>{golang.org/x/package}パッケージは簡略化のため@<code>{x/package}パッケージと表記します。


=== @<code>{golang.org/x/arch}パッケージ
 * @<href>{https://godoc.org/golang.org/x/arch}
 * @<href>{https://github.com/golang/arch}

@<code>{x/arch}パッケージはマシンアーキテクチャ依存のコードが同梱されています。
アセンブラ別の実装が含まれており、Goのデバッガである@<tt>{Delve}（@<tt>{dlv}コマンド）@<fn>{delve}などで利用されています。
システムコールを扱いたい場合は参考になりそうです。

//footnote[delve][@<href>{https://github.com/go-delve/delve}]

=== @<code>{golang.org/x/crypto}パッケージ
 * @<href>{https://godoc.org/golang.org/x/crypto}
 * @<href>{https://github.com/golang/crypto}

@<code>{x/crypto}パッケージには標準パッケージに含まれていない暗号形式の実装が多数含まれています。
暗号化は自分で実装せず、提供されているライブラリを使いましょう。
@<code>{@kaneshin}さん執筆の@<chapref>{kaneshin}には@<code>{x/crypto/bcrypt}サブパッケージの利用例があります。
そちらもご参照ください。

=== @<code>{golang.org/x/image}パッケージ
 * @<href>{https://godoc.org/golang.org/x/image}
 * @<href>{https://github.com/golang/image}

@<code>{x/image}パッケージは標準パッケージの@<code>{image}パッケージと完全互換なパッケージです。
標準パッケージでサポートしていない@<tt>{TIFF}画像や@<tt>{BMP}画像に対する@<code>{encoder}/@<code>{decorder}を提供しています。
また、文字描画に関する@<code>{font}サブパッケージ、標準パッケージの@<code>{image/draw}パッケージよりリッチな機能を提供する @<fn>{draw}@<code>{draw}サブパッケージも提供しています。


//footnote[draw][@<href>{https://text.baldanders.info/golang/resize-image/}]

=== @<code>{golang.org/x/debug}パッケージ
 * @<href>{https://godoc.org/golang.org/x/debug}
 * @<href>{https://github.com/golang/debug}


デバッグツールである@<tt>{viewcore}コマンドの実装が入っています。
コアファイルの解析方法を知りたいときに実装が参考になるでしょう。
ただ、実際にデバッグ情報が知りたいなら、正直@<tt>{Delve}を利用したほうが効率的です。

==== @<tt>{viewcore}コマンドの利用方法
（この機会以外で利用しそうにないので、）@<tt>{viewcore}コマンドの挙動を確認します。
私の@<tt>{macOS Mojave}ではうまくコアダンプファイルを作れなかったので、もし手元で確認される方は@<tt>{Docker}で@<tt>{Linux}環境を用意することをオススメします。
今回は実行中のGoのプロセスにシグナルを送信し、無理やりコアダンプを取得しました。
事前に@<tt>{go言語のデバッガ（delveとgdb)とcore dump}@<fn>{qiita_cd}の記事を参考に@<list>{clash}のサンプルコードを用意しました。

//footnote[qiita_cd][@<href>{https://qiita.com/YasunoriGoto1/items/abd0d23262a72e2be9bf}]

#@# textlint-disable

//list[clash][無限ループで完了しないコード][go]{
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

#@# textlint-enable

サンプルコードを@<tt>{Docker}上で実行し、シグナルで終了させてコアダンプファイルを生成します。
コアダンプファイル内の@<tt>{goroutine}情報を@<tt>{viewcore}コマンドで確認してみたのが、次のコマンドラインの実行結果です。

#@# textlint-disable

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

#@# textlint-enable

コアファイルの取得方法は @<tt>{Debugging Go Code with GDB}@<fn>{debug_gdb}を参考にしました。

//footnote[debug_gdb][@<href>{https://golang.org/doc/gdb}]

=== @<code>{golang.org/x/exp}パッケージ
 * @<href>{https://godoc.org/golang.org/x/exp}
 * @<href>{https://github.com/golang/exp}

試験的に実装されたコードが含まれているのが@<code>{x/exp}パッケージです。
リポジトリをウォッチしていると、Goのエッジな開発情報が手に入るでしょう。
例を挙げると、@<code>{x/xerrors}パッケージも元はこのリポジトリで実装されており、@<code>{github.com/golang/xerrors}リポジトリのコミットログを確認すると、@<code>{x/exp}パッケージからコピーされてリポジトリへのコミットが始まったことがわかります。

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

=== @<code>{golang.org/x/lint}パッケージ
 * @<href>{https://godoc.org/golang.org/x/lint}
 * @<href>{https://github.com/golang/lint}

Goには静的解析ツールが豊富に存在しています。
デファクトスタンダードとなっている静的解析ツールのひとつである、@<tt>{golint}コマンドの実装が@<code>{x/lint}パッケージです。
実装には15以上の静的解析がメソッドとして含まれています。
自分で静的解析ツールを自作する前に、@<code>{x/lint}パッケージを一度読んでおくと何かヒントを得られるでしょう。

=== @<code>{golang.org/x/mobile}パッケージ
 * @<href>{https://godoc.org/golang.org/x/mobile}
 * @<href>{https://github.com/golang/mobile}


@<tt>{gomobile}と呼ばれる@<code>{x/mobile}パッケージはGoのコードからAndroidおよびiOS向けのアプリケーションもしくはライブラリを出力できます。
本リポジトリの詳細は@<code>{@hajimehoshi}さんが解説している@<chapref>{hajimehoshi}をご参照ください。



=== @<code>{golang.org/x/vgo}パッケージ
 * @<href>{https://godoc.org/golang.org/x/vgo}
 * @<href>{https://github.com/golang/vgo}
Go1.11から試験的に導入されているGoの新しい依存性管理の仕組みが@<i>{Go Modules}です@<fn>{modules_pro}。
そして、Go1.10のころにGo Modulesのプロトタイプ実装として公開されたのが@<code>{x/vgo}パッケージと@<tt>{vgo}コマンドです。
プロトタイプ実装のため、現在@<tt>{go}コマンドのサブコマンドになっている@<tt>{go mod}コマンドとは一部の挙動が異なります。現在使うことはないでしょうが、Russ Cox氏が当時公開した@<i>{Go & Versioning}の記事@<fn>{russ_vgo}を読むときに一緒に読むとGo Moudlesに至る過程を読み解く助けになるでしょう。

//footnote[modules_pro][@<href>{https://go.googlesource.com/proposal/+/master/design/24301-versioned-go.md}]
//footnote[russ_vgo][@<href>{https://research.swtch.com/vgo}]

=== @<code>{golang.org/x/mod}パッケージ
 * @<href>{https://godoc.org/golang.org/x/mod}
 * @<href>{https://github.com/golang/mod}

Go Modulesでは@<i>{セマンティックバージョニング}@<fn>{semver}や@<i>{暗号化ハッシュ}（@<tt>{go.sum}）といった技術を用いて依存性管理を実施しています。
これらのGo Modulesが利用している基礎技術の初期実装が@<code>{x/mode}パッケージに含まれています。
@<code>{mode}パッケージに含まれている実装の多くはすでにGo本体の@<code>{internal}パッケージに取り込まれています@<fn>{mod_ref}。
//footnote[semver][@<href>{https://semver.org/lang/ja/}]
//footnote[mod_ref][@<href>{https://github.com/golang/go/tree/go1.13/src/cmd/go/internal}]

=== @<code>{golang.org/x/net}パッケージ
 * @<href>{https://godoc.org/golang.org/x/net}
 * @<href>{https://github.com/golang/net}

@<code>{x/net}パッケージにはネットワーク関連の試験的な実装が含まれています。
@<tt>{WebSocket}プロトコル用の@<code>{websocket}サブパッケージもありますが、@<tt>{GoDoc}では@<code>{github.com/gorilla/websocket}@<fn>{gorilla}の利用を推奨しています@<fn>{godoc_ws}。
@<code>{http2}サブパッケージには@<tt>{HTTP/2}プロトコルに関わる実装が含まれています。
が、@<code>{http2}サブパッケージに含まれる実装は低レイヤの実装です。
通常の実装ユーザーがGoで@<tt>{HTTP/2}プロトコルを利用したい場合は、標準の@<code>{net/http}パッケージの利用で十分であると@<code>{http2}サブパッケージの@<tt>{GoDoc}に明記されています@<fn>{godoc_http2}。
また、@<code>{html}サブパッケージを用いれば@<tt>{HTML}を構造解析をして構文木を取得することもできます。
@<list>{exp_html_parse}@<fn>{play_findlink}は@<code>{html}サブパッケージの@<tt>{Examples}@<fn>{html_example}にあった構文木をパースするサンプルコードを少し改変したものです。

#@# textlint-disable

//list[exp_html_parse][HTML情報から構文木を取得する][go]{
package main

import (
  "fmt"
  "log"
  "strings"

  "golang.org/x/net/html"
)

const s = `<p>Links:</p>
<ul>
  <li><a href="foo">Foo</a>
  <li><a href="/bar/baz">BarBaz</a>
</ul>`

func findLink(n *html.Node) {
  if n.Type == html.ElementNode && n.Data == "a" {
    for _, a := range n.Attr {
      if a.Key == "href" {
        fmt.Println(a.Val)
        break
      }
    }
  }
  for c := n.FirstChild; c != nil; c = c.NextSibling {
    findLink(c)
  }
}

func main() {
  doc, err := html.Parse(strings.NewReader(s))
  if err != nil {
    log.Fatal(err)
  }
  findLink(doc)
}
//}

#@# textlint-enable

そのほか、@<code>{x/http}パッケージにはGo1.7以前に利用されていた@<code>{context}サブパッケージも含まれています。


//footnote[godoc_ws][@<href>{https://godoc.org/golang.org/x/net/websocket}]
//footnote[gorilla][@<href>{https://github.com/gorilla/websocket}]
//footnote[godoc_http2][@<href>{https://godoc.org/golang.org/x/net/http2}]
//footnote[html_example][@<href>{https://godoc.org/golang.org/x/net/html#example-Parse}]
//footnote[play_findlink][@<href>{https://play.golang.org/p/Rl0DwfDe9wM}]

=== @<code>{golang.org/x/oauth2}パッケージ
 * @<href>{https://godoc.org/golang.org/x/oauth2}
 * @<href>{https://github.com/golang/oauth2}

@<tt>{OAuth2.0}@<fn>{rfc6749}を利用した認可・認証処理をGoで実装しているのが@<code>{x/oauth2}パッケージです。
サブパッケージには@<tt>{Google}を始め、@<tt>{GitHub}や@<tt>{PayPal}などのメジャーな企業のエンドポイント情報などが定義されています。
単に@<tt>{OAuth2.0}のライブラリとしてだけではなく、@<code>{http.RoundTripper}インタフェースの実装の参考にもなります。

//footnote[rfc6749][@<href>{https://tools.ietf.org/html/rfc6749}]

=== @<code>{golang.org/x/perf}パッケージ
 * @<href>{https://godoc.org/golang.org/x/perf}
 * @<href>{https://github.com/golang/perf}

Goは標準パッケージでベンチマークを取得する仕組みが提供されているなど、パフォーマンスに対する取り組みも活発です。
@<code>{x/perf}パッケージにはパフォーマンスの計測結果を補完、解析するサーバシステムとパフォーマンスの測定結果をサーバに送信するクライアントの実装が含まれています。
@<code>{storage}サブパッケージや@<code>{analysis}サブパッケージなどの保存、分析用のコードも公開されています。
分析結果については@<tt>{Go Performance Dashboard}@<fn>{perf}で確認できます。

//footnote[perf][@<href>{https://perf.golang.org/}]

=== @<code>{golang.org/x/benchmarks}パッケージ
 * @<href>{https://godoc.org/golang.org/x/benchmarks}
 * @<href>{https://github.com/golang/benchmarks}

@<code>{x/benchmarks}パッケージはいくつかのベンチマークコマンドの実装が含まれているリポジトリです。
サブパッケージのコマンドはそれぞれがHTTPリクエスト、ガベージコレクタ、JSON Unmarshal/Marshalなどのベンチマークを実行します。
正しく計測するにはどのようにベンチマークテストを実装すべきか、という知見が詰まっています。
たとえば、@<list>{bench_gc}に引用したガベージコレクタのベンチマークコードでは、@<code>{runtime.GC}メソッドを2回実行しています。
2回目の@<code>{GC}を実行することで、1回目の@<code>{GC}をバックグラウンド処理も含めて完全に完了させるためです@<fn>{gc_sweep}。

//footnote[gc_sweep][@<href>{https://github.com/golang/go/blob/go1.13//src/runtime/mgc.go#L110}]

#@# textlint-disable
//list[bench_gc][GCを完了するための実装][go]{
// packageMemConsumption returns memory consumption of a single parsed package.
func packageMemConsumption() int {
  // One GC does not give precise results,
  // because concurrent sweep may be still in progress.
  runtime.GC()
  runtime.GC()
  ms0 := new(runtime.MemStats)
//}
#@# textlint-enable

また、ベンチマークコマンドのメインロジックは@<code>{driver}パッケージで実装@<fn>{bench_driver}されており、ツールを作る際の共通化の参考にもなりそうです。

//footnote[bench_driver][@<href>{https://github.com/golang/benchmarks/blob/master/driver/driver.go}]

=== @<code>{golang.org/x/sync}パッケージ
 * @<href>{https://godoc.org/golang.org/x/sync}
 * @<href>{https://github.com/golang/sync}

標準パッケージの@<code>{sync}パッケージの補完的なパッケージです。
@<code>{x/sync/errgroup}サブパッケージ@<fn>{errgroup}の@<code>{errgroup.Group}構造体は
戻り値にエラーがないか判断する標準パッケージの@<code>{sync.WaitGroup}構造体@<fn>{sync_waitgroup}のような機能です。
@<code>{Examples}@<fn>{justerrors}に記載されている@<list>{errgroup}のようにエラーハンドリングをしながら複数の@<code>{goroutine}の終了を待機できます。

//footnote[errgroup][@<href>{https://godoc.org/golang.org/x/sync/errgroup}]
//footnote[justerrors][@<href>{https://godoc.org/golang.org/x/sync/errgroup#example-Group--JustErrors}]
//footnote[sync_waitgroup][@<href>{https://golang.org/pkg/sync/#WaitGroup}]

#@# textlint-disable
//list[errgroup][GCを完了するための実装][go]{
var g errgroup.Group
var urls = []string{
    "http://www.golang.org/",
    "http://www.google.com/",
    "http://www.somestupidname.com/",
}
for _, url := range urls {
    // Launch a goroutine to fetch the URL.
    url := url // https://golang.org/doc/faq#closures_and_goroutines
    g.Go(func() error {
        // Fetch the URL.
        resp, err := http.Get(url)
        if err == nil {
            resp.Body.Close()
        }
        return err
    })
}
// Wait for all HTTP fetches to complete.
if err := g.Wait(); err == nil {
    fmt.Println("Successfully fetched all URLs.")
}
//}
#@# textlint-enable


@<code>{x/sync/syncmap}パッケージに定義されている@<code>{syncmap.Map}はGo1.9から@<code>{sync.Map}@<fn>{sync_map}として標準パッケージに取り込まれています。

//footnote[sync_map][@<href>{https://golang.org/pkg/sync/#Map}]

=== @<code>{golang.org/x/text}パッケージ
 * @<href>{https://godoc.org/golang.org/x/text}
 * @<href>{https://github.com/golang/text}

@<code>{x/text}パッケージは@<i>{Internationalization}（@<tt>{i18n}）や@<i>{Localization}（@<tt>{i10n}）に関するパッケージが含まれているパッケージです。
エンコーディングを変換できる@<code>{encoding}サブパッケージ群や、数値を通貨を変換する@<code>{currency}サブパッケージなどの国際化・ローカライズのためのパッケージがそろっています。
エンコーディングの変換や国際化方法は次の記事を参考にしてください。

 * @<tt>{Go 言語の文字エンコーディング変換}@<fn>{spiegel_enc} by @<tt>{@spiegel}さん@<fn>{spiegel}
 * @<tt>{golang.org/x/text/messageでI18N}@<fn>{ymotongpoo_i18n} by @<tt>{@ymotongpoo}さん@<fn>{ymotongpoo}
 * @<tt>{golang.org/x/text/transformパッケージを使う #golang}@<fn>{mercari_trans} by @<tt>{@tenntenn}さん@<fn>{tenntenn}

//footnote[tenntenn][@<href>{https://twitter.com/tenntenn}]
//footnote[mercari_trans][@<href>{https://tech.mercari.com/entry/2017/12/05/154907}]
//footnote[ymotongpoo_i18n][@<href>{https://ymotongpoo.hatenablog.com/entry/2018/12/25/163455}]
//footnote[ymotongpoo][@<href>{https://twitter.com/ymotongpoo}]
//footnote[spiegel_enc][@<href>{https://text.baldanders.info/golang/transform-character-encoding/}]
//footnote[spiegel][@<href>{https://twitter.com/spiegel_2007}]

=== @<code>{golang.org/x/time}パッケージ
 * @<href>{https://godoc.org/golang.org/x/time}
 * @<href>{https://github.com/golang/time}

@<code>{x/time}パッケージは標準パッケージの@<code>{time}パッケージの補足的なパッケージです。
現在提供されているのは@<code>{rate}サブパッケージのみで、レートリミットを行うための@<code>{rate.Limitter}構造体が含まれています。
@<code>{rate.Limitter}構造体は排他制御の実装はもちろんのこと、複数の処理に重みを付けて制限することもでき、@<tt>{Go言語による並行処理}@<fn>{cigo}の@<tt>{5.5 流量制限}の章でも利用されています。

//footnote[cigo][@<href>{https://www.oreilly.co.jp/books/9784873118468/}]

=== @<code>{golang.org/x/tools}パッケージ
 * @<href>{https://godoc.org/golang.org/x/tools}
 * @<href>{https://github.com/golang/tools}

@<code>{x/tools}パッケージには便利な20以上のコマンドラインツールや、静的解析で利用するパッケージなどが含まれています。
@<code>{cmd}サブパッケージの下に入っているコマンドラインツールは、@<code>{import}文の整形を行う@<tt>{goimports}、@<tt>{LSP}（@<i>{Language  Server Protocol}）のGoクライアントである@<tt>{gopls}、リファクタリングツールである@<tt>{gorename}など開発に必須なものばかりです。
また、@<code>{go}サブパッケージ配下には@<code>{AST}（@<i>{Abstract Syntax Tree}）の解析を行う @<code>{x/tools/go/ast}などの静的解析で利用するパッケージが多数含まれています。
得に@<code>{go/analysis}サブパッケージはGoの静的解析をモージュル構造に再定義したパッケージで、@<code>{x/tools/go/analysis/passes}にはモジュール化された@<tt>{go vet}コマンドの実装が置かれています。
静的解析やASTについては、次の電子書籍や記事にわかりやすくまとまっています。

 * @<tt>{逆引きGoによる静的解析入門}@<fn>{booth_kmt}@<fn>{booth_ten} by @<tt>{@tenntenn}さん@<fn>{tenntenn}、@<tt>{@knsh14}さん@<fn>{knsh14}
 * @<tt>{Goにおける静的解析のモジュール化について}@<fn>{mercari_sa} by @<tt>{@tenntenn}さん
 * @<tt>{GoのためのGo}@<fn>{motemen_go} by @<tt>{@motemen}@<fn>{motemen}さん

//footnote[mercari_sa][@<href>{https://tech.mercari.com/entry/2018/12/16/150000}]
//footnote[knsh14][@<href>{https://twitter.com/knsh14}]
//footnote[motemen][@<href>{https://twitter.com/motemen}]
//footnote[booth_kmt][@<href>{https://booth.pm/ja/items/1319336}]
//footnote[booth_ten][@<href>{https://booth.pm/ja/items/1319394}]
//footnote[motemen_go][@<href>{https://motemen.github.io/go-for-go-book/}]

=== @<code>{golang.org/x/blog}パッケージ
 * @<href>{https://godoc.org/golang.org/x/blog}
 * @<href>{https://github.com/golang/blog}

Goの仕様やツールの利用方法について知りたいと思ったとき、私たちはよく公式ブログである@<tt>{The Go Blog}@<fn>{goblog}をよく参照します。
@<code>{x/blog}パッケージには@<tt>{The Go Blog}の記事がテキスト（@<code>{article}ファイル）として管理されています。
正直@<tt>{The Go Blog}は検索性があまり高くありません。
ブログ内容を全文検索したいとき、このリポジトリを利用すれば手元で@<tt>{grep}したり、@<tt>{GitHub}の検索機能を利用して記事を探すことができます。

//footnote[goblog][@<href>{https://blog.golang.org}]

=== @<code>{golang.org/x/build}パッケージ
 * @<href>{https://godoc.org/golang.org/x/build}
 * @<href>{https://github.com/golang/build}

Goの継続的デプロイや各リリースに関するエコシステムに関連するパッケージが同梱されているのが@<code>{x/build}パッケージです。
リリースマイルストーンに関わる@<tt>{issue}一覧や@<tt>{Open}状態の@<tt>{CL}（@<tt>{Change List}）一覧を確認できる@<tt>{developer dashboard}@<fn>{devgo}、
@<tt>{Gerrit}と@<tt>{GitHub}をミラーリングする@<tt>{bot}、
各@<tt>{review}に対して行われている継続的ビルドの結果を一覧できる@<tt>{build dashboard}@<fn>{buildgo}などに関するパッケージが同梱されています。
@<code>{cmd}パッケージ配下には多くのコマンドラインツールがあるので、コマンドラインツールを作成する際にも参考になるでしょう。

//footnote[devgo][@<href>{https://dev.golang.org/}]
//footnote[buildgo][@<href>{https://build.golang.org/}]


=== @<code>{golang.org/x/tour}パッケージ
 * @<href>{https://godoc.org/golang.org/x/tour}
 * @<href>{https://github.com/golang/tour}

Go未経験者がGoを始めるとき、大半は@<tt>{A Tour of Go}@<fn>{tog}というチュートリアルサイトで勉強するでしょう。
@<code>{x/tour}パッケージにはA Tour of GoのWebサイトの実装が含まれています。
同サイトの誤字や内容の修正が必要な場合はこのリポジトリから行うことができます。

//footnote[tog][@<href>{https://tour.golang.org}]

=== @<code>{golang.org/x/website}パッケージ
 * @<href>{https://godoc.org/golang.org/x/website}
 * @<href>{https://github.com/golang/website}

Goの公式サイトはご存じのとおり、@<tt>{golang.org}@<fn>{golangorg}にあります。
公式サイトのHOME画面を管理しているのが@<code>{x/website}パッケージです。
リリースノートなどのページはGo本体のリポジトリなどに含まれていたりします@<fn>{go_doc}。
Google Cloudにある@<i>{google.golang.org パッケージの一覧}@<fn>{googlegolangorg}から、各GoDocへリダイレクトする簡易Webサーバなどの実装@<fn>{google_pkg_list}も同梱されています。


//footnote[go_doc][@<href>{https://github.com/golang/go/tree/go1.13/doc}]
//footnote[golangorg][@<href>{https://golang.org}]
//footnote[google_pkg_list][@<href>{https://cloud.google.com/go/google.golang.org}]
//footnote[googlegolangorg][@<href>{https://github.com/golang/website/tree/master/cmd/googlegolangorg}]


==={errors_summary} @<code>{golang.org/x/xerrors}パッケージ
 * @<href>{https://godoc.org/golang.org/x/xerrors}
 * @<href>{https://github.com/golang/xerrors}

2018年まではGoでスタックトレースを含んだエラー処理を行うなら、@<code>{github.com/pkg/errors}パッケージを使うのがデファクトスタンダードでした。
@<code>{x/xerrors}パッケージはGo2に向けて提案された@<i>{Proposal: Go 2 Error Inspection}@<fn>{go2_error}をGo1向けに実装したライブラリです。
@<code>{x/xerrors}パッケージの@<code>{Newf}（あるいは@<code>{Errof}）関数から生成されたエラーは内部にスタックトレースを持ちます。このスタックトレースはプリントフォーマットで@<code>{%+v} verbeを使って出力できます。
また、@<code>{x/xerrors}パッケージはスタックトレース以外にもエラーの同値性を検証する@<code>{Is}関数、取得したエラーから具体的な型のオブジェクトを抽出できる@<code>{As}関数が提供されています。
2019年9月3日に公開されたGo1.13で@<code>{x/xerrors}パッケージに含まれる大半の関数が標準パッケージの@<code>{errors}パッケージへ正式に導入されました@<fn>{go113_err}。
しかし、@<code>{%+w}や@<code>{%+v}によるスタックトレースの表示の採用は見送られています@<fn>{xerr_frame}。

//footnote[go2_error][@<href>{https://go.googlesource.com/proposal/+/master/design/29934-error-values.md}]
//footnote[xerr_frame][Go1.13のエラーオブジェクトは内部にスタックトレース（@<code>{Frame}）情報を持っていない]
//footnote[go113_err][@<href>{https://golang.org/doc/go1.13#error_wrapping}]

=={detail} golang.org/xで提供されているパッケージを使ったコーディング
前節で2019年9月現在存在する@<code>{golang.org/x}配下のパッケージを俯瞰的に確認しました。
本節では、実際に業務やOSSでも利用できる実用的なパッケージの利用方法を紹介します。

=== @<code>{golang.org/x/oauth2}パッケージの利用例

@<code>{x/oauth2}パッケージはGoでOAuth2.0形式の認証認可を扱うためのパッケージです。
@<code>{GitHub}や@<code>{Google}、@<code>{PayPal}など各社の認可エンドポイントのURLが定義された構造体オブジェクトも含まれています。
@<code>{github_endpoint}は@<code>{x/oauth2/github}サブパッケージ内に定義された@<tt>{GitHub}のエンドポイント情報です。

#@# textlint-disable

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

#@# textlint-enable

ここでは、一番ポピュラーである認可コード@<i>{Authorization Code}）フローでの使い方を見てみましょう。
@<code>{x/oauth2}パッケージでは、@<code>{Config}構造体を中心に認可・認証を行います。
認可元に登録した（あるいは発行された）情報から認可エンドポイントへのアクセスするためのURLを作成する手順が@<list>{oauth2_example1}です。

#@# textlint-disable
//list[oauth2_example1][認可エンドポイントへのリクエスト]{
ctx := context.Background()
conf := &oauth2.Config{
    ClientID:     "YOUR_CLIENT_ID",
    ClientSecret: "YOUR_CLIENT_SECRET",
    Scopes:       []string{"SCOPE1", "SCOPE2"},
    RedirectURL: "https://clienturl.com/callback"
    Endpoint: oauth2.Endpoint{
        AuthURL:  "https://provider.com/o/oauth2/auth",
        TokenURL: "https://provider.com/o/oauth2/token",
    },
}

url := conf.AuthCodeURL("dummy-state", oauth2.AccessTypeOffline)
// 生成されたurlを使って、認可画面にアクセスできる。
//}
#@# textlint-enable

認可画面で認可に成功すると、認可元は@<code>{RedirectURL}に認可コードを返してきます。
クライアント側は@<list>{oauth2_example2}のようにその認可コードを使ってトークンエンドポイントからトークンを発行するハンドラを実装することになります。

#@# textlint-disable
//list[oauth2_example2][コールバックURLに設定するハンドラ]{
func CallbackHandler(rw http.ResponseWriter, req *http.Request) {
    // 厳密にはクエリ内のstateも確認する必要がある。
    code := req.URL.Query().Get("code")

    tok, err := conf.Exchange(ctx, code)
    if err != nil {
      log.Fatal(err)
    }

    // tok（*oauth2.Token）オブジェクトを保存する
    // 処理の成否をhttp.ResponseWriterに書き込む
//}
#@# textlint-enable

@<code>{*oauth2.Token}オブジェクトが取得できれば、@<code>{*oauth2.Config}オブジェクトと組み合わせて@<code>{*http.Client}オブジェクトが取得できます。
この@<code>{*http.Client}はリクエストヘッダにトークンを自動で埋め込んでくれます。
また、アクセストークンがリクエスト送信前に@<tt>{Expire}していても、自動トークンリフレッシュをしてくれる優れものです。
何らかの理由で独自定義した@<code>{*http.Client}オブジェクトを利用したい場合もあります。
その場合は@<code>{http.RoundTripper}インタフェースを満たした@<code>{*oauth.Transport}オブジェクトも生成できます。
この@<code>{*oauth.Transport}オブジェクトも自動でトークンリフレッシュをしてくれます。

#@# textlint-disable
//list[oauth2_example3][取得したトークンを使ってHTTPクライントを生成する]{
  ctx := context.Background()
  // confはこれまで同様*oauth2.Configオブジェクト
  client := conf.Client(ctx, tok)

  // 通常のhttp.Clientと同様に利用できる。
  client.Get("...")

  // 
  myclient := &http.Client{
    Transport: &oauth2.Transport{
      Source: conf.TokenSource(ctx),
    },
  }
  myclient.Get("...")
//}
#@# textlint-enable

=== @<code>{golang.org/x/time/rate}サブパッケージの利用例
何かサービスを作っていると、あるリソースへのアクセスやリクエストの数を制限したいことがあります。
そんなときに利用できるのがレートリミットと呼ばれる流量制限の仕組みです。
@<code>{x/time/rate}サブパッケージはトークンバケット（@<i>{Token Bucket}@<fn>{limilis}）方式のレートリミットを実装したパッケージです。

//footnote[tb][@<href>{https://en.wikipedia.org/wiki/Token_bucket}]
//footnote[limilis][単純な帯域制限を行う実装として@<code>{x/net/netutil}パッケージに@<code>{LmitListenr}構造体もあります]

トークンバケットアルゴリズムでは、一定間隔で増え続けるトークン数と最大トークン数が決まっている@<tt>{バケット}が存在します。
あるアクションが行われるには、バケットにアクションごとに指定される数のトークンが必要になります。アクション実行時にトークンは消費されます。
そのためバケットにあるトークン数よりもアクションに必要なトークン数が多い場合はトークンの回復を待つ必要があります。
このようなアルゴリズムを@<code>{x/time/rate}サブパッケージを通してどのように利用するのか確認してみます。

@<list>{rate1}は@<code>{rate.Limiter}構造体の初期化方法です。
@<code>{x/time/rate}サブパッケージはこの@<code>{rate.Limiter}構造体を通して利用します。
変数@<code>{r}と@<code>{b}に記載されたコメントのとおり、トークンの発行間隔とバケットが保持できる最大トークン数で初期化します。

#@# textlint-disable
//list[rate1][@<code>{*rate.Limitter}オブジェクトの取得方法][go]{
r = rate.Limit(2) // 1秒間に増加するトークン数
b = 5             // 最大トークン数
l := rate.NewLimiter(r, b)
//}
#@# textlint-enable

@<list>{rate2}は@<code>{*rate.Limitter}オブジェクトがもつレートリミット制御用の主要メソッドです@<fn>{godoc_rate}。
@<code>{rate.Limiter}構造体はこの3種類のメソッドを使ってレートリミットを実現します。

//footnote[godoc_rate][@<href>{https://godoc.org/golang.org/x/time/rate}]

#@# textlint-disable
//list[rate2][レートリミットを実現するためのメソッド一覧][go]{
func (lim *Limiter) Allow() bool
func (lim *Limiter) Wait(ctx context.Context) (err error)
func (lim *Limiter) Reserve() *Reservation

// Reserveメソッドが返す*Reservationオブジェクトで利用できるメソッド
func (r *Reservation) Cancel()
func (r *Reservation) Delay() time.Duration
//}
#@# textlint-enable

以降のサンプルコードでは@<list>{rate3}のような@<code>{*rate.Limiter}オブジェクトのラッパー構造体と、動作確認用の@<code>{main}関数を利用します。
実行条件として今回は最大トークン数が5個のバケットに毎秒2個のアクセストークンが追加されるような条件で設定します。
ラッパーに@<list>{rate2}で確認した各メソッドを利用したメソッドを実装し、アクションを10回発生させ、挙動の違いを確認しましょう。


#@# textlint-disable
//list[rate3][@<code>{*rate.Limiter}オブジェクトのラッパーと動作確認用の実装]{
// LimitConn is Limiter wrapper.
type LimitConn struct {
  lim *rate.Limiter
}

func main() {
  log.SetFlags(log.Ltime)

  n = 10            // 発生するアクション数
  r = rate.Limit(2) // 1秒間に増加するトークン数
  m = 5             // 最大トークン数

  lc := &LimitConn{
    lim: rate.NewLimiter(r, m),
  }
  var wg sync.WaitGroup

  for i := 0; i < n; i++ {
    wg.Add(1)
    go func(i int) {
      defer wg.Done()
      // 後述するレートリミットを実装したLimitConnのメソッドをここで実行する
    }(i)
  }

  wg.Wait()
}
//}
#@# textlint-enable

@<list>{rate4}は@<code>{Allow}メソッドを使ったレートリミットの実装です。
レートリミットで制限される状況で、対象のアクションを破棄あるいはスキップしてよいなら@<code>{Allow}メソッドを使います。

#@# textlint-disable
//list[rate4][@<code>{Allow}メソッドを利用したレートリミットの実装]{
func (lc *LimitConn) allow(i int) {
  if !lc.lim.Allow() {
    log.Printf("allow: cancel %d\n", i)
    return
  }
  log.Printf("allow: done %d!\n", i)
}

// main関数内ではこのように利用される
  for i := 0; i < n; i++ {
    wg.Add(1)
    go func(i int) {
      defer wg.Done()
      lc.wait(i)
    }(i)
  }
//}
#@# textlint-enable

@<code>{Allow}メソッドを使って前述の@<code>{main}関数を実行した結果が次の出力です@<fn>{play_allow}。
@<code>{*rate.Limiter}オブジェクトのバケットは初期化時に最大トークン数が5個の状態になっています。
そのため、5個のアクションは実行されますが、残りの5個はトークンを取得できずそのままキャンセルされています。

//footnote[play_allow][@<href>{https://play.golang.org/p/KuNawacdEFe}]

#@# textlint-disable
//cmd{
07:56:10 allow: done 9!
07:56:10 allow: done 5!
07:56:10 allow: done 2!
07:56:10 allow: done 0!
07:56:10 allow: done 1!
07:56:10 allow: cancel 3
07:56:10 allow: cancel 4
07:56:10 allow: cancel 7
07:56:10 allow: cancel 8
07:56:10 allow: cancel 6
//}
#@# textlint-enable

@<list>{rate5}は@<code>{Wait}メソッドを使ったレートリミットの実装です。
@<code>{Wait}メソッドを使った場合は必要なトークンが回復するまでアクションの実行を待機できます。

#@# textlint-disable
//list[rate5][@<code>{Wait}メソッドを利用したレートリミットの実装]{
func (lc *LimitConn) wait(i int) {
	if err := lc.lim.Wait(context.Background()); err != nil {
		log.Printf("wait: %v\n", err)
		return
	}
	log.Printf("wait: done %d!\n", i)
}
//}
#@# textlint-enable

@<code>{Wait}メソッドを使って前述の@<code>{main}関数を実行した結果が次の出力です@<fn>{play_wait}。
最初の5個のアクションは即座に実行されます。
残りの5個のアクションは毎秒2個ずつ実行されています。
1秒間に増加するトークン数を2個@<code>{r = rate.Limit(2)}と設定しているためです。

//footnote[play_wait][@<href>{https://play.golang.org/p/lBtSotFqAZF}]

#@# textlint-disable
//cmd{
07:56:07 wait: done 2!
07:56:07 wait: done 9!
07:56:07 wait: done 3!
07:56:07 wait: done 4!
07:56:07 wait: done 5!
07:56:08 wait: done 6!
07:56:08 wait: done 0!
07:56:09 wait: done 7!
07:56:09 wait: done 8!
07:56:10 wait: done 1!
//}
#@# textlint-enable

@<list>{rate6}は@<code>{Reserve}メソッドを使ったレートリミットの実装です。
@<code>{Reserve}メソッドを使った場合は戻り値で@<code>{*rate.Reservation}オブジェクトが取得できます。
同オブジェクトの@<code>{Delay}メソッドを使えば@<code>{Wait}メソッドのときのようにアクションを待機できます。
また、@<code>{Delay}メソッドでアクションをキャンセルすることもできます。
@<list>{rate6}では動作確認のため、与えられた整数が偶数のときは@<code>{Cancel}メソッドでキャンセルするようにしています。

#@# textlint-disable
//list[rate6][@<code>{Reserve}メソッドを利用したレートリミットの実装]{
func (lc *LimitConn) reserve(i int) {
  r := lc.lim.Reserve()
  if i%2 == 0 {
    log.Printf("reserve: cancel %d!\n", i)
    r.Cancel()
    return
  }
  if !r.OK() {
    log.Printf("reserve: not ok %d!\n", i)
    return
  }
  time.Sleep(r.Delay())
  log.Printf("reserve: done %d!\n", i)
}
//}
#@# textlint-enable

@<code>{Reserve}メソッドを使って前述の@<code>{main}関数を実行した結果が次の出力です@<fn>{play_reserve}。
@<code>{Cancel}メソッドが実行されるとトークンが回復するので、バケットの最大トークン数の@<code>{5}以上のアクションが同時に実行されています。
その後は@<code>{Wait}メソッド同様時間をずらしながらアクションが実行されています。

//footnote[play_reserve][@<href>{https://play.golang.org/p/qR9S9mUgHVk}]

#@# textlint-disable
//cmd{
15:52:04 reserve: cancel 2!
15:52:04 reserve: done 9!
15:52:04 reserve: cancel 0!
15:52:04 reserve: done 5!
15:52:04 reserve: cancel 4!
15:52:04 reserve: cancel 6!
15:52:04 reserve: cancel 8!
15:52:04 reserve: done 7!
15:52:05 reserve: done 1!
15:52:05 reserve: done 3!
//}
#@# textlint-enable

以上が@<code>{x/time/rate}サブパッケージを使った流量制限の例です。
今回は紹介しませんでしたが、@<code>{*rate.Limiter}オブジェクトには@<code>{AllowN}、 @<code>{ReserveN}、@<code>{WaitN}メソッドが存在します。
これらのメソッドでは引数に任意の要求トークン数@<code>{n}を指定できます。
これを利用してアクション別に必要トークン数をの指定を変化させることで、イベント別に重み付けしながら流量制限をすることも可能です。

=== @<code>{golang.org/x/xerrors}パッケージの利用例

@<code>{x/xerrors}パッケージを使ったエラー表現をみていきます。
@<code>{x/xerrors}パッケージの@<code>{Newf}（あるいは@<code>{Errof}）関数から生成されたエラーは内部にスタックトレースを持ちます。
このスタックトレースはプリントフォーマットで@<code>{%+v} verbeを使って出力できます。
また、@<code>{x/xerrors}パッケージはスタックトレース以外にもエラーの同値性を検証する@<code>{Is}関数、取得したエラーから具体的な型のオブジェクトを抽出できる@<code>{As}関数が提供されています。

#@# textlint-disable

//list[xerr_sample][xerrorsパッケージを使ったエラーハンドリング@<fn>{play_xerr}][go]{
package main

import (
  "fmt"

  "golang.org/x/xerrors"
)

type MyError struct {
  body string
}

func (_ *MyError) Error() string {
  return "original error"
}

// 通常のエラーには存在しないメソッド
func (me *MyError) Body() string {
  return me.body
}

func main() {
  base := &MyError{"original body"}
  err := xerrors.Errorf("wraped: %w", base)
  fmt.Printf("%+v\n", err)
  // errの中にMyError型が含まれていればtrue
  fmt.Println("xerrors.Is", xerrors.Is(err, base))

  var me *MyError
  // errの中からMyError型を抽出できたらmeに代入する
  if xerrors.As(err, &me) {
    fmt.Printf("body: %q\n", me.Body())

  }
}
//}

#@# textlint-enable

@<list>{xerr_sample}を実行した結果は次のとおりです。スタックトレースからファイル名と行数が取得できています。
@<code>{Is}関数では、ネストしたエラーの型もチェックできます。
また、@<code>{As}関数によってエラーの中に含まれている独自型の情報も取得できました。

//cmd{
wraped:
    main.main
        /tmp/sandbox096453308/prog.go:24
  - original error
xerrors.Is true
body: "original body"
//}

//footnote[play_xerr][@<href>{https://play.golang.org/p/9Vq2jTUiL5b}]

前述したとおり、Go1.13の標準パッケージの@<code>{errors}パッケージでは依然としてスタックトレースを取得できません。
スタックトレースの表示が必要な場合は@<code>{x/xerrors}パッケージを利用して、不要な場合には標準ライブラリの@<code>{errors}パッケージを利用してください。
そな太さん @<fn>{sonatard}がまとめている@<tt>{Qiita}の@<tt>{xerrors - 関連情報}@<fn>{sonatard}の記事を参考にするとよいでしょう。

//footnote[pkgerrors][@<href>{https://github.com/pkg/errors}]
//footnote[sonatard][@<href>{https://twitter.com/sonatard}]
//footnote[qiita_xerrors][@<href>{https://qiita.com/sonatard/items/802db82e7275f17fe702}]

#@# textlint-disable

== おわりに

#@# textlint-enable

本章では@<code>{golag.org/x}配下にある準標準パッケージを紹介しました。
準標準パッケージは別のパッケージで@<tt>{import}して使うものばかりではなく、Goのために提供されているWebサイトやエコシステムのためのパッケージも多数存在しました。
私は「便利な準標準パッケージをこの機会にみつけるぞ」という気持ちで今回の執筆を開始しました。
結果として、コード以外でもこの原稿を書くまで知らなかった@<tt>{Go Development Dashboard}などの存在も知り、学びを得ることができました。
#@# textlint-disable
今回サンプルコードを紹介できなかった準標準パッケージも試してみてブログ@<fn>{myblog}で紹介できたらと思います。
#@# textlint-enable
みなさんも気になった準標準パッケージがあればぜひリポジトリやGoDocを読んでみてください。

//footnote[myblog][@<href>{https://budougumi0617.github.io/}]
