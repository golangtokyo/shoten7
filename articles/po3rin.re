= Goとベイズ理論でゼロから記事分類を実装してみよう！

== はじめに

白ヤギコーポレーションでバックエンドエンジニアをしている@po3rin@<fn>{po3rin}です。今回は Go とベイズ理論を使った記事分類の実装方法を紹介します。Naive Bayesの理論を学べるのはもちろんですが、Goでのテキスト前処理の方法や、学習済み分類器の保存方法なども含めてご紹介します。Go で記事分類を実装することで、すでに Go で実装されたアプリケーションへの組み込みも容易になります。

#@# TODO: motivate

//footnote[po3rin][@<href>{Goに関する情報などを呟いてます。https://twitter.com/po3rin}]

== ベイズの理論と Naive Bayes

この節では Naive Bayes の理論をザックリと理解することがゴールです。実装に困らないようになんとなくやっていることが分かる程度で大丈夫です。

=== さらっとベイズの理論を抑えよう

ベイズ理論(@<img>{Bayes})とは「条件付き確率に関して成り立つ定理」のことです。

//image[Bayes][ベイズの理論][scale=0.7]{
//}

一見すると難しそうに見えますが、１つずつ式の意味を見ていくとそれほ程難しくはないです (@<table>{p})。

//table[p][確率の表記]{
式	意味
------------------------------------
P(A)	Aが起こる確率
P(B|A)	Aが起こった時にBが起こる確率
//}

