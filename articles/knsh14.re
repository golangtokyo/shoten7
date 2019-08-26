= 新しい GUI ライブラリ Gio で遊んでみよう！

2019年7月に開催されたGopherCon 2019@<fn>{knsh14_gophercon_link}で新しいGoのGUIライブラリが発表されました。
これまでGoの苦手な分野としてGUIを持ったアプリケーションが開発しづらいという点が挙げられていましたが、これを解決する可能性を秘めたライブラリです。


//footnote[knsh14_gophercon_link][https://www.gophercon.com/]

== Gio とは？
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
インストール方法はほかのGoのライブラリと変わらない方法でインストールできます。
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
=== Hello world

=== 画像を出してみる

=== キーイベントをハンドリングしてみる


