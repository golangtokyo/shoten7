= Goとベイズ理論でゼロから記事分類を実装してみよう！！

== はじめに

白ヤギコーポレーションでバックエンドエンジニアをしている@po3rin@<fn>{po3rin}です。今回は Go とベイズ理論を使った記事分類の実装方法を紹介します。Naive Baye の理論の解説や Go で実装する上でのポイントなども含めてお話しします。Go で記事分類を実装することで、すでに Go で実装されたアプリケーションへの組み込みも容易になります。

#@# TODO: motivate

//footnote[po3rin][@<href>{https://twitter.com/po3rin}]

== ベイズの理論と Naive Bayes

この節では Naive Bayse の理論をザックリと理解することがゴールです。なんとなくやっていることが分かる程度で大丈夫です。

=== さらっとベイズの理論を抑えよう

ベイズ理論(@<img>{Bayes})とは「条件付き確率に関して成り立つ定理」のことです。

//image[Bayes][ベイズの理論][scale=0.8]{
//}

一見すると難しそうに見えますが、１つずつ見ていくとそれほ程難しくはないです。

#@# TODO: table

//table[p][確率の表記]{
式      意味
------------------------------------
P(A)    Aが起こる確率
P(B|A)  Aが起こった時にBが起こる確率
//}

記事のカテゴリ分類を例にベイズの定理の使い方を見ていきましょう。知りたいのは「ドキュメントが与えられた時に、とあるカテゴリに分類される確率」です。これは@<table>{p}を見返すと@<code>{P(C|D)}と表せることがわかります。ベイズの理論より @<code>{P(C|D)} は @<code>{P(C)}（カテゴリーの出現率）と @<code>{P(D|C)}（カテゴリーが与えられた時のドキュメント出現率）の掛け算です(@<code>{P(D)}（カテゴリー内のドキュメント出現率）はすべてのカテゴリで同じであるため無視できます）。

#@# TODO: refact

=== 具体例から Naive Bayes の具体的な計算方法を理解する

理論だけだと何が行われているのか分からないと思うので具体的な例で理解しましょう。次の5つの文章があります。

経済 : 株価
経済 : 保険
IT: Pythonで株価
IT: PythonとGo
エンタメ: マーベル

#@# TODO: table

==== P(C）

@<code>{P(C)} はカテゴリーの出現率です。たとえば「経済カテゴリ」の出現率は @<code>{経済カテゴリの数 / カテゴリ総数}なので 2/5 になります。

#@# TODO: table

==== P(D|C）

@<code>{P(D|C)} はカテゴリーが与えられた時のドキュメント出現率でした。ドキュメントは一連の単語で構成されているため @<code>{P(D|C)} はカテゴリ内のドキュメントのすべての単語の結合確率で計算できます。これは計算が難しいように思えます。一方でカテゴリ内の単語出現率は簡単です。ITカテゴリには単語が4つあるので、たとえば「Python」が「ITカテゴリ」に出現する確率は 2/4 、「株価」が「ITカテゴリ」に出現する確率は 1/4 です。

#@# TODO: table

この数字を使って欲しい @<code>{P(D|C)} を計算します。Naive Bayse では @<code>{P(D|C)} は 各単語の確率の掛け算で表せます。@<code>{P(Pythonで株価|IT)} は (「Python」が「ITカテゴリ」に出現する確率 2/4 ）* (「株価」が「ITカテゴリ」に出現する確率 1/4 ）で 1/8 になります。

#@# TODO: table

実は今までの計算では本来あるべき単語の条件付き確率を無視し、各単語が互いに独立していると仮定してます。つまり単語がドキュメント内にランダムに現れると仮定しているのです。この過程が Naive Bayse が Naive たる所以です。

== Go で Naive Bayes Classifier を実装しよう

では実際に Go で Naive Bayes を実装していきましょう。

=== Goによるテキストの前処理

具体的な Classifier を実装する前に入力されるテキストを前処理する必要があります。今回は英語を対象にするので、英語のテキストを前処理する必要があります。

==== stop words の削除

「the」や 「it」などの高頻度で出現する単語はその文章の特徴を表す単語とはいえない為、前処理で削除しておく必要があります。このような高頻度で出現する単語を自然言語処理界隈で stop words を呼ばれています。では Go で stop words を検知しましょう。

//list[stopwords][stop wordsの検知][go]{
package gonbayes

var stopWords = map[string]struct{}{
	"i":          struct{}{},
	"me":         struct{}{},
	"my":         struct{}{},
	"myself":     struct{}{},

    // 省略 (github.com/po3rin/gonbayes/blob/master/const.go を参照)
}

func isStopWords(word string) bool {
	_, ok := stopWords[word]
	return ok
}
//}

stop words には Python で提供されている NLTK(Natural Language Tool Kit） で使われている stop words のリスト@<fn>{swlidt}から拝借しました。
//footnote[swlidt][ここで一覧にしてくれているのをみつけました。https://gist.github.com/sebleier/554280]

stop words のマップは key に stop word を置き、value に空の構造体を置きます。Goにおいて空の構造体はメモリを消費しません。その為マップが存在確認のためだけに存在する場合は空の構造体が有用です。知らなかった人は実際に@<code>{unsafe.Sizeof}を使って0になることを確認しましょう。

//list[innnerfunc][空の構造体][go]{
fmt.Println(unsafe.Sizeof(s))
//}

これで任意の単語が stop words か検知する関数ができました。

=== 単語のステミング処理

swimという単語は「swims」「swimming」「swimmer」などの複数の語形があるので確率を正しく計算できません。単語の語形が変化する言語では前処理としてステミング処理を行う必要があります。とりあえず英語で何も考えずに使えるステミングアルゴリズムは、「Lancaster」「Porter」「Snowball Stemmers」などいろんな種類がある。Goのパッケージとして提供されているステミングライブラリは調べたところ下記が候補。

#@# TODO: list of stemming package in Go

=== Goによる Naive Bayes Classifier の実装

== 口コミの ネガティブ or ポジティブ 判定をやってみよう

== ニュース記事のカテゴリ分類をやってみよう

== 訓練済み分類器を gob パッケージで保存する

== おわりに
