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
実際のコードを動かして、Gioがどのような機能を持っているか紹介します。
これから紹介するコードは、Go Playgroundで動かすことはできません。
ローカルで動かしてください。

=== 真っ白なWindowを出力する
まずはGioのプログラムを動かしてみましょう。
@<list>{knsh14_gioui_sample_blank_window}のプログラムを実行すると真っ白なウィンドウが現れます。

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

//list[knsh14_gioui_sample_hello_world][GioおけるHello world][go]{
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

        lbl := text.Label{
          Face: faces.For(regular, ui.Sp(72)),
          Text: "Hello, World!",
        } // HLdraw
        lbl.Layout(ops, cs) // HLdraw

        w.Update(ops)
      }
    } // HLeventloop
  }()
  app.Main()
}

// END OMIT
//}

このコードを実行すると@<img>{knsh14_gioui_hello_world}のような実行結果が得られます。
//image[knsh14_gioui_hello_world][GioにおけるHello world]{
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
@<code>{gioui.org/ui.Ops}は
@<code>{gioui.org/ui/layout.Constraints}はイベントを処理する毎に取得する必要があります。
なぜなら画面サイズが変わったりした場合に再度計算する必要があるからです。

//footnote[knsh14_gioui_sample_hello_world_link][https://github.com/eliasnaur/gophercon-2019-talk/blob/master/helloworld.go]
//footnote[knsh14_gioui_app_updateevent_doc_link][https://godoc.org/gioui.org/ui/app#UpdateEvent]

=== レイアウトを変更する
右寄せで画面のN%部分に表示したいという状況はGUIアプリケーションを作っているとよく遭遇します。
そのためのサンプルとして、@<list>{knsh14_gioui_sample_layout_image}にレイアウトを自由に変更するサンプルを示します。


//list[knsh14_gioui_sample_layout_image][Gioでウィンドウサイズに依存した表示を行う][go]{
package main

import (
  "image"
  _ "image/png"
  "log"
  "os"

  "gioui.org/ui"
  "gioui.org/ui/app"
  "gioui.org/ui/layout"
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
    case app.UpdateEvent:
      cfg = e.Config
      ops.Reset()
      cs := layout.RigidConstraints(e.Size)
      flex := layout.Flex{}
      flex.Init(ops, cs)
      cs = flex.Flexible(0.5)
      dimensions := widget.Image{
        Src:  img,
        Rect: img.Bounds(),
      }.Layout(&cfg, ops, cs)
      child1 := flex.End(dimensions)
      cs = flex.Flexible(0.5)
      dimensions = widget.Image{
        Src:  img,
        Rect: img.Bounds(),
      }.Layout(&cfg, ops, cs)
      child2 := flex.End(dimensions)
      flex.Layout(child1, child2)
      w.Update(ops)
    }
  }
}
//}

@<list>{knsh14_gioui_sample_layout_image}を実行すると、画面の同じ画像が横に2つ並んでいるウィンドウが開きます。
端をドラッグしてウィンドウのサイズを変更するとそれに伴って画像のサイズも変化します。

画像などのレイアウトを変更する場合は@<code>{gioui.org/ui/layout}パッケージを利用します。
@<code>{Flex}は横方向に要素を並べます。
横幅に対する割合は@<code>{Flexible}メソッドで決定し、その戻り値を利用して表示を行います。


=== キーイベントをハンドリングしてみる
GUIアプリケーションに必須の機能として、キーボードなどの入力を受けて処理を実行することが挙げられます。
@<list>{knsh14_gioui_sample_handle_input}はキーボード入力があった際にコンソール上に入力された文字を表示するプログラムです。

//list[knsh14_gioui_sample_handle_input][Gioでキーボード入力を表示するサンプル][go]{
package main

import (
  "fmt"
  "log"

  "gioui.org/ui/app"
  "gioui.org/ui/key"
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
  for {
    e := <-w.Events()
    switch e := e.(type) {
    case key.Event:
      v := string([]rune{e.Name})
      fmt.Println(v)
    }
  }
}
//}

キーボード入力は@<code>{gioui.org/ui/key.Event}というイベントで定義されています。
このイベントが来た場合に中身を取り出して処理をすることができます。
