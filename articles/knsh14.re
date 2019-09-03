= GUIライブラリGioで遊んでみよう

2019年7月に開催されたGopherCon 2019@<fn>{knsh14_gophercon_link}で新しいGoのGUIライブラリが発表されました。
これまでGoの苦手な分野としてGUIを持ったアプリケーションが開発しづらいという点が挙げられていましたが、これを解決する可能性を秘めたライブラリです。


//footnote[knsh14_gophercon_link][https://www.gophercon.com/]

== Gioの概要
Gio@<fn>{knsh14_gioui_link}はGo製のGUIライブラリです。
作者のElias NaurさんはGo Mobileを開発されていた方です。
その経験を踏まえつつ、今回さらに強力なライブラリを設計しました。

Gioはまだv1.0.0がリリースされておらず、今でも大きな変更があります。
2019年9月2日執筆時点でのバージョン@<fn>{knsh14_gio_version}を正式なものとして扱います。

//footnote[knsh14_gioui_link][https://gioui.org]
//footnote[knsh14_gio_version][https://git.sr.ht/~eliasnaur/gio/commit/dc62058bcefc51bd138d12668bba5a11dfed3e3f]

=== 特徴
大きな特徴として私達が書くアプリケーションのコードをすべてGoで書ける点にあります。
さらに、Goそのものの特徴であるクロスプラットフォームにも対応しています。
対応しているプラットフォームはiOS、AndroidはもちろんtvOSやWebGLなどにも出力できます。

=== インストール方法
インストール方法は一般的なGoのライブラリと同じ方法でインストールできます。
ライブラリの取得方法を@<list>{knsh14_gioui_install}に示します。

//listnum[knsh14_gioui_install][Gio UIのインストール方法][shell]{
$ go get -u gioui.org/...
//}

開発中は@<code>{go run main.go}のようにそのまま実行できます。
実際にアプリケーションをビルドしてバイナリにする場合は@<list>{knsh14_gioui_builds}に上げたGioのコマンドを実行する必要があります。

//listnum[knsh14_gioui_builds][Gioアプリケーションのビルド方法][shell]{
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

== Gioで遊んで見る
実際のコードを動かして、Gioがどのような機能を持っているか紹介します。
これから紹介するコードは、Go Playgroundで動かすことはできません。
ローカルで動かしてください。

=== 真っ白なWindowを出力する
まずはGioのプログラムを動かしてみましょう。
@<list>{knsh14_gioui_sample_blank_window}のプログラムを実行すると真っ白なウィンドウが現れます。

#@# textlint-disable
//listnum[knsh14_gioui_sample_blank_window][真っ白なWindow][go]{
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
#@# textlint-enable

@<code>{main}関数で@<code>{gioui.org/ui/app}の@<code>{Main}関数を呼び出します。
この関数は必ず呼ぶ必要があります。@<code>{gioui.org/ui/app.Main}関数はすべてのウィンドウが閉じられるまで@<code>{main}関数をブロックします。
ウィンドウに画像や文字を出したり、キーの入力を受けたりする処理は別の@<code>{goroutine}で実行する必要があります。

=== Hello World
GopherConで例として利用されたHello Worldのコード@<fn>{knsh14_gioui_sample_hello_world_link}を実行します。

#@# textlint-disable
//listnum[knsh14_gioui_sample_hello_world][Hello Worldを表示するサンプル][go]{
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
#@# textlint-enable

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
これら2つを指定してどのように画面に表示するかを決定します。

文字を実際に出すためには@<code>{Layout}メソッドを呼び出す必要があります。
このメソッドに渡すために、@<code>{gioui.org/ui.Ops}と@<code>{gioui.org/ui/layout.Constraints}を取得する必要があります。
@<code>{gioui.org/ui.Ops}は画面更新などの処理を保存して、画面更新時に渡すための変数です。
@<code>{Ops}は@<code>{Reset}メソッドを使って中身をクリアできるので、何度も使い回すことが推奨されます。
@<code>{gioui.org/ui/layout.Constraints}はイベントを処理する毎に取得する必要があります。
なぜなら画面サイズが変わったりした場合に再度計算する必要があるからです。

//footnote[knsh14_gioui_sample_hello_world_link][https://github.com/eliasnaur/gophercon-2019-talk/blob/master/helloworld.go]
//footnote[knsh14_gioui_app_updateevent_doc_link][https://godoc.org/gioui.org/ui/app#UpdateEvent]

