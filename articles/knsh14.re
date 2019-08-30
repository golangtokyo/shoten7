= 新しい GUI ライブラリ Gio で遊んでみよう

2019年7月に開催されたGopherCon 2019@<fn>{knsh14_gophercon_link}で新しいGoのGUIライブラリが発表されました。
これまでGoの苦手な分野としてGUIを持ったアプリケーションが開発しづらいという点が挙げられていましたが、これを解決する可能性を秘めたライブラリです。


//footnote[knsh14_gophercon_link][https://www.gophercon.com/]

== Gio の概要
Gio@<fn>{knsh14_gioui_link}はGo製のGUIライブラリです。
作者のElias NaurさんはGo Mobileを開発されていた方です。
その経験を踏まえつつ、今回さらに強力なライブラリを設計しました。

Gioはまだv1.0.0がリリースされておらず、今でも大きな変更があります。
2019年9月1日執筆時点でのバージョンを正式なものとして扱います。
//footnote[knsh14_gioui_link][https://gioui.org]

=== 特徴
大きな特徴として私達が書くアプリケーションのコードをすべてGoで書ける点にあります。
さらに、Goそのものの特徴であるクロスプラットフォームにも対応しています。
対応しているプラットフォームはiOS、AndroidはもちろんtvOSやWebGLなどにも出力できます。

=== インストール方法
インストール方法は他のGoのライブラリと変わらない方法でインストールできます。
ライブラリの取得方法を@<list>{knsh14_gioui_install}に示します。

//list[knsh14_gioui_install][Gio UIのインストール方法][shell]{
$ go get -u gioui.org/...
//}

開発中は@<code>{go run main.go}のようにそのまま実行できます。
実際にアプリケーションをビルドしてバイナリにする場合は@<list>{knsh14_gioui_builds}に上げたGioのコマンドを実行する必要があります。

//list[knsh14_gioui_builds][Gioアプリケーションのビルド方法][shell]{
// iOS
$ go run gioui.org/cmd/gio -target ios -appid <bundle-id> PATH/TO/APP

// Android
$ go run gioui.org/cmd/gio -target android PATH/TO/APP

// WebGL
$ go run gioui.org/cmd/gio -target js PATH/TO/APP
$ go get github.com/shurcooL/goexec
$ goexec 'http.ListenAndServe(":8080", http.FileServer(http.Dir("APP")))'
//}

ビルドするためのコマンドが@<code>{gio}ではなく、@<code>{go run gioui.org/cmd/gio}なのはgioという別のコマンドがmacOSではデフォルトで入っているからです。
iOS向けやAndroid向けのビルドコマンドで生成されたバイナリは各ツールでさらに端末にインストールできます。

== Gio の設計

== Gio で遊んで見る
=== 真っ白なWindowを出力する
まずはGioのプログラムを動かしてみましょう。

//list[knsh14_gioui_sample_blank_window][真っ白なWindow][go]{
package main

import (
  "gioui.org/ui/app"
)

func main() {
  go func() {
    w := app.NewWindow()
    for _ = range w.Events() {
    }
  }()
  app.Main()
}
//}

@<code>{main}関数で@<code>{gioui.org/ui/app}の@<code>{Main}関数を呼び出します。
この関数は必ず呼ぶ必要があります。@<code>{gioui.org/ui/app.Main}関数はすべてのウィンドウが閉じられるまで@<code>{main}関数をブロックします。
ウィンドウに画像や文字を出したり、キーの入力を受けたりする処理は別の@<code>{goroutine}で実行する必要があります。

=== Hello world
GopherConで例として利用されたHello worldのコード@<fn>{knsh14_gioui_sample_hello_world_link}を実行します。

//list[knsh14_gioui_sample_hello_world][GioにおけるHello world][go]{
package main

import (
  "gioui.org/ui"
  "gioui.org/ui/app"
  "gioui.org/ui/layout"
  "gioui.org/ui/measure"
  "gioui.org/ui/text"

  "golang.org/x/image/font/gofont/goregular"
  "golang.org/x/image/font/sfnt"
)

