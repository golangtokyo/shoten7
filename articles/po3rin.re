#@# textlint-disable
= Goとベイズ理論でゼロからシンプルな記事分類を実装してみよう！

== はじめに

白ヤギコーポレーションでバックエンドエンジニアをしている@po3rin@<fn>{po3rin}です。仕事ではGoをメインにサーバーサイドの開発をしています。今回はGoとベイズ理論を使ったシンプルな記事分類の実装方法を紹介します。Naive Bayesの理論を簡単に学べるのはもちろんですが、Goでのテキスト前処理の方法や、学習済み分類器の保存方法なども含めてご紹介します。本章でGoでも自然言語処理ができるというイメージを持っていただければ幸いです。今回実装するコードは@<code>{github.com/po3rin/gonbayes}@<fn>{gonbayes}というリポジトリ名でGitHubに公開しているので参考にしてください。

#@# TODO: motivate

//footnote[po3rin][@<href>{https://twitter.com/po3rin}]
//footnote[gonbayes][@<href>{https://github.com/po3rin/gonbayes}]

== ベイズの理論とNaive Bayes

この節ではNaive Bayesの理論をザックリと理解することがゴールです。なるべく難しい数式は使わずに説明してみます。実装に困らない程度に「なんとなくやっていることが分かる」程度で大丈夫です。

=== さらっとベイズの理論を抑えよう

ベイズの理論(@<img>{Bayes})とは「条件付き確率に関して成り立つ定理」のことです。

//image[Bayes][ベイズの理論][scale=0.4]{
//}

