= @<tt>{gomobile}の現況解説

== 著者について

星一 (@<tt>{@hajimehoshi}) と申します。
趣味でGoを使っています。
代表作はEbiten@<fn>{ebiten}という2Dゲームライブラリです。
Ebitenと@<tt>{gomobile}を使ったゲームをいくつか共同制作していて、2019年8月現在の代表作は「くまのレストラン@<fn>{bearsrestaurant}」です。
GopherCon 2019 San Diegoで、Ebitenを使ったモバイルゲーム開発について発表しました@<fn>{lt}。

最近は@<tt>{gomobile}のメンテナンスを気が向いたときに行っています。
正式に「メンテナー」というポジションがあるわけではありませんが、
Goチームの人と連携し、大きめのissueに取り組んでいます。

//footnote[ebiten][@<href>{https://ebiten.org}]
//footnote[bearsrestaurant][@<href>{https://daigostudio.com/bearsrestaurant}]
//footnote[lt][@<href>{https://www.youtube.com/watch?v=zRgmXiO1el4}]

== 概要

本稿は@<tt>{gomobile}についてのまとめです。
とりわけ@<tt>{gomobile bind}のAndroid版について、内部で何を行っているのかについて説明します。
Objective-C版も流れはだいたい同じはずですので、本稿では割愛します。

本稿で使用する@<tt>{gomobile}のバージョンは次のものです。

//cmd{
597adff16ade9d88626f8caea514bb189b8c74ee
//}