=== レイアウトを変更する
右寄せで画面のN%部分に表示したいという状況はGUIアプリケーションを作っているとよく遭遇します。
そのためのサンプルとして、@<list>{knsh14_gioui_sample_layout_image}にレイアウトを自由に変更するサンプルを示します。

#@# textlint-disable
//listnum[knsh14_gioui_sample_layout_image][Gioでウィンドウサイズに依存した表示を行う][go]{
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
#@# textlint-enable

@<list>{knsh14_gioui_sample_layout_image}を実行すると、画面の同じ画像が横に2つ並んでいるウィンドウが開きます。
端をドラッグしてウィンドウのサイズを変更するとそれに伴って画像のサイズも変化します。

画像などのレイアウトを変更する場合は@<code>{gioui.org/ui/layout}パッケージを利用します。
@<code>{Flex}は横方向に要素を並べます。
横幅に対する割合は@<code>{Flexible}メソッドで決定し、その戻り値を利用して表示を行います。


=== キーイベントをハンドリングする
GUIアプリケーションに必須の機能として、キーボードなどの入力を受けて処理を実行することが挙げられます。
@<list>{knsh14_gioui_sample_handle_input}はキーボード入力を受け付けて、ウィンドウ上に文字を表示するものです。

#@# textlint-disable
//listnum[knsh14_gioui_sample_handle_input][Gioでキーボード入力を表示するサンプル][go]{
package main

import (
  "log"

  "gioui.org/ui"
  "gioui.org/ui/app"
  "gioui.org/ui/key"
  "gioui.org/ui/layout"
  "gioui.org/ui/measure"
  "gioui.org/ui/text"
  "golang.org/x/image/font/gofont/goregular"
  "golang.org/x/image/font/sfnt"
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
  regular, _ := sfnt.Parse(goregular.TTF)
  var faces measure.Faces
  ops := new(ui.Ops)
  var input string
  for {
    e := <-w.Events()
    switch e := e.(type) {
    case app.UpdateEvent:
      cfg = e.Config
      faces.Reset(&cfg)
      ops.Reset()
      queue := w.Queue()
      for e, ok := queue.Next(input); ok; e, ok = queue.Next(input) {
        if v, ok := e.(key.Event); ok {
          input += string([]rune{v.Name})
        }
      }
      cs := layout.RigidConstraints(e.Size)
      text.Label{
        Face: faces.For(regular, ui.Dp(72)),
        Text: input,
      }.Layout(ops, cs)
      key.InputOp{Key: input, Focus: false}.Add(ops)
      w.Update(ops)
    }
  }
}

//}
#@# textlint-enable

キーボード入力などの入力は@<code>{gioui.org/ui/input.Queue}というインターフェースを通して取得します。
このQueueは@<code>{app.Window}オブジェクトから取得します。
取得できるイベントの種類はキーボードの入力以外にも、マウスのクリックやウィンドウのフォーカスなどがあります。

== Gioでのアプリケーション設計
サンプルを実行することで、ある程度Gioの機能をどのように利用するか説明しました。
ここからはGioを実際に使う上で気をつけることを説明します。

=== GioのWindowは状態を持たない
GopherConのtalkでもGioは状態を持たないという説明がされています。
前の画面更新時の状態を持っていることもありません。
GioのWindowオブジェクトは全体の描画イベントなどのQueueのchannel、入力イベントのQueueや、画面再描画のメソッドなどだけを提供します。
画面の要素の状態や、再描画の際のフックのためのコールバック関数は定義されていません。

画面が今どう表示されているか、そのためのデータはどのような状態なのかは自分たちで管理する必要があります。
これはアプリケーションの状態を管理するコードを上手く自分たちで管理する必要があるため、作るのが大変になります。
ですが、自分たちですべてを管理できるので、テストコードで正しく状態遷移できているかを担保しやすいというメリットがあります。

Gioを動かす部分、UIの描画などをハンドリングする部分、実際のデータなどを操作する機能などでうまくアプリケーションを設計する必要があります。

=== layoutをうまく利用する
@<code>{gioui.org/ui/layout}はGioの薄いライブラリの中にあって、かなり自由な画面構成を作成できます。
固定されたサイズで描画するよりも柔軟なレイアウトが可能になるので、さまざまな画面サイズに対応できます。

とはいえ縦長の画面、横長の画面などそれぞれで適したUIが変わってきます。
自分のアプリケーション、利用されるプラットフォームにあった画面を意識しましょう。

== Gioへのコントリビュート
Gioへ興味が湧いてきて、いろいろ使っているとバグを見つけたり、修正したいことが出てくると思います。
ぜひ積極的にコントリビュートしていきましょう。
//}
#@# textlint-enable