一見すると難しそうに見えますが、１つずつ式の意味を見ていくとそれほど難しくはないです (@<table>{p})。ちなみに@<m>{P(B|A）}は@<em>{P B given A}と読みます。

//table[p][確率の表記]{
式	意味
------------------------------------
P(A)	Aが起こる確率
P(B|A)	ある事象 A が起こったという条件のもとでの事象 B の確率(事後確率)
//}

ドキュメントのカテゴリ分類を例にベイズの定理の使い方を見ていきましょう。知りたいのは@<em>{「ドキュメントが与えられた時に、とあるカテゴリに分類される確率」}です。これは@<table>{p}を見返すと@<m>{P(C|D）}と表せることがわかります(@<m>{C} はCategory、@<m>{D}はDocumentを表しています)。最終的に1つのドキュメントに対して各カテゴリごとの@<m>{P(C|D）}を計算して@<m>{P(C|D）}がもっとも大きい値のカテゴリにドキュメントを分類します(@<img>{Which})。

//image[Which][各カテゴリごとのP(C|D）の結果から確率がもっとも大きいカテゴリにドキュメントを分類][scale=0.7]{
//}

ベイズの理論(@<img>{Bayes})より @<m>{P(C|D）}は@<m>{P(C）}(カテゴリーの出現率）と@<m>{P(D|C）}（カテゴリーが与えられた時のドキュメント出現率）を掛けて@<m>{P(D）}(ドキュメントの出現率）で割った値になります。一方でカテゴリごとに@<m>{P(D）}は不変なのでカテゴリごとの@<m>{P(C|D）}の計算結果の大小関係に影響を及ぼしません。その為、@<m>{P(D）}の値は無視できます。よって今回Goで実装する計算は@<img>{Pcd}です。

//image[Pcd][記事分類で使う為にベイズ理論から式変形(CはCategory、DはDocument)][scale=0.4]{
//}

=== 具体例からNaive Bayesの具体的な計算方法を理解する

@<img>{Pcd}を計算したいので次は@<m>{P(C）}と@<m>{P(D|C）}について考えます。ここからは理論だけだと何が行われているのか分からないと思うので具体的な例で理解していきましょう。@<table>{documents}のように5つのドキュメントがある場合を考えます。

//table[documents][ドキュメントの実際の例]{
カテゴリ	ドキュメント
------------------------------------
経済	Price
経済	Insurance
IT	Python and Price
IT	Python and Go
エンタメ	Marvel
//}

==== P(C）

@<m>{P(C）}はカテゴリーの出現率です。たとえば「経済カテゴリ」の出現率は@<em>{経済カテゴリの数/カテゴリ総数（重複分もカウント）}なので@<m>{2/5}になります。

//table[pc][P(C）の例]{
カテゴリ	P(C）
------------------------------------
経済	2/5
IT	2/5
エンタメ	1/5
//}

==== P(D|C）

@<m>{P(D|C）}はカテゴリーが与えられた時のドキュメント出現率でした。これは計算が難しいように思えます。一方でカテゴリ内の単語出現率は簡単です(これを@<m>{P(W|C）}とします。@<m>{W}は"Word"の頭文字です）。たとえばITカテゴリには単語が4つある中で「Python」は2回出てきているので「Python」が「ITカテゴリ」に出現する確率は@<em>{2/4}です。

//table[pwc][P(W|C）の例]{
.	経済	IT	エンタメ
------------------------------------
Price	1/2	1/4	0/1
Insurance	1/2	0/4	0/1
Python	0/2	2/4	0/1
Go	0/2	1/4	0/1
Marvel	0/2	0/4	1/1
//}

では@<m>{P(D|C）}はどのように計算するのでしょうか。一番シンプルな方法として@<m>{P(D|C）}を各単語の出現確率(@<m>{P(W|C）})の掛け算で計算する方法が知られています(@<img>{Pdc})。@<img>{Pdc}の導出については確率の連鎖律などの説明で込み入ってくるのでここでは割愛します。

//image[Pdc][P(D|C）を各単語の出現確率P(W|C）の掛け算で計算する][scale=0.4]{
//}

@<img>{Pdc}を使うと、たとえばITカテゴリが"Python and Go"というドキュメントに分類される確率@<m>{P(D|C）}は(「Python」が「ITカテゴリ」に出現する確率@<m>{2/4}）*(「Go」が「ITカテゴリ」に出現する確率@<m>{1/4}）で@<m>{1/8}になります(andなどのドキュメントに頻出する単語は @<em>{stop words}と呼ばれ、今回の分類器の実装では無視します。次の節で詳しく説明します）。


//table[pdc][P(D|C）の例]{
.	ITカテゴリがドキュメントが分類される確率 P(D|C）
------------------------------------
Python and Go	2/4 * 1/4 = 1/8
//}

ここまでで欲しかった@<m>{P(C|D）}を計算できます。復習ですが@<m>{P(C|D）}は@<m>{P(C）}と@<m>{P(D|C）}の掛け算です

//table[pcd][P(C|D）の例]{
.	ドキュメントがITカテゴリに分類される確率(P(C|D）= P(D|C）*P(C）)
------------------------------------
Python and Go	1/8 * 2/5 = 1/20
//}

これで"Python and Go"というドキュメントがITカテゴリに含まれる確率は@<m>{0.05}という結果が計算できました。分類においては分類したいドキュメントに対してそれぞれのカテゴリで@<m>{P(C|D）}を計算し、もっとも高い確率の物を分類の結果として採択します。

== Goによるテキストの前処理

前節では Naive Bayesの理論について簡単に紹介しました。早速GoでNaive Bayesを実装したいのですが、その前にテキストの前処理を行う関数をGoで実装しておきましょう。今回は英語を対象にするので、英語のテキストを前処理する必要があります。本章のコードではGitHubのサンプルパッケージ名と同じ「gonbayes」パッケージとして実装していきますが好きなパッケージ名で構いません。

=== stop words の削除

「the」や「it」などの高頻度で出現する単語はその文章の特徴を表す単語とはいえない為、前処理で削除しておく必要があります。このような高頻度で出現する単語を自然言語処理界隈でstop wordsを呼ばれています。ではGoでstop wordsを検知しましょう。

//list[stopwords][stop wordsの検知][go]{
// ./utils.go
package gonbayes

var stopWords = map[string]struct{}{
	"i":          struct{}{},
	"me":         struct{}{},
	"my":         struct{}{},

    // ...省略 (github.com/po3rin/gonbayes/blob/master/const.go を参照)
}

func isStopWord(word string) bool {
	_, ok := stopWords[word]
	return ok
}
//}

stop wordsにはPythonで提供されているNLTK(Natural Language Tool Kit）で使われているstop wordsのリスト@<fn>{swlidt}を参照しました。
//footnote[swlidt][NLTKのStop Wrodsの一覧。https://gist.github.com/sebleier/554280]

stop wordsのマップはkeyにstop wordを置き、valueに空の構造体を置きます。Goにおいて空の構造体はメモリを消費しません。その為マップが存在確認のためだけに存在する場合は空の構造体が有用です。初耳の方は実際に@<code>{unsafe.Sizeof}を使って0になることを確認しましょう。

//list[sizezero][空の構造体][go]{
fmt.Println(unsafe.Sizeof(s))
//}

これで任意の単語がstop wordsかどうかを検知する関数ができました。テストで動作を確認してみましょう。関数が非公開なので@tenntennさんの記事である「Go Fridayこぼれ話：非公開（unexported）な機能を使ったテスト #golang」@<fn>{export_test}を参考にテストを書いていきます。まず非公開関数をテストするために@<code>{export_test.go}を作成して関数をテストでのみ公開できるようにしておきます。

//footnote[export_test][@<href>{https://tech.mercari.com/entry/2018/08/08/080000}]

//list[export_test_sw][isStopWordの公開][go]{
// ./export_test.go

package gonbayes

var IsStopWord = isStopWord

//}

これでテスト時にだけ@<code>{IsStopWord}がビルドされるようになりました。早速テストを書いていきましょう。

//list[test_isStopWord][isStopWordのテスト][go]{
// ./utils_test.go

package gonbayes_test

import (
	"testing"

	"github.com/po3rin/gonbayes"
)

func TestIsStopWord(t *testing.T) {
	tests := []struct {
		name, word string
		want bool
	}{
		{
			name: "stop word",
			word: "you",
			want: true,
		},
		{
			name: "not stop word",
			word: "great",
			want: false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := gonbayes.IsStopWord(tt.word)
			if got != tt.want {
				t.Errorf("want = %v, got = %v\n", tt.want, got)
			}
		})
	}
}
//}

@<code>{you}はstop wordなので@<code>{true}が返り、@<code>{great}はstop wordではないので@<code>{false}が返ることが期待されます。実行するとテストが通るはずです。テストの書き方に関しては@<em>{@budougumi0617}さんのブログ@<fn>{how_to_test}が体系的にまとまっていて勉強になるので@<list>{test_isStopWord}でテストの書き方の知識が怪しいなと思った人は読んでみることをお勧めします。

//footnote[how_to_test][@<href>{https://budougumi0617.github.io/2018/08/19/go-testing2018/#sub-test-testing-t-run}]

=== 単語のステミング処理

swimという単語は「swims」「swimming」「swimmer」などの複数の語形があります。単語の語形が変化する言語では前処理としてステミング処理を行う必要があります。英語で使えるステミングアルゴリズムは、「Lancaster」「Porter」「Snowball Stemmers」などがあります。Goのパッケージとして提供されているステミングライブラリで、ある程度starが付いているパッケージを調べたところ@<table>{stem_pkgs}が候補です。

//table[stem_pkgs][Goで実装された主要なステミングライブラリ]{
パッケージ名	説明
------------------------------------
github.com/reiver/go-porterstemmer	Goで実装された「Porter Stemming」
github.com/agonopol/go-stem	こちらもGoで実装された「Porter Stemming」
github.com/kljensen/snowball	Goで実装された「Snowball Stemming」
//}

たまに@<code>{kljensen/snowball}が更新されるくらいで、活発に開発されているパッケージは見つからなかったです(そもそもこういうアルゴリズムの実装のパッケージがバリバリ更新されていくことの方が珍しい）。今回は英語以外の言語にも対応している@<code>{kljensen/snowball}を採用しましょう。Goで任意の文字列をステミングする関数を実装します。

//list[stem][ステミング処理の実装][go]{
// ./utils.go

package gonbayes

import (
	// ...

	"github.com/kljensen/snowball"
)

func stem(word string) string {
	stemmed, err := snowball.Stem(word, "english", true)
	if err != nil {
		// ignore error
		fmt.Printf("cannot stem word: %s\n", word)
		return word
	}
	return stemmed
}
//}

@<code>{snowball.Stem}関数は未知の単語がきた時などにエラーを返しますが、今回はそのまま単語を返し、エラーを無視する実装にしています。いくつかの単語でテストしてみましょう。@<list>{export_test_sw}と同じようにテストしたい関数をテスト中だけ公開できるようにした後に、ステミングのテストを書いてみます。

//list[test_stem][stemのテスト][go]{
// ./utils_test.go

// ...

func TestStem(t *testing.T) {
	tests := []struct {
		name, word, want string
	}{
		{
			name: "stemming swin",
			word: "swims",
			want: "swim",
		},
		{
			name: "stemminh ceiling",
			word: "ceiling",
			want: "ceil",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := gonbayes.Stem(tt.word)
			if got != tt.want {
				t.Errorf("want = %v, got = %v\n", tt.want, got)
			}
		})
	}
}
//}

このテストで単語のステミングができていることが確認できます。

=== テキストクリーニング

「You are the best friend!!!!!(^^）」という文字列がある場合、顔文字や「！」は邪魔なのでテキストクリーニングをしておきます。つまり先ほどの文字列を「you are the best friend」という文字列に変換します。

//list[clean][テキストクリーニングの実装][go]{
// ./utils.go

var re = regexp.MustCompile("[^a-zA-Z 0-9]+")

func clean(document string) string {
	document = strings.ToLower(document)
	return re.ReplaceAllString(document, "")
}
//}

@<list>{clean}では大文字を小文字に変換した上で正規表現でアルファベットと数字以外の文字を除外しています。Goにおける正規表現は標準パッケージの@<code>{regexp}を使っています。@<code>{Regexp}は初期化コストが高いのでグローバルに定義して、関数が呼ばれるたびに初期化するということがないようにしています。それでは@<list>{export_test_sw}と同じように関数を公開した後に、テストで動作を確認してみます。

//footnote[regexp][@<href>{https://golang.org/pkg/regexp/}]

//list[clean_test][テキストクリーニングのテスト][go]{
// ./utils_test.go

func TestClean(t *testing.T) {
	tests := []struct {
		name, word ,want string
	}{
		{
			name: "clean",
			word: "You are the best friend!!!!!(^^）",
			want: "you are the best friend",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := gonbayes.Clean(tt.word)
			if got != tt.want {
				t.Errorf("want = %v, got = %v\n", tt.want, got)
			}
		})
	}
}
//}

テキストクリーニングができているのが確認できます。ここでは紹介しませんがHTMLタグの削除なども実装していくと、クローリングしてきた記事などをNaive Bayesで処理できるようになります。

===[column] Go の正規表現は本当に遅いの？

Goの標準パッケージでは@<em>{Thompson NFA}という正規表現の実装手法が採用されています。Russ Coxさんが「Regular Expression Matching Can Be Simple And Fast 」というブログ@<fn>{regexp_blog}で@<em>{Thompson NFA}を解説しています。ブログから引用した@<img>{Regex}をみてみましょう。@<m>{a^n}に対する正規表現@<m>{a?^n a^n}のケースでのパフォーマンスです。@<em>{Thompson NFA}ではパフォーマンスが線形的に落ちていくのに対し、Perlで採用されている手法では最初は高速ですが、@<m>{n}がある程度大きくなると急激にパフォーマンスが落ちます。よってGoの正規表現が絶対的に遅いというわけではなく、場合によってはGoの正規表現の方が高速に動く可能性もあります。「Goの正規表現は遅い」ということを鵜呑みにせずに実際にBenchmarkをとって、問題になるパフォーマンスなのかどうかを確認することをオススメします。@<em>{@momotaro98}さんが日本語でまとめてくれているのでこちら@<fn>{regexp_blog_ja}も是非読んでみてください。

//image[Regex][入力文字に対する正規表現手法のパフォーマンス][scale=1]{
//}
===[/column]

//footnote[regexp_blog][@<href>{https://swtch.com/~rsc/regexp/regexp1.html}]
//footnote[regexp_blog_ja][@<href>{https://qiita.com/momotaro98/items/09d0f968d44c7027450d}]

=== 単語数のカウント

確率を計算する際に便利なように単語数をカウントする関数も実装しておきます。その際に今まで作ってきた関数を使います。

//list[count][テキストクリーニングした上で単語数のカウント][go]{
// ./utils.go

func countWords(document string) (wordCount map[string]int) {
	document = clean(document)
	words := strings.Split(document, " ")
	wordCount = make(map[string]int)
	for _, word := range words {
		if !isStopWord(word) {
			key := stem(strings.ToLower(word))
			wordCount[key]++
		}
	}
	return
}
//}

これでドキュメントに単語がそれぞれ何個あったかを返す関数ができました。@<list>{export_test_sw}と同じようにテストしたい関数を公開した後にテストを書いて動作を確認しましょう。

//list[count_test][countWordsのexport][go]{
// ./utils_test.go

func TestCountWords(t *testing.T) {
	tests := []struct {
		name, word string
		want map[string]int
	}{
		{
			name: "count words",
			word: "I say hello. You say hi",
			want: map[string]int{
				"say":   2,
				"hello": 1,
				"hi":    1,
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := gonbayes.CountWords(tt.word)
			if !reflect.DeepEqual(got, tt.want) {
				t.Errorf("want = %v, got = %v\n", tt.want, got)
			}
		})
	}
}
//}

== Goによる Naive Bayes Classifier の実装

前処理を行う関数ができたので、いよいよNaive Bayesを実装します。設計としては@<code>{Classifier}構造体に学習用のメソッドと分類用のメソッドを生やします。

//list[classifier][学習用のメソッドと分類用のメソッド][go]{
// ./gonbayes.go

package gonbayes

// ...

// Classifier is a classifier for document categories.
type Classifier struct {
	// TODO: implement
}

// Tratrains documents classifier.
func (c *Classifier) Train(category string, document string) {
	// TODO: implement
}

// Classify classifies documents.
func (c *Classifier) Classify(document string) (category string) {
	// TODO: implement
}
//}

この章では主に@<list>{classifier}の中身をそれぞれ実装していきます。

=== Classifier構造体を実装する

@<code>{Classifier}には確率計算で使うデータを格納するためのフィールドを定義しておきます。@<code>{Classifier}にはどんなフィールドが必要でしょうか？@<m>{P(C）}を計算するために各カテゴリに何個ドキュメントが格納されているかのデータ(@<code>{TotalDocsInCategories})とドキュメントの総数(@<code>{TotalDocs})の２つは必要です。@<m>{P(D|C）}ではカテゴリそれぞれの各単語の数(@<code>{Words})に加えて、全単語の総数(@<code>{TotalWords})、カテゴリごとに単語が何個あるかのデータ(@<code>{TotalWordsInCategories})も必要です。早速@<code>{Classifier}と、ついでに@<code>{Classifier}を初期化する関数を実装しましょう。

//list[classifier_init][Classifier の実装][go]{
// ./gonbayes.go

// Classifier is documents categories clasifier.
type Classifier struct {
	Words             map[string]map[string]uint64
	TotalWords        uint64
	TotalDocsInCategories  map[string]uint64
	TotalDocs         uint64
	TotalWordsInCategories map[string]uint64
}

// NewClassifier initializes classifier.
func NewClassifier(categories []string) *Classifier {
	c := &Classifier{
		Words:             make(map[string]map[string]uint64),
		TotalDocsInCategories:  make(map[string]uint64),
		TotalWordsInCategories: make(map[string]uint64),
	}

	for _, category := range categories {
		c.Words[category] = make(map[string]uint64)
	}
	return c
}
//}

Goにおいて@<code>{map}のゼロ値は@<code>{nil}なので明示的に@<code>{make}などで初期化しておく必要があります。@<code>{Clasifier}の用途がある程度想定されるならmapの初期化にcapsを設定しておくとよいかもしれません。@<code>{NewClassifier}の第一引数ではドキュメントをどのように分類するかのカテゴリの一覧を受けます。@<list>{classifier_init}では分類したいカテゴリの数だけマップを初期化しています。

=== データセットからの学習を実装する

続いてClassifierを学習させるための関数@<code>{Classifier.Train}を実装しましょう。

//list[train][Classifierを学習させるための関数][go]{
// ./gonbayes.go

// Tratrains documents classifier.
func (c *Classifier) Train(category string, document string) {
	for word, count := range countWords(document) {
		c.Words[category][word] += uint64(count)
		c.TotalWordsInCategories[category] += uint64(count)
		c.TotalWords += uint64(count)
	}
	c.TotalDocsInCategories[category]++
	c.TotalDocs++
}
//}

@<list>{train}では与えられた学習用データから@<code>{Classifier}のフィールドを更新していくだけです。分類ではこのフィールドに入った値を使って確率を計算していきます。

=== ベイズ確率計算を実装する

それではNaive Bayesの根幹である確率計算を実装しましょう。

//list[pcd][各確率を計算するメソッド][go]{
// ./gonbayes.go

// P(C）の計算
func (c *Classifier) pCategory(category string) float64 {
	return float64(c.TotalDocsInCategories[category]) / float64(c.TotalDocs)
}

// P(D|C）の計算
func (c *Classifier) pDocCategory(category string, document string) float64 {
	p := 1.0
	for word := range countWords(document) {
		p *= c.pWordCategory(category, word)
	}
	return p
}

// P(w|C）の計算 (P(D|C）の計算のため)
func (c *Classifier) pWordCategory(category string, word string) float64 {
	// 本誌からはみ出るので分子と分母で分けました。
	d := float64(c.Words[category][stem(word)])
	n := float64(c.TotalWordsInCategories[category])
	return d / n
}

// P(C|D）の計算 (最終的に欲しい値)
func (c *Classifier) pCategoryDocument(category string, document string) float64 {
	return c.pDocCategory(category, document) * c.pCategory(category)
}
//}

@<list>{pcd}では@<m>{P(C）}と@<m>{P(D|C）}を計算する関数を実装し、それを使って　@<m>{P(C|D）}を計算しています(@<m>{P(C|D）= P(D|C）P(C）}であることを思い出しましょう)。@<m>{P(D|C）}はカテゴリー内の各単語の出現確率(@<m>{P(w|C）})の掛け算でした。

次に @<code>{Classifier.pCategoryDocument} を使って、ドキュメントに対してそれぞれのカテゴリに分類される確率を１つの@<code>{map}にまとめる関数を作っておきましょう。

//list[p][ドキュメントに対してそれぞれのカテゴリに分類される確率を1つのmapにまとめる関数][go]{
// ./gonbayes.go

// P is Probabilities of each categories.
func (c *Classifier) P(document string) map[string]float64 {
	p := make(map[string]float64)
	for category := range c.Words {
		p[category] = c.pCategoryDocument(category, document)
	}
	return p
}
//}

早速、テストをしてみましょう。最初に示した具体例を利用します。

//list[p_test][各確率を計算するメソッドのテスト][go]{
// ./gonbayes_test.go

func TestP(t *testing.T) {
	tests := []struct {
		name       string
		categories []string
		dataset    map[string]string
		document   string
		want       map[string]float64
	}{
		{
			name:       "example",
			categories: []string{"経済", "IT", "エンタメ"},
			dataset: map[string]string{
				"Price":            "経済",
				"Insurance":        "経済",
				"Python and Price": "IT",
				"Python and Go":    "IT",
				"Marvel":           "エンタメ",
			},
			document: "Python and Go",
			want:     map[string]float64{
				"IT": 0.05,
				"エンタメ": 0,
				"経済": 0,
			},
		},
	}

	for _, tt := range tests {
		tt := tt
		t.Run(tt.name, func(t *testing.T) {
			t.Parallel()
			c := gonbayes.NewClassifier(tt.categories)
			for s, v := range tt.dataset {
				c.Train(v, s)
			}
			got := c.P(tt.document)
			if !reflect.DeepEqual(got, tt.want) {
				t.Errorf("want = %v, got = %v\n", tt.want, got)
			}
		})
	}
}
//}

@<list>{p_test}の結果と最初に示したベイズ理論の適用の具体例(@<table>{pdc})と結果が一致することを確認してください。

=== Naive Bayesを使って記事分類を実装する

これで任意のドキュメントが与えられた時に、それぞれのカテゴリに何%の確率で分類されるのかを@<code>{map}として返す関数ができました。最後にドキュメントに対してどのカテゴリに分類されるかを返す関数@<code>{Classify}を実装しましょう。

//list[classify][ドキュメントに対してどのカテゴリに分類されるかを返す関数][go]{
// ./gonbayes.go

// Classify classify documents.
func (c *Classifier) Classify(document string) string {
	// 各カテゴリごとの P(C|D)を計算
	prob := c.P(document)

	// P(C|D)の値でソート
	type pm struct {
		category    string
		probability float64
	}
	var ps []pm
	for c, p := range prob {
		ps = append(ps, pm{c, p})
	}
	sort.Slice(ps, func(i, j int) bool {
		return ps[i].probability > ps[j].probability
	})

	// 最も確率の高いカテゴリを返す
	return ps[0].category
}
//}

おめでとうございます！これでNaive Bayesを使った分類器が実装できました！

== 口コミの感情分析をやってみよう

早速、実装した分類器を使って口コミの感情分析(ネガティブorポジティブ）判定をやってみましょう。@<em>{"I am happy"}というドキュメントはポジティブ、@<em>{"It's out of my mind"}というドキュメントはネガティブというように口コミを分類することがゴールです。

=== データセットの読み込み

学習に使うデータセットは「Sentiment Labelled Sentences Data Set」です。このデータセットはドキュメントとネガティブ/ポジティブ(0/1）がセットになっているテキストファイルです(@<list>{slsds})。サイト@<fn>{slsds}から「Data Folder」>「sentiment labelled sentences.zip」でzipファイルをダウンロードしておきましょう。

//footnote[slsds][@<href>{https://archive.ics.uci.edu/ml/datasets/Sentiment+Labelled+Sentences}]

//list[slsds][Sentiment Labelled Sentences Data Set][txt]{
Wow... Loved this place.	1
Crust is not good.	0
Not tasty and the texture was just nasty.	0
The selection on the menu was great and so were the prices.	1
Now I am getting angry and I want my damn pho.	0
Honeslty it didn't taste THAT fresh.)	0

...(省略)

//}

続いて @<code>{cmd/negaposi/main.go}を作り、感情分析を実行するmain関数を作りましょう。まずは「Sentiment Labelled Sentences Data Set」を読み込んでGoで使える形にして返す関数を作りましょう。

//list[slsds_parse][データセットの読み込み][go]{
// ./cmd/negaposi/main.go

package main

// ...

const (
	posiLabel = "positive"
	negaLabel = "negative"
)

func loadDataset(file string) (map[string]string, error) {
	f, err := os.Open(file)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	dataset := make(map[string]string)
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		l := scanner.Text()
		data := strings.Split(l, "\t")
		if len(data) != 2 {
			continue
		}
		s := data[0]
		if data[1] == "0" {
			dataset[s] = negaLabel
		} else if data[1] == "1" {
			dataset[s] = posiLabel
		}
	}

	if err := scanner.Err(); err != nil {
		return nil, err
	}
	return dataset, nil
}
//}

この関数は標準パッケージの機能である@<code>{func (*Scanner) Scan}@<fn>{scan}を使って1行ずつ読み込んでラベルとドキュメントを@<code>{map}に格納していきます。Go初心者の方は1行ずつ読み込む鉄板の処理なので覚えておきましょう。

=== いざ感情分析!!

いよいよ口コミの感情分析を行う@<code>{main}関数を実装します。

//footnote[scan][@<href>{https://golang.org/pkg/bufio/#Scanner.Scan}]

//list[main][感情分析を実行するmainパッケージ][go]{
// ./cmd/negaposi/main.go

func main() {
	s := flag.String("s", "", "input string")
	f := flag.String("f", "./yelp_labelled.txt", "dataset file path")
	flag.Parse()

	// Classifier の初期化
	class := []string{posiLabel, negaLabel}
	classifier := gonbayes.NewClassifier(class)

	// 「Sentiment Labelled Sentences Data Set」の読み込み
	dataset, err := loadDataset(*f)
	if err != nil {
		log.Fatal(err)
	}

	// 学習
	for s, v := range dataset {
		classifier.Train(v, s)
	}

	// 分類
	result := classifier.Classify(*s)
	fmt.Println(result)
}
//}

早速実行してみましょう。

//list[exec_nega][口コミの感情分析の実行 1][go]{
$ go run cmd/negaposi/main.go -s "I hate this" -f "./yelp_labelled.txt"
negative
//}

確かに"@<em>{"I hate this"}はネガティブな口コミです。それでは @<em>{"this is wonderful"}ではどうでしょうか。

//list[exec][口コミの感情分析の実行 2][go]{
$ go run cmd/negaposi/main.go -s "this is wonderful" -f "./yelp_labelled.txt"
positive
//}

確かに口コミの感情分析ができています！まだ改良の余地はありますが、これでNaive Bayesを使ったテキスト分類が成功していることがわかります。

== 学習済み分類器を gob パッケージで保存する

分類を実行する度に同じデータセットから同じ学習を行って分類器を生成するのは無駄だと気づいたかもしれません。学習済みの@<code>{Classifier}をいつでも呼び出せるように修正しましょう。その為には標準パッケージである@<code>{encoding/gob}@<fn>{gob}を利用します。Goではデータ構造のシリアライズのために標準でいくつかのパッケージが用意されていますがGo内でデータ構造を使うのであればXMLやJSONなどではなくバイナリエンコーディングした形で保存して、読み込み時などで効率性を上げたいところです。そのような目的で@<code>{encoding/gob}が誕生しています。データ構造さえ自明であればそのままEncodeできるので使用感としてもとてもシンプルに収まります。早速　@<code>{Classifier}に新しいメソッドを定義して学習済みの@<code>{Classifier}をEncode & Decodeできるようにしておきましょう。

//footnote[gob][@<href>{https://golang.org/pkg/encoding/gob/}]

//list[gob][gobを使った学習済みClassifierの Encode & Decode][go]{
// ./gonbayes.go

// Encode trained Classifier
func (c *Classifier) Encode(fileName string) error {
	if c.TotalDocs == 0 {
		return errors.New("gonbayes: classifier is not trained yet")
	}
	f, err := os.Create(fileName)
	if err != nil {
		return err
	}
	defer f.Close()

	err = gob.NewEncoder(f).Encode(&c)
	if err != nil {
		return err
	}
	return nil
}

// Decode CBOW output file to struct.
func (c *Classifier) Decode(fileName string) error {
	f, err := os.Open(fileName)
	if err != nil {
		return err
	}
	defer f.Close()

	err = gob.NewDecoder(f).Decode(c)
	if err != nil {
		return err
	}
	return nil
}
//}

@<list>{gob}ではシンプルなメソッドとしてファイル名さえ指定すればEncode & Decodeできるように設計します。たとえば@<code>{Encode}を呼べば指定ファイルへ学習済みの@<code>{Clasifier}をバイナリエンコーディングした形でファイルに保存できます。これを使って@<list>{main}を改良しましょう。

//list[main_with_gob][gobパッケージを使ったmainの実装][go]{
// ./cmd/negaposi/main.go

func main() {
	s := flag.String("s", "", "input string")
	f := flag.String("f", "./yelp_labelled.txt", "dataset file path")
	c := flag.String("c", "", "trained model file path")
	o := flag.String("o", "classifier.gob", "trained file name")
	flag.Parse()

	class := []string{posiLabel, negaLabel}

	classifier := gonb.NewClassifier(class)

	if *c != "" {
		// 学習済みモデルのファイルパスを指定した場合
		// それをデコードして Classifier を生成する。
		err := classifier.Decode(*c)
		if err != nil {
			log.Fatal(err)
		}
	} else {
		// 新しく学習して Classifirerを生成する。
		dataset, err := loadDataset(*f)
		if err != nil {
			log.Fatal(err)
		}

		for s, v := range dataset {
			classifier.Train(v, s)
		}
	}

	result := classifier.Classify(*s)
	fmt.Println(result)

	// 学習済み Classifier を バイナリエンコーディング
	if *c == "" {
		classifier.Encode(*o)
	}
}
//}

@<list>{main_with_gob}ではFlagを増やして学習済みのClassifierをEncodeしたファイルを指定できるようにしておきます。指定ファイルがあればそれをデコードして学習済み@<code>{Classifier}として使い、指定がなければ新しく学習します。

//list[exec_f][学習済み Classifer を使う][go]{
# 新しく学習して Classifier をバイナリエンコード
$ go run cmd/negaposi/main.go -s "this is wonderful" -f "./yelp_labelled.txt"

# 学習済みの Classifier を利用した分類
$ go run cmd/negaposi/main.go -s "this is wonderful" -c "classifier.gob"
//}

これで毎回学習を走らせなくても、感情分析ができるようになりました。もちろん@<code>{encoding/gob}はNaive Bayesのときだけでなく、ニューラルネットワークの実装で学習済みモデルを保存しておく用途でも使えます。一方でGoで学習したモデルをGo以外の言語の言語で使いたい場合はJSONなどにEncodeして利用してしまうのも一手です。

== ベイズ分類器の改善

実はここまでの実装ではいくつかの問題があります。１つずつ問題をみていきましょう。

=== ゼロ頻度問題

@<m>{P(D|C）}の計算を思い出しましょう。@<m>{P(D|C）}は各単語の確率の掛け算で表せました。しかし学習用データには存在しないワードがあった場合、つまり、１つでも確率0の単語があった場合、カテゴリーの確率が0になってしまいます。これを解決する為にさまざまな手法が提案されています。今回はその中の１つを紹介します。

==== Additive smoothings

@<em>{スムージング}という手法を使ってゼロ頻度問題を解決します。@<em>{Additive smoothings}はもっとも単純なスムージングの1つです。実際よりもわずかに全単語の出現率を上げて計算します。すべてのカウントに係数@<m>{δ}を追加します。通常は@<m>{0<δ≤1}です。基本的には@<m>{δ=1}として計算します。分子に1加え、分母に全単語数を足します。

//list[add][Additive smoothings の実装][go]{
// ./gonbayes.go

func (c *Classifier) pWordCategory(category string, word string) float64 {
	// 本誌からはみ出るので分子と分母で分けました。
	n := float64(c.Words[category][stem(word)]+1)
	d := float64(c.TotalWordsInCategories[category]+c.TotalWords)
	return n / d
}
//}

他にもさまざまなスムージングがあるので調べてみてください。「An empirical study of smoothing techniques for
language modeling」@<fn>{sm} というペーパーが非常に勉強になります。

//footnote[sm][@<href>{http://u.cs.biu.ac.il/~yogo/courses/mt2013/papers/chen-goodman-99.pdf}]

=== アンダーフロー

学習データが大きくなるとアンダーフローを起こしてしまう可能性があります。アンダーフローとは分母が大きくなりすぎて計算結果の指数部が小さくなり過ぎ、使用している記述方式で数値が表現できなくなることです。アンダーフローの対策として対数をとることが考えられます。@<m>{log(P(D|C）P(C））}は対数の公式@<m>{log(A・B）= logA + logB}から@<m>{log(P(D|C））+ log(P(C））}になります。さらに@<m>{log(P(D|C））}は各単語の確率の掛け算だったのでここも足し算に変換できます。最終的に@<img>{Log}ような式に変換できます。

//image[Log][対数をとってオーバーフロー対策][scale=0.5]{
//}

Goのコードをアンダーフロー対応バージョンに変えていきましょう。

//list[plog][対数をとってオーバーフロー対策した確率計算][go]{
func (c *Classifier) pCategory(category string) float64 {
	// 本誌からはみ出るので分子と分母で分けました。
	n := float64(c.TotalDocsInCategories[category])
	d := float64(c.TotalDocs)

	// 対数をとる
	return math.Log(n / d)
}

func (c *Classifier) pDocCategory(category string, document string) float64 {
	// ゼロ値で初期化
	var p float64
	for word := range countWords(document) {
		// 対数をとって足していく処理に変更
		p += math.Log(c.pWordCategory(category, word))
	}
	return p
}

func (c *Classifier) pCategoryDocument(category string, document string) float64 {
	// 足し算に変更
	return c.pDocCategory(category, document) + c.pCategory(category)
}
//}

これで大きな学習データが来てもアンダーフローを起こさずに計算ができます。

#@# absolute discounting
#@# additive smoothing
#@# backoff

== おわりに

本章では Go とベイズ理論を使ったシンプルな記事分類の実装方法を紹介しました。是非みなさんもGoで自然言語処理に挑戦してみてください！
#@# textlint-enable