ソースコードは主に@<href>{https://godoc.org/golang.org/x/mobile/cmd/gobind}からの引用です。

== @<tt>{gomobile}

@<tt>{gomobile}はAndroidおよびiOS向けのアプリケーションもしくはライブラリを出力するソフトウェアです。
パッケージは@<tt>{golang.org/x/mobile}です。

なぜ@<tt>{gomobile}がGo本体以外に必要なのかというと、Go本体ではモバイル向けのアプリケーションやライブラリを作れないからです。
Go本体では、AndroidとiOS向けに、C互換の共有ライブラリまたは静的ライブラリを出力できます。
実際@<tt>{gomobile}はGo本体にコンパイル処理を丸投げしています。
@<tt>{gomobile}はGoのコードを単にコンパイルするだけではなく、AndroidやiOSアプリケーションとして使いやすい形に変換します。
たとえばAndroidが使えるライブラリは@<tt>{*.aar}形式であり、特定のファイルをzipで固めたものになりますが、
Go本体はこれを生成してくれません。
また、たとえばGoの関数をJavaで使う際には、バインディングのためのコードが別途必要になってしまいます。
@<tt>{gomobile}はそれらをやっているわけです。

@<tt>{gomobile}コマンドは大雑把に2つのモードがあります。
@<tt>{gomobile build}と@<tt>{gomobile bind}です。

@<tt>{gomobile build}は単独のアプリケーションを出力します。
Android向けには@<tt>{*.apk}ファイル、iOS向けには@<tt>{*.ipa}ファイルです。
アプリケーション全体をすべてGoで書くことが可能です。
機能は限定されていて、アプリケーションはOpenGLサーフェスを一個だけ持ちます。

一方@<tt>{gomobile bind}はライブラリを出力します。
Android向けには@<tt>{*.aar}ファイル、iOS向けには@<tt>{*.framework}ファイルです。
Goのexportされた関数などは、一定の条件@<fn>{gobind}を満たしたものは、JavaもしくはObjective-C側でも呼び出せます。
Goで定義したinterfaceをJava側で実装してGoの関数に渡す、といったことも可能です。
あくまで共有ライブラリですので、これ単独では完成したアプリケーションになりません。
ユーザーはGo以外の言語を書く必要があります。

//footnote[gobind][@<href>{https://godoc.org/golang.org/x/mobile/cmd/gobind}]

@<tt>{gomobile build}は簡単に完成されたアプリケーションを作れる一方、柔軟性に欠けます。
@<tt>{gomobile build}で作られたアプリケーションはOpenGLサーフェス一個だけしかもっておらず、
他のコンポーネントを入れることができません。またストアに必要な署名などのファイルを挿入するのは、
不可能ではないにしろ困難です。
@<tt>{gomobile build}だけで、ストアに載せる実用アプリケーションを作ることは現実的ではありません@<fn>{tenntenn}。
実際、我々が開発しているゲームでは@<tt>{gomobile bind}を活用しています。

//footnote[tenntenn][実際に@<tt>{gomobile build}で作られたアプリケーションをストアに載せた例もあります: @<href>{https://www.slideshare.net/takuyaueda967/go-mobileandroid}]

== @<tt>{gomobile}と@<tt>{gobind}

@<tt>{golang.org/x/mobile/cmd}配下には2つコマンドが存在します。@<tt>{gomobile}と@<tt>{gobind}です。@<tt>{gomobile}は内部的に@<tt>{gobind}を呼び出しています。

普段ユーザーが使うコマンドは@<tt>{gomobile}です。@<tt>{gobind}を直接使用することも可能です。

@<tt>{gobind}コマンドはバインディング用のソースコードを生成します。
指定されたパッケージで、一定条件を満たした関すなどはJavaやObjective-Cに露出しますが、それの橋渡しとなる部分を生成します。
またリバースバインディングに関するソースコードも生成します。

一方@<tt>{gomobile}は@<tt>{gobind}で生成されたソースコードをGo本体でコンパイルし、パッケージ化します。
ソースコードをコンパイルするのはGo本体であって、@<tt>{gomobile}はあくまでGoを呼び出しているだけです。

== @<tt>{gomobile bind}の使用例

たとえば次のようなGoパッケージがあるとします。

//listnum[gomobile-go][@<tt>{mypkg}パッケージ][go]{
package mypkg

type Counter struct {
	Value int
}

func (c *Counter) Inc() { c.Value++ }

func NewCounter() *Counter { return &Counter{ 5 } }
//}

これを@<tt>{gomobile bind}でビルドすると、@<tt>{Mypkg.aar}ファイルが生成されます。
その@<tt>{Mypkg.aar}ファイルには次のようなJavaのクラスなどが定義されています。

//listnum[gomobile-java][@<tt>{Mypkg.aar}ファイルがもつ定義][java]{
public final class Counter {
	public Counter() { ... }

	public final long getValue();
	public final void setValue(long v);
	public void inc();
}
//}

Goのメソッドをもつ構造体がクラスになっていることがわかります。
メソッドはそのままJavaのクラスのメソッドになります。
exportされているフィールドはgetterおよびsetterになります。

Java側の@<tt>{inc}メソッドを呼ぶとGo側の@<tt>{Inc}関数が呼ばれます。
@<tt>{gomobile bind}によって「バインディング」、つまり多言語間の橋渡しが作られたわけです。

すべてのGoの関数がJava側で定義されるわけではありません。
たとえば関数の場合は、すべての引数が一定の条件を満たしたときにバインディングが生成されます。
詳しくは@<href>{https://godoc.org/golang.org/x/mobile/cmd/gobind}を参照してください。

なお@<tt>{gomobile}はGo moduleに対応していません。
そのため@<tt>{$GOPATH}（デフォルトは@<tt>{$HOME/go}）配下にソースが存在することが必要です。

== Reverse bindingの使用例

Reverse bindingはJavaまたはObjective-CのクラスをGo側で直接呼ぶことができる機能です。
@<tt>{Java/}もしくは@<tt>{ObjC/}から始まる、特殊なimport文を使用します。

//listnum[reversebinding][Reverse bindingの使用例][go]{
import "Java/java/lang/System"

t := System.CurrentTimeMillis()
//}

なお@<tt>{golang.org/x/mobile/example}にはreverse bindingの使用例はありません。
実は過去にありましたが消されました@<fn>{reversebindingexample}。
この仕様例はreverse bindingのみでアプリケーションを作るというものでした。
しかしJava側とGo側で循環参照を生んでしまい、
オブジェクトの寿命管理がうまくいかなくなるという問題がありました@<fn>{reversebindingreason}。
Reverse bindingのみでアプリケーションを作るのは現実的ではないようです。

//footnote[reversebindingexample][@<href>{https://go-review.googlesource.com/c/mobile/+/101155/}]
//footnote[reversebindingreason][@<href>{https://github.com/golang/go/issues/27234#issuecomment-518261769}]

== @<tt>{gomobile}の現状

@<tt>{gomobile}はElias Naur氏@<fn>{elias}がメンテナーとして活躍されていましたが、
最近メンテナーを辞めてしまい、誰もアクティブにメンテナンスしていないという状況でした。
特にmodule対応@<fn>{module}については放置されている状態でした。
そこで僭越ながら、私がこのmodule対応対策をやる人として立候補しました。

//footnote[elias][@<href>{https://github.com/eliasnaur}]
//footnote[module][@<href>{https://github.com/golang/go/issues/27234}]

ちなみにElias氏はGio@<fn>{gio}というGUIライブラリを作成しています。
GioはAndroidやiOS向けアプリケーションを生成できます。
しかしながら@<tt>{gomobile}は一切使われていません。
さきほど述べたとおり@<tt>{gomobile}を一切使わずとも自力でパッケージを作ることが可能です。
実際自力でパッケージを作ることができ、かつバインディング部分も自力で代用できるのであれば
@<tt>{gomobile}を使う必要がないわけです。
興味のある方はGioのソースコードを読んでみるとよいでしょう。

//footnote[gio][@<href>{https://gioui.org}]

== @<tt>{gobind}が何をしているか

実際に@<tt>{gomobile bind}を実行したときに何が行われるのかを追っていきます。
@<tt>{gomobile}コマンドは内部で@<tt>{gobind}コマンドを呼びます。

@<tt>{gobind}はバインディングに必要なソースコードを生成します。
ソースコードを生成するだけでコンパイルは行いません。

@<tt>{gomobile}が内部で@<tt>{gobind}を
Androidの場合、次のことを行います。

  1. @<tt>{$WORK/src/gobind}配下に@<tt>{*.go}ファイル、@<tt>{*.c}ファイル、@<tt>{*.h}ファイルを生成する
  2. @<tt>{$WORK/java}配下にJavaファイルを生成する。

@<tt>{$WORK}は一時ディレクトリです。
@<tt>{gomobile}コマンドが@<tt>{gobind}に対して@<tt>{-outdir}で指定します。
作業後に@<tt>{gomobile}は@<tt>{$WORK}を消してしまいますが、@<tt>{-work}フラグを指定すると消さないで残してくれます。
内部で何を行っているのか調査したいときに便利です。

@<tt>{gobind}は@<tt>{main}パッケージを生成します。
共有ライブラリの場合でも@<tt>{main}パッケージは必要だからです。
@<tt>{main}関数は存在しません。あったとしても自動で呼ばれません。

== @<tt>{gomobile}が何をしているか

Androidの場合は次のとおりです。

  1. @<tt>{$GOPATH}に@<tt>{$WORK}を追加し、@<tt>{gobind}という名前のパッケージをGoでビルドする。@<tt>{$WORK/android/src/main/jniLibs/[ARCH]}に@<tt>{libgojni.so}ファイルを出力する。
  2. @<tt>{$WORK/java}次のJavaファイルを${javac}でコンパイルする。@<tt>{$WORK/java-output}配下に@<tt>{*.class}ファイルを出力する。
  3. @<tt>{$WORK/java-output}に対し、@<tt>{jar c -C}コマンドで@<tt>{classes.jar}ファイルを出力する。
  4. @<tt>{AndroidManifest.xml}、@<tt>{proguard.txt}、@<tt>{R.txt}ファイルなどを生成する。
  5. 今まで生成した@<tt>{libgojni.so}、@<tt>{classes.jar}などのファイルをすべてzipで固めて、@<tt>{*.aar}ファイルを出力する。

1.は紛らわしいのですが、@<tt>{$PATH/src/gobind}にソースがある@<tt>{gobind}という名前のパッケージをビルドします。
@<tt>{gobind}コマンドが生成したものですが、@<tt>{gobind}コマンドはビルドに関与しません。
@<tt>{libgojni.so}ファイルは各アーキテクチャごとに生成されます。
デフォルトだとx86、x86-64、ARM、ARM64をすべて生成します。
アーキテクチャは@<tt>{gomobile}コマンドで指定することもできます。

@<tt>{*.aar}ファイルは実は単なるzipファイルであることがわかります。
また実は@<tt>{*.jar}ファイルもzipファイルですので、解凍することで構造が簡単にわかります。

== @<tt>{gomobile}の課題

=== module対応

最大の問題はGo module対応です@<fn>{module}。具体的にはやるべきことは次のとおりです。

  1. @<tt>{gobind}のmodule対応。
  2. @<tt>{gomobile bind}のmodule対応。
  3. @<tt>{gomobile build}のmodule対応。

//footnote[module][@<href>{https://github.com/golang/go/issues/27234}]

1.と3.は割とやるべきことは素直です。
@<tt>{go/build}を@<tt>{golang.org/x/tools/go/packages}に置き換えれば、理屈上は終わります。

ただしreverse bindingをどう扱うかなど、細かいところを決定するところが課題です。
reverse bindingのimportは@<tt>{Java}または@<tt>{ObjC}で始まります。
moduleを使わない場合は@<tt>{$GOPATH/src}配下に適切なディレクトリ構成でファイルを置けば
どんなimport文でも問題はありませんでした。
しかしmoduleを使う場合、import文の最初の要素はFQDNであることが要求されます。
@<tt>{Java}も@<tt>{ObjC}も有効なFQDNではありません。
厳密にはFQDNでなくてよい場合もあるのですが、標準ライブラリであることや@<tt>{main}パッケージであることなどが条件で、
今回はいずれも当てはまりません。
moduleとreverse bindingを両立させようとする場合、
Goの仕様に手を入れなければならない可能性があります。
暫定的な結論としては、moduleが無効のときにだけreverse bindingを有効にするというもので、
この方向で実装を進めています。

他の問題としては、@<tt>{golang.org/x/tools/go/packages}がそもそもまだ安定しておらず、
うっかり@<tt>{gomobile}の挙動を壊してしまう場合があることです@<fn>{gopackagesbug}。

//footnote[gopackagesbug][たとえば@<href>{https://github.com/golang/go/issues/33859}]

2.はあまり自明でありません。
@<tt>{gobind}は@<tt>{main}パッケージを自動生成しますが、
「開発者は@<tt>{main}パッケージが依存するライブラリを指定できる」というmoduleの仮定と合わないからです。
これについては@<tt>{gomobile}の仕様の大幅な見直しが必要な可能性があります。