// START OMIT
func main() {
  go func() {
    w := app.NewWindow()
    regular, _ := sfnt.Parse(goregular.TTF)
    var cfg ui.Config
    var faces measure.Faces
    ops := new(ui.Ops)
    for e := range w.Events() {
      if e, ok := e.(app.UpdateEvent); ok {
        cfg = &e.Config
        cs := layout.RigidConstraints(e.Size)
        ops.Reset()
        faces.Reset(cfg)

        lbl := text.Label{Face: faces.For(regular, ui.Sp(72)), Text: "Hello, World!"} // HLdraw
        lbl.Layout(ops, cs)                                                           // HLdraw

        w.Update(ops)
      }
    } // HLeventloop
  }()
  app.Main()
}

// END OMIT
//}

このコードを実行すると@<img>{knsh14_gioui_hello_world}のような実行結果が得られます。
//image[knsh14_gioui_hello_world][GioにおけるHello world]{
//}

ウィンドウを出す方法は真っ白なウィンドウを出す方法と変わりません。
このサンプルでは文字を出す方法を新しく紹介します。
23行目のfor文でウィンドウオブジェクトからイベントを取り出します。
イベントには「画面を更新した」、「何らかの入力を受けた」などがあります。
このサンプルでは画面を更新する際のイベント@<code>{gioui.org/ui/app.UpdateEvent}@<fn>{knsh14_gioui_app_updateevent_doc_link}の場合に文字を出す処理を行います。

文字を出力するためには@<code>{gioui.org/ui/text.Label}を利用します。
@<code>{Label}には2つのフィールドがあります。
1つ目はもちろん出力するための@<code>{Text}フィールドです。
2つ目は文字のフォント、大きさを決める@<code>{Face}フィールドです。
これら2つを指定して文字を表示するための準備をします。

文字を実際に出すためには@<code>{Layout}メソッドを呼び出す必要があります。
このメソッドに渡すために、@<code>{gioui.org/ui.Ops}と@<code>{gioui.org/ui/layout.Constraints}を取得する必要があります。
@<code>{gioui.org/ui.Ops}はループの外側で定義し、再利用することができます。
@<code>{gioui.org/ui/layout.Constraints}はイベントを処理する毎に取得する必要があります。
なぜなら画面サイズが変わったりした場合に再度計算する必要があるからです。

//footnote[knsh14_gioui_sample_hello_world_link][https://github.com/eliasnaur/gophercon-2019-talk/blob/master/helloworld.go]
//footnote[knsh14_gioui_app_updateevent_doc_link][https://godoc.org/gioui.org/ui/app#UpdateEvent]

=== 画像を出してみる

//list[knsh14_gioui_sample_display_image][Gioで画像を出す][go]
package main

// A simple Gio program. See https://gioui.org for more information.

import (
  "fmt"
  "image"
  _ "image/png"
  "log"
  "os"

  "gioui.org/ui"
  "gioui.org/ui/app"
  "gioui.org/ui/layout"
  "gioui.org/ui/measure"
  "gioui.org/ui/widget"
)

func main() {
  go func() {
    w := app.NewWindow()
    if err := loop(w); err != nil {
      log.Fatal(err)
    }
  }()
  app.Main()
}

func loop(w *app.Window) error {
  var cfg app.Config
  var faces measure.Faces
  file, err := os.Open("gophercon.png")
  defer file.Close()
  if err != nil {
    return err
  }
  img, _, err := image.Decode(file)
  if err != nil {
    return err
  }
  ops := new(ui.Ops)
  for {
    e := <-w.Events()
    switch e := e.(type) {
    case app.DestroyEvent:
      return e.Err
    case app.UpdateEvent:
      cfg = e.Config
      faces.Reset(&cfg)
      cs := layout.RigidConstraints(e.Size)
      fmt.Println(e.Size)
      stack := layout.Stack{Alignment: layout.Center}
      stack.Init(ops, cs)
      cs = stack.Rigid()
      dimensions := widget.Image{Src: img, Rect: img.Bounds()}.Layout(&cfg, ops, cs)
      red := stack.End(dimensions)
      stack.Layout(red)
      w.Update(ops)
    }
  }
}
//}

=== キーイベントをハンドリングしてみる
GUIアプリケーションに必須の機能として、キーボードなどの入力を受けて、何かの処理を実行することがあります。
