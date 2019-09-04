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
ローカルで@<code>{go run}コマンドを利用して動かしてください。

=== 真っ白なWindowを出力する
最初のサンプルとしてシンプルなGioのプログラムを動かしてみます。
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

22行目のfor文でウィンドウオブジェクトからイベントを取り出します。
イベントには「画面を更新した」、「何らかの入力を受けた」などがあります。
このサンプルでは画面を更新する際のイベント@<code>{gioui.org/ui/app.UpdateEvent}@<fn>{knsh14_gioui_app_updateevent_doc_link}の場合に文字を出す処理を行います。

文字を出力するためには@<code>{gioui.org/ui/text.Label}を利用します。
@<code>{Label}には2つのフィールドがあります。
1つ目はもちろん出力するための@<code>{Text}フィールドです。
2つ目は文字のフォント、大きさを決める@<code>{Face}フィールドです。
これら2つを指定してどのように画面に表示するかを決定します。

文字を実際に出すためには@<code>{Layout}メソッドを呼び出します。
このメソッドに渡すために、@<code>{gioui.org/ui.Ops}と@<code>{gioui.org/ui/layout.Constraints}を取得する必要があります。
@<code>{gioui.org/ui.Ops}は画面更新などの処理を保存して、画面更新時に渡すための変数です。
@<list>{knsh14_gioui_sample_hello_world}のサンプルでは1つのラベルだけを出すので、あまり役に立つように見えません。
ですが、複数の描画処理をしたい場合にそれぞれを@<code>{Ops}に格納し、一度に処理させることができます。
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
@<code>{Flex}は画面の比率に対して要素をどれぐらいのサイズで並べるかという方法です。
横幅に対する割合は@<code>{Flexible}メソッドで最大の画面比率を決定し、その戻り値を利用して表示を行います。


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

キーボード入力などの入力は@<code>{gioui.org/ui/input.Queue}というインタフェースを通して取得します。
このQueueは@<code>{app.Window}オブジェクトから取得します。
取得できるイベントの種類はキーボードの入力以外にも、マウスのクリックやウィンドウのフォーカスなどがあります。

== Gioでのアプリケーション設計
サンプルを実行することで、ある程度Gioの機能をどのように利用するか説明しました。
ここからはGioを実際に使う上で気を付けることを説明します。

=== GioのWindowは状態を持たない
GopherConのtalkでもGioは状態を持たないという説明がされています。
前の画面更新時の状態を持っていることもありません。
GioのWindowオブジェクトは全体の描画イベントなどのQueueのchannel、入力イベントのQueueや、画面再描画のメソッドなどだけを提供します。
画面の要素の状態や、再描画の際のフックのためのコールバック関数は定義されていません。

画面が今どう表示されているか、そのためのデータはどのような状態なのかは自分たちで管理する必要があります。
これはアプリケーションの状態を管理するコードを自分たちで管理する必要があるため、より自分たちでコードを書く必要があります。
ですが、自分たちで状態の変化を管理できるので、テストコードで正しく状態遷移できているかを担保しやすいというメリットがあります。

Gioを動かす部分、UIの描画などをハンドリングする部分、実際のデータなどを操作する機能などでうまくアプリケーションを設計する必要があります。

=== layoutをうまく利用する
@<code>{gioui.org/ui/layout}はGioの薄いライブラリの中にあって、かなり自由な画面構成を作成できます。
固定されたサイズで描画するよりも柔軟なレイアウトが可能になるので、さまざまな画面サイズに対応できます。

とはいえ縦長の画面、横長の画面などそれぞれで適したUIが変わってきます。
自分のアプリケーション、利用されるプラットフォームにあった画面を意識しましょう。

== Gioへのコントリビュート
Gioへ興味が湧いてきて、いろいろ使っているとバグを発見したり、修正したいことが出てきます。
ぜひ積極的にGioのコミュニティに参加して、コントリビュートしていきましょう。

