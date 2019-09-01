= Goとベイズ理論でゼロからシンプルな記事分類を実装してみよう！

== はじめに

白ヤギコーポレーションでバックエンドエンジニアをしている@po3rin@<fn>{po3rin}です。仕事では記事レコメンドAPIをGoで実装しています。今回はGoとベイズ理論を使ったシンプルな記事分類の実装方法を紹介します。Naive Bayesの理論を簡単に学べるのはもちろんですが、Goでのテキスト前処理の方法や、学習済み分類器の保存方法なども含めてご紹介するので、今後Goで自然言語処理をする際に役立つかもしれません。今回実装するコードは@<code>{github.com/po3rin/gonbayes}@<fn>{gonbayes}というリポジトリ名でGitHubに公開しているので参考にしてください。

#@# TODO: motivate

//footnote[po3rin][@<href>{https://twitter.com/po3rin}]
//footnote[gonbayes][@<href>{https://github.com/po3rin/gonbayes}]

== ベイズの理論とNaive Bayes

この節ではNaive Bayesの理論をザックリと理解することがゴールです。実装に困らない程度に「なんとなくやっていることが分かる」程度で大丈夫です。

=== さらっとベイズの理論を抑えよう

ベイズ理論(@<img>{Bayes})とは「条件付き確率に関して成り立つ定理」のことです。

//image[Bayes][ベイズの理論][scale=0.7]{
//}

一見すると難しそうに見えますが、１つずつ式の意味を見ていくとそれほ程難しくはないです (@<table>{p})。

//table[p][確率の表記]{
式	意味
------------------------------------
P(A)	Aが起こる確率
P(B|A)	Aが起こった時にBが起こる確率 (事後確率と呼ぶ)
//}