記事のカテゴリ分類を例にベイズの定理の使い方を見ていきましょう。知りたいのは「ドキュメントが与えられた時に、とあるカテゴリに分類される確率」です。これは@<table>{p}を見返すと@<code>{P(C|D)}と表せることがわかります。ベイズの理論より @<code>{P(C|D)} は @<code>{P(C)}（カテゴリーの出現率）と @<code>{P(D|C)}（カテゴリーが与えられた時のドキュメント出現率）の掛け算です(@<code>{P(D)}（カテゴリー内のドキュメント出現率）はすべてのカテゴリで同じであるため無視できます）。

=== 具体例から Naive Bayes の具体的な計算方法を理解する

理論だけだと何が行われているのか分からないと思うので具体的な例で理解しましょう。5つの文章 @<table>{documents} がある場合を考えます。

//table[documents][ドキュメントの実際の例]{
カテゴリ	ドキュメント
------------------------------------
経済	株価
経済	保険
IT	Pythonで株価
IT	PythonとGo
エンタメ	マーベル
//}

==== P(C）

@<code>{P(C)} はカテゴリーの出現率です。たとえば「経済カテゴリ」の出現率は @<code>{経済カテゴリの数 / カテゴリ総数}なので 2/5 になります。

#@# TODO: table

==== P(D|C）

@<code>{P(D|C)} はカテゴリーが与えられた時のドキュメント出現率でした。ドキュメントは一連の単語で構成されているため @<code>{P(D|C)} はカテゴリ内のドキュメントのすべての単語の結合確率で計算できます。これは計算が難しいように思えます。一方でカテゴリ内の単語出現率は簡単です。ITカテゴリには単語が4つあるので、たとえば「Python」が「ITカテゴリ」に出現する確率は 2/4 、「株価」が「ITカテゴリ」に出現する確率は 1/4 です。

#@# TODO: table

この数字を使って欲しい @<code>{P(D|C)} を計算します。Naive Bayes では @<code>{P(D|C)} は 各単語の確率の掛け算で表せます。@<code>{P(Pythonで株価|IT)} は (「Python」が「ITカテゴリ」に出現する確率 2/4 ）* (「株価」が「ITカテゴリ」に出現する確率 1/4 ）で 1/8 になります。

#@# TODO: table

実は今までの計算では本来あるべき単語の条件付き確率を無視し、各単語が互いに独立していると仮定してます。つまり単語がドキュメント内にランダムに現れると仮定しているのです。この過程が Naive Bayes が Naive たる所以です。

== Goによるテキストの前処理

実際に Naive Bayes を実装する前に入力されるテキストを前処理する必要があります。今回は英語を対象にするので、英語のテキストを前処理する必要があります。本章のコードでは GitHub のサンプルパッケージ名と同じ「gonbayes」パッケージとして実装していきます。

=== stop words の削除

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
//footnote[swlidt][NLTK の Stop Wrods の一覧。https://gist.github.com/sebleier/554280]

stop words のマップは key に stop word を置き、value に空の構造体を置きます。Goにおいて空の構造体はメモリを消費しません。その為マップが存在確認のためだけに存在する場合は空の構造体が有用です。初耳の方は実際に@<code>{unsafe.Sizeof}を使って0になることを確認しましょう。

//list[sizezero][空の構造体][go]{
fmt.Println(unsafe.Sizeof(s))
//}

これで任意の単語が stop words かどうかを検知する関数ができました。テストで動作を確認してみましょう。関数が非公開なので @tenntenn さんの記事である「Go Fridayこぼれ話：非公開（unexported）な機能を使ったテスト #golang」@<fn>{export_test}を参考にテストを書いていきます。まず非公開関数をテストするために@<code>{export_test.go}を作成して関数をテストでのみ公開できるようにしておきます。

//footnote[export_test][@<href>{https://tech.mercari.com/entry/2018/08/08/080000}]

//list[export_test_sw][isStopWordsの公開][go]{
package gonbayes

var IsStopWords = isStopWords

//}

これでテスト時にだけIsStopWordsがビルドされるようになりました。早速テストを書いていきましょう。

//list[test_isStopWords][isStopWordsのテスト][go]{
package gonbayes_test

import (
	"testing"

	"github.com/po3rin/gonbayes"
)

func TestIsStopWords(t *testing.T) {
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
			got := gonbayes.IsStopWords(tt.word)
			if got != tt.want {
				t.Errorf("want = %v, got = %v\n", tt.want, got)
			}
		})
	}
}
//}

@<code>{you} は stop word なので @<code>{true} が返り、@<code>{great} は stop word ではないなので @<code>{false} が返ることが期待されます。テストを実行するとテストが通るはずです。テストの書き方に関しては @budougumi0617 さんのブログ @<fn>{how_to_test} が体系的にまとまっていて勉強になるので @<list>{test_isStopWords} で知らないコードが会った人は読んでみることをお勧めします。

//footnote[how_to_test][@<href>{https://budougumi0617.github.io/2018/08/19/go-testing2018/#sub-test-testing-t-run}]

=== 単語のステミング処理

swim という単語は「swims」「swimming」「swimmer」などの複数の語形があります。単語の語形が変化する言語では前処理としてステミング処理を行う必要があります。英語で使えるステミングアルゴリズムは、「Lancaster」「Porter」「Snowball Stemmers」などがあります。Goのパッケージとして提供されているステミングライブラリである程度starが付いているパッケージを調べたところ @<table>{stem_pkgs} が候補。

//table[stem_pkgs][ドキュメントの実際の例]{
パッケージ名	説明
------------------------------------
reiver/go-porterstemmer	Goで実装された「Porter Stemming」
agonopol/go-stem	こちらもGoで実装された「Porter Stemming」
kljensen/snowball	Go で実装された「Snowball Stemming」
//}


たまに kljensen/snowball が更新されるくらいで、活発に開発されているパッケージは見つからなかったです。今回は英語以外の言語にも対応している kljensen/snowball を採用しましょう。Goで任意のstringをstemmingする関数を実装します。

//list[stem][stemming処理の実装][go]{
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

@<code>{snowball.Stem} 関数は未知の単語がきた時などにエラーを返しますが、今回はそのまま単語を返し、エラーを無視する実装にしています。いくつかの単語でテストしてみましょう。@<list>{export_test_sw} と同じようにテストしたい関数を公開した後に、stemmingのテストを書いてみます。

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

このテストで stemming ができていることが確認できます。

=== テキストクリーニング

「You are Best friend!!!!!(^^）」 という文字列がある場合、顔文字や「！」は邪魔なのでテキストクリーニングをしておきます。つまり先ほどの文字列を「you are best friend」というテキストに変換します。

//list[clean][テキストクリーニングの実装][go]{
func clean(document string) string {
	document = regexp.MustCompile("[^a-zA-Z 0-9]+")
	document.ReplaceAllString(strings.ToLower(document), "")
	return document
}
//}

これで大文字を小文字にした上で正規表現でアルファベットと数字以外の文字を除外しています。@<list>{export_test_sw} と同じように関数を公開した後に、テストで動作を確認してみます。

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

テキストクリーニングができているのが確認できます。

=== 単語数のカウント

確率を計算する際に便利なように単語数をカウントする関数も実装しておきます。その際に今まで作ってきた関数を使います。

//list[count][テキストクリーニングのテスト][go]{
func countWords(document string) (wordCount map[string]int) {
	document = clean(document)
	words := strings.Split(document, " ")
	wordCount = make(map[string]int)
	for _, word := range words {
		if !isStopWords(word) {
			key := stem(strings.ToLower(word))
			wordCount[key]++
		}
	}
	return
}
//}

これでdocumentに単語がそれぞれ何個あったかを返す関数ができました。@<list>{export_test_sw} と同じようにテストしたい関数を公開した後にテストを書いて動作を確認しましょう。

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

前処理を行う関数ができたので、いよいよ Naive Bayes を実装します。設計としては @<code>{Classifier} 構造体に学習用のメソッドと分類用のメソッドを生やします。

//list[classifier][countWordsのテスト][go]{
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

この章では主に @<list>{classifier} の中身をそれぞれ実装していきます。

=== Classifier 構造体を実装する

@<code>{Classifier} には確率計算で使うデータを格納するためのフィールドを定義しておきます。@<code>{Classifier} にはどんなフィールドが必要でしょうか？単語総数はもちろん、@<code>{P(C)} を計算するためのに各カテゴリに何個ドキュメントが格納されているかのデータ(@<code>{TotalDocsInCategories})とドキュメントの総数(@<code>{TotalDocs})の２つは必要です。@<code>{P(D|C)} では各単語の数(@<code>{Words})に加えて、全単語の総数(@<code>{TotalWords})、カテゴリごとに単語が何個あるかのデータ(@<code>{TotalWordsInCategories})も必要です。早速 @<code>{Classifier} と、ついでに @<code>{Classifier} を初期化する関数を実装しましょう。

//list[classifier_init][countWordsのテスト][go]{
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

Goにおいて map のゼロ値は @<code>{nil} なので明示的に @<code>{make} で初期化しておく必要があります。@<code>{Clasifier} の用途がある程度想定されるなら map の初期化にcapsを設定しておくとよいかもしれません。@<code>{NewClassifier}の第一引数では記事をどのように分類するカテゴリ数を受けます。これはその数でmapを初期化する為です。第二引数の @<code>{threshold} は低い確率を足切りするための閾値です。閾値に関しては後ほど説明します。


=== データセットからの学習を実装する

続いてClassifierを学習させるための関数 @<code>{Classifier.Train}を実装しましょう。

//list[train][Clasifierを学習させるための関数][go]{
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

@<list>{train} では与えられた訓練データから Classifier のフィールドを更新していくだけです。分類ではこのフィールドに入った値を使って確率を計算していきます。

=== ベイズ確率計算を実装する

それでは Naive Bayes の根幹である確率計算を実装しましょう。

//list[pcd][各確率を計算するメソッド][go]{
// P(C)の計算
func (c *Classifier) pCategory(category string) float64 {
	return float64(c.TotalDocsInCategories[category]) / float64(c.TotalDocs)
}

// P(D|C)の計算
func (c *Classifier) pDocCategory(category string, document string) (p float64) {
	p = 1.0
	for word := range countWords(document) {
		p *= c.pWordCategory(category, word)
	}
	return p
}

// P(w|C)の計算 (P(D|C)の計算のため)
func (c *Classifier) pWordCategory(category string, word string) float64 {
	return float64(c.Words[category][stem(word)]+1) / float64(c.TotalWordsInCategories[category])
}

// P(C|D)の計算 (最終的に欲しい値)
func (c *Classifier) pCategoryDocument(category string, document string) float64 {
	return c.pDocCategory(category, document) * c.pCategory(category)
}
//}

これで @<code>{P(C)} @<code>{P(D|C)} を計算する関数を実装し、それを使って　@<code>{P(C|D)} を計算します (@<code>{P(C|D)} = @<code>{P(D|C)} * @<code>{P(C)} であることを思い出しましょう)。@<code>{P(D|C)} は カテゴリー内の単語出現確率 (@<code>{P(w|C)}) の掛け算でした。

#@# TODO: test

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

=== Naive Bayes を使って記事分類を実装する

これで任意の記事が与えられた時に、それぞれのカテゴリに何%の確率で分類されるのかをmapとして返す関数ができました。最後に @<code>{Classify} を実装しましょう。

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

おめでとうございます！これで Naive Bayes を使った記事分類が実装できました！

== 口コミの感情分析をやってみよう

早速、実装した記事分類を使って口コミの感情分析(ネガティブ or ポジティブ）判定をやってみましょう。

=== データセットの読み込み

学習に使うデータセットは「Sentiment Labelled Sentences Data Set」@<fn>{slsds}です。このデータセットはドキュメントとネガティブ/ポジティブ(0/1）がセットになっているテキストファイルです@<list>{slsds}。サイト@<fn>{slsds}から「Data Folder」>「sentiment labelled sentences.zip」でzipファイルをダウンロードしておきましょう。

//footnote[slsds][@<href>{「Sentiment Labelled Sentences Data Set」https://archive.ics.uci.edu/ml/datasets/Sentiment+Labelled+Sentences}]

//list[slsds][Sentiment Labelled Sentences Data Set][go]{
Wow... Loved this place.	1
Crust is not good.	0
Not tasty and the texture was just nasty.	0
Stopped by during the late May bank holiday off Rick Steve recommendation and loved it.	1
The selection on the menu was great and so were the prices.	1
Now I am getting angry and I want my damn pho.	0
Honeslty it didn't taste THAT fresh.)	0

...(省略)

//}

続いて @<code>{cmd/negaposi/main.go} を作り、感情分析を実行する main 関数を作りましょう。まずは「Sentiment Labelled Sentences Data Set」を読み込んでGoで使える形にして返す関数を作りましょう。

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

この関数は標準パッケージの機能である @<code>{func (*Scanner) Scan} @<fn>{scan}を使って1行ずつ読み込んでデータをmapに格納していきます。Go初心者の方は1行ずつ読み込む鉄板の処理なので覚えておきましょう。

=== いざ Sentiment analysis !!

いよいよ記事分類を行う@<code>{main}関数を実装します。

//footnote[scan][@<href>{https://golang.org/pkg/bufio/#Scanner.Scan}]

//list[main][記事のネガポジ分類][go]{
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

//list[exec_nega][口コミの感情分析の実行][go]{
$ go run cmd/negaposi/main.go -s "i unlike this" -f "yelp_labelled.txt"
negative
//}

確かに "i unlike this" はネガティブなコメントです。それでは @<code>{"this is wonderful"} ではどうでしょうか。

//list[exec][口コミの感情分析の実行 2][go]{
go run cmd/negaposi/main.go -s "this is wonderful" -f "dataset/yelp_labelled.txt"
positive
//}

確かに口コミの感情分析ができています！これで Naive Bayes を使ったテキスト分類が成功していることがわかります。

== 訓練済み分類器を gob パッケージで保存する

分類を実行する度に同じデータセットから同じ学習を行って分類器を生成するのは無駄だと気づいたかもしれません。最後に訓練済みの @<code>{Classifier} をいつでも呼び出せるように修正しましょう。その為には標準パッケージである @<code>{encoding/gob} を利用します。

== おわりに