=== 質問をする
Gophers Slackには@<tt>{#gioui}というGioに関連した話題について議論するチャンネルがあります。
使い方についての質問やベストプラクティスに関する質問など困ったことがあれば参加して聞いて見るとよいでしょう。
Eliasさんもこのチャンネルを見てくださっているので、本人から意見がもらえることもあります。

=== Gioに修正を投げる
Gioはsourcehut@<fn>{knsh14_sourcehut_link}という日本ではなかなか見かけないホスティングサービスを使ってコードを管理しています。
sourcehutはかなりシンプルな機能を提供しているサービスで、GitHubでの開発に慣れていると分かりづらいことが多いです。
Pull Requestのようなブランチ間の差分をウェブ上でレビューする仕組みもないため、メーリングリストにパッチを送る必要があります。
以前GitHubに移行するかどうかという議論がありましたが、GitHubにロックインされることを懸念した作者のEliasさんが移行する予定はないとしています。

そのための手順について説明をします。
私はGmailを使っています。
なので、Gmailを利用したパッチ送信について説明します。
なお、すでにローカルにリポジトリがcloneされている状態を想定しています。

//footnote[knsh14_sourcehut_link][https://sourcehut.org/]

=== 事前準備
実際にパッチを投げる前にいくつか事前準備を行います。

==== sourcehutでアカウントを作成する
sourcehutのサイト@<fn>{knsh14_sourcehut_service_link}へアクセスし、アカウントを作成しておきます。
この手順は必須ではありませんが、やっておくとうまく行かなかった際に実際に自分でリポジトリを作成して実験できます。

//footnote[knsh14_sourcehut_service_link][https://git.sr.ht/]

==== GmailのIMAPを有効にする
Gmailの画面に行き、設定項目の「メール転送と POP/IMAP」を表示し、IMAPが有効にします。
他の設定はデフォルトのままで大丈夫です。

==== Gmailのアカウントでアプリパスワードを作成する
まず初めにGoogleアカウントの二段階認証を有効にさせます。
二段階認証が有効になると、アプリパスワードが作成できるようになります。

セキュリティ項目のアプリパスワード生成画面で「アプリ」と「デバイス」を選択する画面が出てきます。
アプリを「メール」、デバイスを「その他」にして「git send-email」と入力し、生成ボタンを押します。
するとパスワードが表示されるので、パスワードマネージャーなどに記憶させます。

==== gitの設定を追加する
cloneされたリポジトリの中で@<tt>{.git/config}に@<list>{knsh14_gioui_repo_git_setting}の設定を行います。

#@# textlint-disable
//listnum[knsh14_gioui_repo_git_setting][Gioのリポジトリに設定する項目][gitconfig]{
[sendemail]
  to = ~eliasnaur/gio@lists.sr.ht
  annotate = yes
  smtpEncryption = tls
  smtpServer = smtp.gmail.com
  smtpUser = username@gmail.com
  smtpServerPort = 587
//}
#@# textlint-enable

@<tt>{sendmail.to}を設定するのはパッチの送り先がGioのメーリングリストしかないからです。
この値が設定されているとデフォルトの送信先として選ばれます。
@<tt>{sendmail.annotate}が有効になっていると、パッチを送信する際に確認画面を出してくれます。
下の4行はGmail用の設定です。@<tt>{sendmail.smtpUser}の部分は自分のGmailアドレスに変更してください。

=== 修正を行う
取り入れて欲しい変更を実際に修正します。
この操作は普段と同じようにブランチを切って修正します。

1つだけ注意する必要があるのがコミットするときです。
@<code>{git commit}コマンドを使いますがその際に@<code>{-s}もしくは@<code>{--signoff}オプションを付けてコミットする必要があります。
このオプションを付けることで、Developer Certificate of Origin@<fn>{knsh14_certificate_of_origin}に同意し、自分の変更がGioのライセンスに属することを証明します。

//footnote[knsh14_certificate_of_origin][https://developercertificate.org/]

=== 実際にパッチを送信する
では実際にパッチをメーリングリストに送信します。
@<list>{knsh14_gio_git_send-email}は最新のコミットだけをパッチにして送るコマンドです。

#@# textlint-disable
//listnum[knsh14_gio_git_send-email][メーリングリストにパッチを送るためのコマンド][bash]{
$ git send-email HEAD^
//}
#@# textlint-enable

このコマンドを実行すると、エディタが開き、@<list>{knsh14_send_email_editor}の画面が開きます。

#@# textlint-disable
//listnum[knsh14_send_email_editor][git send-emailコマンド実行時][vim]{
  1 From HASH ANSIC_Style_time
  2 From: GIT_USERNAME <USERNAME@gmail.com>
  3 Date: RFC1123Z_style_time
  4 Subject: [PATCH] PATCH_TITLE
  5 
  6 Signed-off-by: GIT_USERNAME <USERNAME@gmail.com>
  7 ---
  ... ここにパッチの内容が表示されます。
 33 ---
 34 2.23.0
 35 
//}
#@# textlint-enable

この画面でSubjectに自分が変更したパッケージのパスを自分で書き足す必要があります。
これはリポジトリのスタイルで決められています。
確認して問題なければ保存して、エディタを終了させます。

成功すると@<list>{knsh14_send_email_annotate}のような確認画面が現れます。
再度タイトル、内容を確認し、問題がなさそうならyを入力して進みます。

#@# textlint-disable
//listnum[knsh14_send_email_annotate][git send-emailコマンド実行前の確認画面][bash]{
/PATH/TO/MY/PATCH/FILE.patch
@<tt>{(mbox) Adding cc: GIT_USERNAME <USERNAME@gmail.com> from line 'From: GIT_USERNAME <USERNAME@gmail.com>'}
@<tt>{(body) Adding cc: GIT_USERNAME <USERNAME@gmail.com> from line 'Signed-off-by: GIT_USERNAME <USERNAME@gmail.com>'}

From: GIT_USERNAME <USERNAME@gmail.com>
To: ~eliasnaur/gio@lists.sr.ht
Cc: GIT_USERNAME <USERNAME@gmail.com>
Subject: [PATCH] PATCH_TITLE
Date: RFC1123Z_style_time
Message-Id: <ID-USERNAME@gmail.com>
X-Mailer: git-send-email 2.23.0
MIME-Version: 1.0
Content-Transfer-Encoding: 8bit

    The Cc list above has been expanded by additional
    addresses found in the patch commit message. By default
    send-email prompts before sending whenever this occurs.
    This behavior is controlled by the sendemail.confirm
    configuration setting.

    For additional information, run 'git send-email --help'.
    To retain the current behavior, but squelch this message,
    run 'git config --global sendemail.confirm auto'.

Send this email? ([y]es|[n]o|[e]dit|[q]uit|[a]ll): y

//}
#@# textlint-enable

送信する際にパスワードが求められます。
ここで事前準備で作成したアプリパスワードを入力します。
そうすると送信されて、メーリングリストに自分のパッチが掲載されます。
パッチ一覧@<fn>{knsh14_gio_patch_list_url}に自分のパッチが送られているか確認しましょう。

#@# textlint-disable
//listnum[knsh14_send_email_password][パッチ送信時のパスワード入力画面][bash]{
Password for 'smtp://USERNAME@gmail.com@smtp.gmail.com:587':
OK. Log says:
Server: smtp.gmail.com
MAIL FROM:<USERNAME@gmail.com>
RCPT TO:<~eliasnaur/gio@lists.sr.ht>
RCPT TO:<USERNAME@gmail.com>
From: GIT_USERNAME <USERNAME@gmail.com>
To: ~eliasnaur/gio@lists.sr.ht
Cc: GIT_USERNAME <USERNAME@gmail.com>
Subject: [PATCH] PATCH_TITLE
Date: RFC1123Z style time
Message-Id: <ID-USERNAME@gmail.com>
X-Mailer: git-send-email 2.23.0
MIME-Version: 1.0
Content-Transfer-Encoding: 8bit

Result: 250 2.0.0 OK  1567093114 f63sm7776226pfa.144 - gsmtp
//}
#@# textlint-enable

//footnote[knsh14_gio_patch_list_url][https://lists.sr.ht/~eliasnaur/gio]

== まとめ
GopherConで発表された新しいGUIライブラリGioについて説明しました。
Gioはまだ若いライブラリですが、作者が同じなのでGo Moblleと使い勝手が似ています。
昔Go Mobileを利用していた方はすんなり馴染めそうです。

Goで自作したCLIコマンドがGUI上で動作すると、さらに楽しい気持ちになります。
ぜひGioで新しいGoのアプリケーションを作ってみてください。