記事のカテゴリ分類を例にベイズの定理の使い方を見ていきましょう。知りたいのは「ドキュメントが与えられた時に、とあるカテゴリに分類される確率」です。これは@<table>{p}を見返すと@<m>{P(C|D）}と表せることがわかります(@<code>{C} はCategory、@<code>{D}はDocumentを表しています)。ベイズの理論より @<m>{P(C|D）}は@<m>{P(C）}カテゴリーの出現率と@<m>{P(D|C）}（カテゴリーが与えられた時のドキュメント出現率）の掛け算です(@<m>{P(D）}（カテゴリー内のドキュメント出現率）はすべてのカテゴリで同じであるため無視できます）。

=== 具体例からNaive Bayesの具体的な計算方法を理解する

理論だけだと何が行われているのか分からないと思うので具体的な例で理解しましょう。5つの文章@<table>{documents}がある場合を考えます。

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

@<m>{P(C）}はカテゴリーの出現率です。たとえば「経済カテゴリ」の出現率は@<code>{経済カテゴリの数/カテゴリ総数(重複分もカウント)}なので2/5になります。

//table[pc][P(C）の例]{
カテゴリ	P(C）
------------------------------------
経済	2/5
IT	2/5
エンタメ	1/5
//}

==== P(D|C）

@<m>{P(D|C）}はカテゴリーが与えられた時のドキュメント出現率でした。これは計算が難しいように思えます。一方でカテゴリ内の単語出現率(@<m>{P(W|C）}）は簡単です。ITカテゴリには単語が4つあるので、たとえば「Python」が「ITカテゴリ」に出現する確率は2/4、「Price」が「ITカテゴリ」に出現する確率は1/4です。

//table[pwc][P(W|C）の例]{
.	経済	IT	エンタメ
------------------------------------
Price	1/2	1/4	0/1
Insurance	1/2	0/4	0/1
Python	0/2	2/4	0/1
Go	0/2	1/4	0/1
Marvel	0/2	0/4	1/1
//}

この数字を使って欲しい@<m>{P(D|C）}を計算する簡単な方法があります。@<m>{P(D|C）}を各単語の確率の掛け算で計算する方法です。たとえばITカテゴリが@<code>{"Python and Go"}というドキュメントに分類される確率@<m>{P(D|C）}は(「Python」が「ITカテゴリ」に出現する確率2/4）*(「Go」が「ITカテゴリ」に出現する確率1/4）で1/8になります(@<code>{and}などのドキュメントに頻出する単語は @<code>{stop words}と呼ばれ、Naive Bayesでは無視します。次の節で詳しく説明します）。

//table[pdc][P(D|C）の例]{
.	ITカテゴリがドキュメントが分類される確率 P(D|C）
------------------------------------
Python and Go	2/4 * 1/4 = 1/8
//}

実はこの計算では本来あるべき単語の条件付き確率を無視し、各単語が互いに独立していると仮定してます。つまり単語がドキュメント内にランダムに現れると仮定しているのです。この過程がNaive BayesがNaiveたる所以です。ここまでこれば欲しかった@<m>{P(C|D）}を計算できます。復習ですが@<m>{P(C|D）}は@<m>{P(C）}と@<m>{P(D|C）}の掛け算です

//table[pcd][P(C|D）の例]{
.	ドキュメントがITカテゴリに分類される確率 {P(C|D）= P(D|C）*P(C）
------------------------------------
Python and Go	1/8 * 2/5 = 1/20
//}

これで@<m>{Python and Go}というドキュメントが@<code>{ITカテゴリ}に含まれる確率は@<code>{0.05}という結果が計算できました。分類においては分類したいドキュメントに対してそれぞれのカテゴリで@<m>{P(C|D）}を計算し、もっとも高い確率の物を分類の結果として採択します。

== Goによるテキストの前処理

実際にNaive Bayesを実装する前に入力されるテキストを前処理する必要があります。今回は英語を対象にするので、英語のテキストを前処理する必要があります。本章のコードではGitHubのサンプルパッケージ名と同じ「gonbayes」パッケージとして実装していきますが好きなパッケージ名で構いません。

=== stop words の削除

「the」や「it」などの高頻度で出現する単語はその文章の特徴を表す単語とはいえない為、前処理で削除しておく必要があります。このような高頻度で出現する単語を自然言語処理界隈でstop wordsを呼ばれています。ではGoでstop wordを検知しましょう。

//list[stopwords][stop wordの検知][go]{
package gonbayes

var stopWords = map[string]struct{}{
	"i":          struct{}{},
	"me":         struct{}{},
	"my":         struct{}{},
	"myself":     struct{}{},

    // 省略 (github.com/po3rin/gonbayes/blob/master/const.go を参照)
}

func isStopWord(word string) bool {
	_, ok := stopWords[word]
	return ok
}
//}

stop wordsにはPythonで提供されているNLTK(Natural Language Tool Kit）で使われているstop wordsのリスト@<fn>{swlidt}から拝借しました。
//footnote[swlidt][NLTKのStop Wrodsの一覧。https://gist.github.com/sebleier/554280]

stop wordsのマップはkeyにstop wordを置き、valueに空の構造体を置きます。Goにおいて空の構造体はメモリを消費しません。その為マップが存在確認のためだけに存在する場合は空の構造体が有用です。初耳の方は実際に@<code>{unsafe.Sizeof}を使って0になることを確認しましょう。

//list[sizezero][空の構造体][go]{
fmt.Println(unsafe.Sizeof(s))
//}

これで任意の単語がstop wordsかどうかを検知する関数ができました。テストで動作を確認してみましょう。関数が非公開なので@tenntennさんの記事である「Go Fridayこぼれ話：非公開（unexported）な機能を使ったテスト #golang」@<fn>{export_test}を参考にテストを書いていきます。まず非公開関数をテストするために@<code>{export_test.go}を作成して関数をテストでのみ公開できるようにしておきます。

//footnote[export_test][@<href>{https://tech.mercari.com/entry/2018/08/08/080000}]

//list[export_test_sw][isStopWordの公開][go]{
package gonbayes

var IsStopWord = isStopWord

//}

これでテスト時にだけ@<code>{IsStopWord}がビルドされるようになりました。早速テストを書いていきましょう。

//list[test_isStopWord][isStopWordのテスト][go]{
package gonbayes_test

import (
	"testing"

	"github.com/po3rin/gonbayes"
)

func TestIsStopWord(t *testing.T) {
	tests := []struct {
		name string
		word string
		want bool
	}{
		{
			name: "stop word",
			word: "you",
			want: true,
		},
		{
			name: "stop word",
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

@<code>{you}はstop wordなので@<code>{true}が返り、@<code>{great}はstop wordではないなので@<code>{false}が返ることが期待されます。実行するとテストが通るはずです。テストの書き方に関しては@budougumi0617さんのブログ@<fn>{how_to_test}が体系的にまとまっていて勉強になるので@<list>{test_isStopWord}でテストの書き方の知識が怪しいなと思った人は読んでみることをお勧めします。

//footnote[how_to_test][@<href>{https://budougumi0617.github.io/2018/08/19/go-testing2018/#sub-test-testing-t-run}]

=== 単語のステミング処理

swimという単語は「swims」「swimming」「swimmer」などの複数の語形があります。単語の語形が変化する言語では前処理としてステミング処理を行う必要があります。英語で使えるステミングアルゴリズムは、「Lancaster」「Porter」「Snowball Stemmers」などがあります。Goのパッケージとして提供されているステミングライブラリである程度starが付いているパッケージを調べたところ@<table>{stem_pkgs}が候補。

//table[stem_pkgs][ドキュメントの実際の例]{
パッケージ名	説明
------------------------------------
reiver/go-porterstemmer	Goで実装された「Porter Stemming」
agonopol/go-stem	こちらもGoで実装された「Porter Stemming」
kljensen/snowball	Goで実装された「Snowball Stemming」
//}

たまに@<code>{kljensen/snowball}が更新されるくらいで、活発に開発されているパッケージは見つからなかったです(そもそもこういうアルゴリズムの実装のパッケージがバリバリ更新されていくことの方が珍しい）。今回は英語以外の言語にも対応している@<code>{kljensen/snowball}を採用しましょう。Goで任意のstring をステミングする関数を実装します。

//list[stem][ステミング処理の実装][go]{
package gonbayes

import (
	// ...

	"github.com/kljensen/snowball"
)

func stem(word string) string {
	stemmed, err := snowball.Stem(word, "english", true)
	if err != nil {
		// ignore error
		fmt.Println("Cannot stem word:", word)
		return word
	}
	return stemmed
}
//}

@<code>{snowball.Stem}関数は未知の単語がきた時などにエラーを返しますが、今回はそのまま単語を返し、エラーを無視する実装にしています。いくつかの単語でテストしてみましょう。@<list>{export_test_sw}と同じようにテストしたい関数をテスト中だけ公開できるようにした後に、ステミングのテストを書いてみます。

//list[test_stem][stemのテスト][go]{

// ...

func TestStem(t *testing.T) {
	tests := []struct {
		name string
		word string
		want string
	}{
		{
			name: "stop word",
			word: "swims",
			want: "swim",
		},
		{
			name: "stop word",
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

「You are Best friend!!!!!(^^）」という文字列がある場合、顔文字や「！」は邪魔なのでテキストクリーニングをしておきます。つまり先ほどの文字列を「you are best friend」というテキストに変換します。

//list[clean][テキストクリーニングの実装][go]{
func clean(document string) string {
	document = strings.ToLower(document)
	return regexp.MustCompile("[^a-zA-Z 0-9]+").ReplaceAllString(document, "")
}
//}

@<list>{clean}では大文字を小文字に変換した上で正規表現でアルファベットと数字以外の文字を除外しています。Goにおける正規表現は標準パッケージの@<code>{regexp}を使っています。それでは@<list>{export_test_sw}と同じように関数を公開した後に、テストで動作を確認してみます。

//footnote[regexp][@<href>{https://golang.org/pkg/regexp/}]

//list[clean_test][テキストクリーニングのテスト][go]{
func TestClean(t *testing.T) {
	tests := []struct {
		name string
		word string
		want string
	}{
		{
			name: "clean",
			word: "You are Best friend!!!!!(^^）",
			want: "you are best friend",
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

Goの標準パッケージでは@<code>{Thompson NFA}という正規表現の実装手法が採用されています。Russ Coxさんが「Regular Expression Matching Can Be Simple And Fast 」というブログ@<fn>{regexp_blog}で@<code>{Thompson NFA}を解説しています。ブログから引用した図@<img>{Regex}をみてみましょう。@<m>{a^n}に対する正規表現@<m>{a?^n a^n}のケースでのパフォーマンスです。@<code>{Thompson NFA}ではパフォーマンスが線形的に落ちていくのに対し、Perlで採用されている手法では最初は高速ですが、@<code>{n}がある程度大きくなると急激にパフォーマンスが落ちます。よってGoの正規表現が絶対的に遅いというわけではなく、場合によってはGoの正規表現の方が高速に動く可能性もあります。「Goの正規表現は遅い」ということを鵜呑みにせずに実際にBenchmarkをとって、問題になるパフォーマンスなのかどうかを確認することをオススメします。@<code>{@momotaro98}さんが日本語でまとめてくれているのでこちら@<fn>{regexp_blog_ja}も是非読んでみてください。

//image[Regex][入力文字に対する正規表現手法のパフォーマンス][scale=1]{
//}
===[/column]

//footnote[regexp_blog][@<href>{https://swtch.com/~rsc/regexp/regexp1.html}]
//footnote[regexp_blog_ja][@<href>{https://qiita.com/momotaro98/items/09d0f968d44c7027450d}]

=== 単語数のカウント

確率を計算する際に便利なように単語数をカウントする関数も実装しておきます。その際に今まで作ってきた関数を使います。

//list[count][テキストクリーニングのテスト][go]{
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

//list[count_test][countWordsのテスト][go]{
func TestCountWords(t *testing.T) {
	tests := []struct {
		name string
		word string
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
package gonbayes

// ...

// Classifier is documents categories clasifier.
type Classifier struct {
	// TODO: impliments
}

// Train trains documents classifier.
func (c *Classifier) Train(category string, document string) {
	// TODO: impliments
}

// Classify classify documents.
func (c *Classifier) Classify(document string) (category string) {
	// TODO: impliments
}
//}

この章では主に@<list>{classifier}の中身をそれぞれ実装していきます。

=== Classifier構造体を実装する

@<code>{Classifier}には確率計算で使うデータを格納するためのフィールドを定義しておきます。@<code>{Classifier}にはどんなフィールドが必要でしょうか？単語総数はもちろん、@<m>{P(C）}を計算するためのに各カテゴリに何個ドキュメントが格納されているかのデータ(@<code>{TotalDocsInCategories})とドキュメントの総数(@<code>{TotalDocs})の２つは必要です。@<m>{P(D|C）}ではカテゴリそれぞれの各単語の数(@<code>{Words})に加えて、全単語の総数(@<code>{TotalWords})、カテゴリごとに単語が何個あるかのデータ(@<code>{TotalWordsInCategories})も必要です。早速@<code>{Classifier}と、ついでに@<code>{Classifier}を初期化する関数を実装しましょう。

//list[classifier_init][Classifier の実装][go]{
// Classifier is documents categories clasifier.
type Classifier struct {
	Words             map[string]map[string]uint64
	TotalWords        uint64
	TotalDocsInCategories  map[string]uint64
	TotalDocs         uint64
	TotalWordsInCategories map[string]uint64
	Threshold         float64
}

// NewClassifier inits classifier.
func NewClassifier(categories []string, threshold float64) (c Classifier) {
	c = Classifier{
		Words:             make(map[string]map[string]uint64),
		TotalDocsInCategories:  make(map[string]uint64),
		TotalWordsInCategories: make(map[string]uint64),
		Threshold:         threshold,
	}

	for _, category := range categories {
		c.Words[category] = make(map[string]uint64)
	}
	return
}
//}

Goにおいてmapのゼロ値は@<code>{nil}なので明示的に@<code>{make}で初期化しておく必要があります。@<code>{Clasifier}の用途がある程度想定されるならmapの初期化にcapsを設定しておくとよいかもしれません。@<code>{NewClassifier}の第一引数では記事をどのように分類するカテゴリ数を受けます。これはその数でmapを初期化する為です。第二引数の@<code>{threshold}は低い確率を足切りするための閾値です。閾値に関しては後ほど説明します。

=== データセットからの学習を実装する

続いてClassifierを学習させるための関数@<code>{Classifier.Train}を実装しましょう。

//list[train][Classifierを学習させるための関数][go]{
// Train trains documents classifier.
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

@<list>{train}では与えられた学習用データからClassifierのフィールドを更新していくだけです。分類ではこのフィールドに入った値を使って確率を計算していきます。

=== ベイズ確率計算を実装する

それではNaive Bayesの根幹である確率計算を実装しましょう。

//list[pcd][各確率を計算するメソッド][go]{
// P(C）の計算
func (c *Classifier) pCategory(category string) float64 {
	return float64(c.TotalDocsInCategories[category]) / float64(c.TotalDocs)
}

// P(D|C）の計算
func (c *Classifier) pDocCategory(category string, document string) (p float64) {
	p = 1.0
	for word := range countWords(document) {
		p *= c.pWordCategory(category, word)
	}
	return p
}

// P(w|C）の計算 (P(D|C）の計算のため)
func (c *Classifier) pWordCategory(category string, word string) float64 {
	d := float64(c.Words[category][stem(word)])
	return d / float64(c.TotalWordsInCategories[category])
}

// P(C|D）の計算 (最終的に欲しい値)
func (c *Classifier) pCategoryDocument(category string, document string) float64 {
	return c.pDocCategory(category, document) * c.pCategory(category)
}
//}

@<list>{pcd}では@<m>{P(C）}と@<m>{P(D|C）}を計算する関数を実装し、それを使って　@<m>{P(C|D）}を計算しています(@<m>{P(C|D）} = @<m>{P(D|C）}*@<m>{P(C）}であることを思い出しましょう)。@<m>{P(D|C）}はカテゴリー内の単語出現確率(@<m>{P(w|C）})の掛け算でした。

次に @<code>{Classifier.pCategoryDocument} を使って、記事に対してそれぞれのカテゴリに分類される確率を求める関数を作っておきましょう。

//list[p][記事に対してそれぞれのカテゴリに分類される確率を求める関数][go]{
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
			want:     map[string]float64{"IT": 0.05, "エンタメ": 0, "経済": 0},
		},
	}

	for _, tt := range tests {
		tt := tt
		t.Run(tt.name, func(t *testing.T) {
			t.Parallel()
			c := gonbayes.NewClassifier(tt.categories, 0)
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

@<list>{p_test}の結果と最初に示したベイズ理論の図と結果が一致することを確認してください。

=== Naive Bayesを使って記事分類を実装する

これで任意の記事が与えられた時に、それぞれのカテゴリに何%の確率で分類されるのかをmapとして返す関数ができました。最後に@<code>{Classify}を実装しましょう。

//list[classify][記事に対してどのカテゴリに分類されるかを返す関数][go]{
// Classify classify documents.
func (c *Classifier) Classify(document string) string {
	prob := c.P(document)

	// 出現確率でソート
	type sorted struct {
		category    string
		probability float64
	}
	var sp []sorted
	for c, p := range prob {
		sp = append(sp, sorted{c, p})
	}
	sort.Slice(sp, func(i, j int) bool {
		return sp[i].probability > sp[j].probability
	})

	// 最も確率の高いカテゴリを返す (最も高い確率が閾値を下回ったら"unknown"と返す)
	var category string
	if sp[0].probability/sp[1].probability > c.Threshold {
		category = sp[0].category
	} else {
		category = "unknown"
	}

	return category
}
//}

おめでとうございます！これでNaive Bayesを使った記事分類が実装できました！

== 口コミの感情分析をやってみよう

早速、実装した記事分類を使って口コミの感情分析(ネガティブorポジティブ）判定をやってみましょう。@<code>{"I am happy"}というドキュメントはポジティブ、@<code>{"It's out of my mind"}というドキュメントはネガティブというように口コミを分類することがゴールです。

=== データセットの読み込み

学習に使うデータセットは「Sentiment Labelled Sentences Data Set」です。このデータセットはドキュメントとネガティブ/ポジティブ(0/1）がセットになっているテキストファイルです@<list>{slsds}。サイト@<fn>{slsds}から「Data Folder」>「sentiment labelled sentences.zip」でzipファイルをダウンロードしておきましょう。

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
package main

// ...

const (
	posiLabel = "positive"
	negaLabel = "negative"
)

func loadNegaPosiDataset(file string) (map[string]string, error) {
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

この関数は標準パッケージの機能である@<code>{func (*Scanner) Scan}@<fn>{scan}を使って1行ずつ読み込んでデータをmapに格納していきます。Go初心者の方は1行ずつ読み込む鉄板の処理なので覚えておきましょう。

=== いざ感情分析!!

いよいよ口コミの感情分析を行う@<code>{main}関数を実装します。

//footnote[scan][@<href>{https://golang.org/pkg/bufio/#Scanner.Scan}]

//list[main][感情分析を実行するmainパッケージ][go]{
func main() {
	s := flag.String("s", "", "input string")
	f := flag.String("f", "./yelp_labelled.txt", "dataset file path")
	flag.Parse()

	// Classifier の初期化
	class := []string{posiLabel, negaLabel}
	threshold := 1.4
	classifier := gonbayes.NewClassifier(class, threshold)

	// 「Sentiment Labelled Sentences Data Set」の読み込み
	dataset, err := loadNegaPosiDataset(*f)
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
$ go run cmd/negaposi/main.go -s "i unlike this" -f "yelp_labelled.txt"
negative
//}

確かに"@<code>{"i unlike this"}はネガティブな口コミです。それでは @<code>{"this is wonderful"}ではどうでしょうか。

//list[exec][口コミの感情分析の実行 2][go]{
$ go run cmd/negaposi/main.go -s "this is wonderful" -f "dataset/yelp_labelled.txt"
positive
//}

確かに口コミの感情分析ができています！これでNaive Bayesを使ったテキスト分類が成功していることがわかります。

== 学習済み分類器を gob パッケージで保存する

分類を実行する度に同じデータセットから同じ学習を行って分類器を生成するのは無駄だと気づいたかもしれません。最後に学習済みの@<code>{Classifier}をいつでも呼び出せるように修正しましょう。その為には標準パッケージである@<code>{encoding/gob}@<fn>{gob}を利用します。Goではデータ構造のシリアライズのために標準でいくつかのパッケージが用意されていますがGo内でデータ構造を使うのであればXMLやJSONなどにではなくバイナリエンコーディングした形で保存して読み込み時などで効率性を上げたいところです。そのような目的で@<code>{encoding/gob}が誕生しています。もちろんデータ構造さえ自明であればそのままEncodeできるので使用感としてもとてもシンプルに収まります。早速　@<code>{Classifier}に新しいメソッドを定義して学習済みの@<code>{Classifier}をEncode & Decodeできるようにしておきましょう。

//footnote[gob][@<href>{https://golang.org/pkg/encoding/gob/}]

//list[gob][gobを使った学習済みClassifierの Encode & Decode][go]{
// Encode trained Classifier
func (c *Classifier) Encode(fileName string) error {
	if c.TotalDocs == 0 {
		return errors.New("gonbayes: classifier is not trained yet")
	}
	f, err := os.Create(fileName)
	if err != nil {
		log.Fatal(err)
	}

	err = gob.NewEncoder(f).Encode(&c)
	if err != nil {
		return errors.Wrap(err, "gonbayes: failed to encode")
	}
	return nil
}

// Decode CBOW output file to struct.
func (c *Classifier) Decode(fileName string) error {
	f, err := os.Open(fileName)
	if err != nil {
		log.Fatal(err)
	}

	err = gob.NewDecoder(f).Decode(c)
	if err != nil {
		return errors.Wrap(err, "gonbayes: failed to dencode file")
	}
	return nil
}
//}

@<list>{gob}ではシンプルなメソッドとしてファイル名さえ指定すればEncode & Decodeできるように設計します。たとえば@<code>{Encode}を呼べば指定ファイルへ学習済みの　@<code>{Clasifier}をバイナリエンコーディングした形でファイルに保存できます。これを使って@<list>{main}を改良しましょう。

//list[main_with_gob][gob を 使った main の実装][go]{
func main() {
	s := flag.String("s", "", "input string")
	f := flag.String("f", "./yelp_labelled.txt", "dataset file path")
	t := flag.String("t", "", "trained model file path")
	o := flag.String("o", "classifier.gob", "trained file name")
	flag.Parse()

	class := []string{posiLabel, negaLabel}
	threshold := 1.4

	classifier := gonbayes.NewClassifier(class, threshold)

	// 学習済みファイルを指定した場合、それをデコードして Classifier を生成する。
	if *t != "" {
		err := classifier.Decode(*t)
		if err != nil {
			log.Fatal(err)
		}
	// 新しく学習して Classifirerを　生成する。
	} else {
		dataset, err := loadNegaPosiDataset(*f)
		if err != nil {
			log.Fatal(err)
		}

		for s, v := range dataset {
			classifier.Train(v, s)
		}
	}

	result := classifier.Classify(*s)
	fmt.Println(result)

	学習済み Classifier を バイナリエンコーディング
	if *t == "" {
		classifier.Encode(*o)
	}
}
//}

@<list>{main_with_gob}ではFlagを増やして学習済みのClassifierをEncodeしたファイルを指定できるようにしておきます。指定ファイルがあればそれをデコードして学習済み@<code>{Classifier}として使い、指定がなければ新しく学習します。

//list[exec_f][学習済み Classifer を使う][go]{
# 新しく学習して Classifier をバイナリエンコード
$ go run cmd/negaposi/main.go -s "this is wonderful" -f "dataset/yelp_labelled.txt"

# 学習済みの Classifier を利用した分類
$ go run cmd/negaposi/main.go -s "this is wonderful" -t "negaposi_classifier.gob"
//}

これで毎回学習を走らせなくても、感情分析ができるようになりました。もちろん@<code>{encoding/gob}はNaive Bayesのときだけでなく、ニューラルネットワークの実装で学習済みモデルを保存しておく用途でも使えます。一方でGo以外の言語で学習済みのモデルを使いたい場合はJSONなどにEncodeして利用してしまうのも一手です。

== もっとNaive Bayesの精度をよくするには

実はここまでの実装では分類の精度が高いとはいえません。１つずつ問題をみていきましょう。

=== ゼロ頻度問題

@<m>{P(D|C）}の計算を思い出しましょう。@<m>{P(D|C）}は各単語の確率の掛け算で表せました。しかし学習用データには存在しないワードがあった場合、つまり、１つでも確率0の単語があった場合、カテゴリーの確率が0になってしまいます。これを解決する為にさまざまな手法が提案されています。何個か対策方法があるのでみていきましょう。

==== 単に確率0を無視する

愚直な解決方法です。単に出現確率0の単語はなかったものとして計算していきます。つまり@<code>{pWordCategory}では出現確率が0だった場合、乗算に影響を与えないように1を返します。

//list[ignore][出現確率0の単語を無視する実装][go]{
func (c *Classifier) pWordCategory(category string, word string) float64 {
	n := float64(c.Words[category][stem(word)])
	if n == 0 {
		return 1
	}
	return n / float64(c.TotalWordsInCategories[category])
}
//}

==== Additive smoothings

@<list>{ignore}だと単語を単に無視する為、正しい対策とはあまりいえません。そこで@<code>{スムーシング}という手法を使ってゼロ頻度問題を解決します。@<code>{Additive smoothings}はもっとも単純なスムージングの1つです。実際よりもわずかに全単語の出現率を上げて計算します。すべてのカウントに係数@<code>{δ}を追加します。通常は@<code>{0<δ≤1}です。基本的には@<code>{δ=1}として計算します。分子に1加え、分母に全単語数を足します。

//list[add][Additive smoothings の実装][go]{
func (c *Classifier) pWordCategory(category string, word string) float64 {
	n = float64(c.Words[category][stem(word)]+1)
	d = float64(c.TotalWordsInCategories[category]+c.TotalWords)
	return n / d
}
//}

他にもさまざまなスムーシングがあるので調べてみてください。「An empirical study of smoothing techniques for
language modeling」@<fn>{sm} というペーパーが非常に勉強になります。

//footnote[sm][@<href>{http://u.cs.biu.ac.il/~yogo/courses/mt2013/papers/chen-goodman-99.pdf}]

#@# absolute discounting
#@# additive smoothing
#@# backoff

== おわりに

本章では Go とベイズ理論を使ったシンプルな記事分類の実装方法を紹介しました。是非みなさんもGoで自然言語処理に挑戦してみてください！
