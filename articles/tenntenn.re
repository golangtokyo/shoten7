= Goの新しいコントラクト

== 始めに

株式会社メルペイのバックエンドエンジニアの@tenntenn@<fn>{tenntenn}です。
この記事はGopherCon 2019のIan Lance Taylor氏の"Generics in Go"@<fn>{goingenerics-liveblog}というセッションとその後公開されたDesign Doc@<fn>{contract-draft2}を元に書いています。

//footnote[tenntenn][@<href>{https://tenntenn.dev}]
//footnote[goingenerics-liveblog][@<href>{https://about.sourcegraph.com/go/gophercon-2019-generics-in-go}]
//footnote[contract-draft2][@<href>{https://go.googlesource.com/proposal/+/master/design/go2draft-contracts.md}]

なお、2019年8月時点では本稿で扱う内容がそのままGo2に入るという話ではありません。
「Go2でジェネリクスが入る」という情報は正確ではなく、新しい機能提案の1つとしてジェネリクスが議論されているというだけです。
むしろ、後方互換性がうまく保たれ、よい実装が登場すればGo1.Xでリリースされることすらあるでしょう。

本稿ではGoにおけるジェネリクスの必要性については議論しません。
その点について知りたい場合は、Goの公式ブログの記事"Why Generics"を参照するとよいでしょう。
また本稿では、他の言語のジェネリクスやそれに類する機能との比較は行いません。

== 型パラメタ

本稿で議論しているジェネリクスには、型パラメタという概念が登場します。
型パラメタを用いることで特定の型に限定しないジェネリックな関数や型を定義できます。
型パラメタを用いて定義した関数や型を使用する際には、型引数として具体的な型を指定します。

たとえば、任意の型のスライスを引数に受け取るような関数@<code>{Print}は、型パラメタを用いると@<list>{Print_T}ように定義できます。
型パラメタは、関数と引数の間に書くことができ、ここでは型パラメタとして@<code>{T}型を設けています。

//list[Print_T][型パラメタを用いた例][go]{
func Print(type T)(s []T) {
  for _, v := range s {
    fmt.Println(v)
  }
}
//}

関数@<code>{Print}を呼び出すには、引数となるスライスのほかに、そのスライスの型を型引数として指定する必要があります。
たとえば、@<code>{int}型のスライスを引数に渡したい場合は、@<list>{Print_int}ように型引数として@<code>{int}型を渡す必要があります。

//list[Print_int][型引数としてint型を指定した例][go]{
Print(int)([]int{1, 2, 3})
//}

== コントラクト

型パラメタという概念を導入することで関数や型を柔軟に定義できるようになりました。しかし、型パラメタだけでは任意の型に対する関数や型を定義できるだけです。

たとえば、@<list>{Stringify_without_contract}のようにある型のスライスを@<code>{string}型のスライスに変換する@<code>{Stringify}を考えた場合、型パラメタだけでは不十分でしょう。

//list[Stringify_without_contract][型パラメタだけでは不十分な例][go]{
// 型パラメタだけでは不十分
func Stringify(type T)(s []T) (ret []string) {
  for _, v := range s {
    // T型がStringメソッドを持っているか分からない
    ret = append(ret, v.String())
  }
}
//}

@<code>{Stringify}関数の@<code>{for}で定義されている変数@<code>{v}は@<code>{T}型であり、
@<code>{T}型は呼び出し時に型引数として指定されます。
@<code>{T}型は任意の型であるため、@<code>{v.String()}のように呼び出せるとは限りません。
@<code>{int}型のように@<code>{String}メソッドを定義していない型も型引数として指定できてしまうからです。

そこで@<code>{T}型に制約を入れるコントラクトという概念を導入されました。
@<list>{Stringify_with_contract}のように、型パラメタの後ろにコントラクトを指定することで@<code>{T}型を@<code>{stringer}コントラクトで制限できます。
なお、ここでは@<code>{stringer}コントラクトは、@<code>{String}メソッドを実装している型に限定するコントラクトとします。

//list[Stringify_with_contract][コントラクトを用いた例][go]{
// 型TはStringメソッドを実装している型に限定される
func Stringify(type T stringer)(s []T) (ret []string) {
    for _, v := range s {
        ret = append(ret, v.String())
    }
}
//}

== 旧コントラクト

コントラクトは、GopherCon 2018で公開された"Go 2 Draft Designs"@<fn>{go2draft}のうち、Genericsに関するDraft Designがあり@<fn>{go2draft_generics}そこで登場しました。
そこで提案されたコントラクトを新しく提案されたコントラクトと区別をつけるために本稿では"旧コントラクト"と呼ぶこととします。

//footnote[go2draft][@<href>{https://blog.golang.org/go2draft}]
//footnote[go2draft_generics][@<href>{https://go.googlesource.com/proposal/+/master/design/go2draft-generics-overview.md}]

旧コントラクトに対する議論は、Go Conference運営が主催した"Go 2 Draft Designs フィードバック会"@<fn>{go2fb}でも行われました。
そのときの議論はドキュメント@<fn>{go2fb_doc}にまとめられているので興味のある読者は参照するとよいでしょう。

//footnote[go2fb][@<href>{https://gocon.connpass.com/event/101167/}]
//footnote[go2fb_doc][@<href>{http://bit.ly/go2-contracts-feedback-ja}]

この議論の中で、筆者は"Contract Tricks"@<fn>{contract_tricks}という形で旧コントラクトの定義の難しさを示しました。

//footnote[contract_tricks][@<href>{https://gist.github.com/tenntenn/3049e06b301b1d7df952d41b12bef7d8}]

たとえば、旧コントラクトでは@<code>{String}メソッドを実装している型を表す@<code>{stringer}コントラクトは@<list>{old_contract}のように書けます。
実際に@<code>{String}メソッドを呼び出すようなコードを書くことで、型@<code>{T}が@<code>{String}メソッドを実装していることを規定していました。

しかし、この方法ではたしかに@<code>{String}メソッドを実装する型は許容されますが、そうではない型についても許容されてしまいます。

//list[old_contract][旧コントラクトによるstringerの定義][go]{
contract stringer(t T) {
  // フィールドかメソッドか分からない
  var _ string = t.String()
}
//}

たとえば、@<list>{field_func}のような@<code>{String}という名前の@<code>{string}型を返す関数をフィールドとして保持するような構造体についても許容されてしまいます。

//list[field_func][関数をフィールドとしてもつ構造体][go]{
type A struct {
  String func() string
}
//}

そのため、メソッドのみを許容するためには、@<list>{contract_trick_method}のように定義する必要がありました。

//list[contract_trick_method][メソッドのみを許容する旧コントラクト][go]{
contract Stringer(t T) {
  var _ interface{
    String() string
  } = t
}
//}

@<code>{String}メソッドを規定するインタフェース型の変数に代入することにより、メソッドだけが許容されます。

このように、旧コントラクトは表現力がある分、気を付けて定義する必要がありました。そのため、うまく定義しないと想定外の使われ方をしてしまう可能性があります。

== 新コントラクト

旧コントラクトのDesign Docに対してGoコミュニティのさまざまなフィードバックがありました。それらを受けてシンプルなコントラクトがあらためて登場しました。
ここでは、新しく提案されたコントラクトを新コントラクトと呼ぶことにします。

新コントラクトの詳しい定義についてはDesign Doc@<fn>{contract-draft2}を参照するとよいでしょう。

新コントラクトでは、@<code>{stringer}コントラクトは@<list>{contract_stringer}のように定義できます。

//list[contract_stringer][新コントラクトにおけるstringerの定義][go]{
contract stringer(T) {
  T String() string
}
//}

@<code>{stringer}コントラクトは、@<code>{T}型が@<code>{String}メソッドを実装していることを規定しています。
旧コントラクトと比べて表現力は劣るものの、定義方法が非常にシンプルになりました。

== 複数の型パラメタとコントラクト

型パラメタは2つ以上指定することもできます。たとえば、@<list>{multiple_typeparam}のように関数@<code>{Print2}を定義した場合、型@<code>{T1}と型@<code>{T2}は別の型であってもかまいません。

//list[multiple_typeparam][複数の型パラメタを指定した例][go]{
func Print2(type T1, T2)(s1 []T1, s2 []T2) { ... }
//}

一方、@<list>{typeparam_same}のように定義した@<code>{Print2Same}関数は引数@<code>{s1}と@<code>{s2}は同じ型@<code>{T1}である必要があります。

//list[typeparam_same][同じ型パラメタで指定された型をもつ複数の引数][go]{
func Print2Same(type T1)(s1 []T1, s2 []T1) { ... }
//}

コントラクトは@<list>{multiple_typeparam_contract}のように複数の型パラメタに対して定義することも可能です。

//list[multiple_typeparam_contract][複数の型パラメタを用いたコントラクトの定義][go]{
contract viaStrings(To, From) {
  To   Set(string)
  From String() string
}
//}

@<code>{viastrings}コントラクトは、型パラメタ@<code>{To}と@<code>{From}に対してコントラクトを定義しています。@<code>{To}型は@<code>{Set}メソッド、@<code>{From}型は@<code>{String}メソッドを定義している型であることが規定されています。

@<code>{viastrings}コントラクトは@<list>{multiple_typeparam_contract}のように記述することで型パラメタ@<code>{To}と@<code>{From}に対して制約を設けています。
@<code>{SetViaStrings}関数は、引数で指定された@<code>{From}型のスライスの要素に対して、@<code>{String}メソッドを呼び出すことで@<code>{string}型の値を取得しています。そして、@<code>{To}型のスライスの要素に@<code>{Set}メソッドを用いてその値を設定しています。

//list[use_multiple_typeparam_contract][複数の型パラメタに対するコントラクトの指定][go]{
func SetViaStrings(type To, From viaStrings)(s []From) []To {
  r := make([]To, len(s))
  for i, v := range s {
    r[i].Set(v.String())
  }
  return r
}
//}

== 型パラメタと型定義

型パラメタは関数定義だけではなく型定義に用いることができます。
たとえば、任意の型のベクトルを表す@<code>{Vector}型は次のように定義できます。

//list[typeparam_typedef][型パラメタを用いた型の定義][go]{
type Vector(type Element) []Element
//}

@<code>{int}型を要素としてもつ@<code>{Vector}型の変数を定義したい場合には、@<list>{typeparam_type_typearg}のように型引数として@<code>{int}型を指定することで行えます。

//list[typeparam_type_typearg][型パラメタを用いた型への型引数の指定][go]{
var v Vector(int)
//}

型パラメタを用いて定義され型にも今までどおりメソッドを設けることができます。
@<code>{Vector}型の値に要素を追加する@<code>{Push}メソッドは@<list>{typeparam_method}に定義できます。

//list[typeparam_method][型パラメタを用いた型のメソッド][go]{
func (v *Vector(Element)) Push(x Element) { *v = append(*v, x) }
//}

レシーバや引数では型パラメタの@<code>{Element}に対して、@<code>{contract}キーワードや@<code>{type}キーワードをつける必要はありません。

型定義の中で型パラメタを用いることも可能です。
たとえば、@<list>{typedef_typeparam}のように構造体のフィールドに型パラメタをもつ型を指定できます。

//list[typedef_typeparam][型定義で型パラメタを用いた例][go]{
type List(type Element) struct {
  next *List(Element)
  val Element
}
//}

複数の型パラメタをもつ型においては、@<list>{typedef_typeparam_reverse}のように型パラメタの順番を変えることはできません。

//list[typedef_typeparam_reverse][型パラメタの順番を変えた例][go]{
type P(type Element1, Element2) struct {
  F *P(Element2, Element1) // (Element1, Element2)の順でなければダメ
}
//}

しかし、Design Docにはこのルールは緩める可能性があると記載されています。

型パラメタを用いた型の定義においても、@<list>{typedef_contract}のようにコントラクトを用いることができます。

//list[typedef_contract][型定義にコントラクトを用いた例][go]{
type StringableVector(type T stringer) []T

func (s StringableVector(T)) String() string {
  var sb strings.Builder
  sb.WriteString("[")
  for i, v := range s {
    if i > 0 {
      sb.WriteString(", ")
    }
    sb.WriteString(v.String())
  }
  sb.WriteString("]")
  return sb.String()
}
//}

@<list>{typedef_contract}では、@<code>{StringableVector}型の要素は@<code>{stringer}コントラクトが指定されているため、@<code>{String}メソッドを実装している必要があります。

@<list>{typeparam_embedded}のように、構造体型の定義で型パラメタを匿名フィールドとして埋め込むことができます。

//list[typeparam_embedded][型パラメタを埋め込んだ例][go]{
type Lockable(type T) struct {
  T
  mu sync.Mutex
}

func (l *Lockable(T)) Get() T {
  l.mu.Lock()
  defer l.mu.Unlock()
  return l.T
}
//}

@<code>{Lockable}は@<code>{T}型の値にロックを掛けることのできる構造体型として定義されます。
@<code>{Get}メソッドによってロックされている@<code>{T}型の値を取得できるようにしています。

== 型パラメタと型エイリアス

@<list>{type_alias}のように、型パラメタを使って定義された型に対して型引数を定義してエイリアス型を定義できます。

//list[type_alias][型エイリアスで型引数を指定した例][go]{
type Vector(type Element) []Element
type VectorInt = Vector(int)
//}

この場合、@<code>{VectorInt}型は要素が@<code>{int}型の@<code>{Vector}型になります。
なお、型パラメタを使って定義された型に対して型エイリアスを定義する場合、必ず型引数によって型を指定する必要があります。

== メソッドと型パラメタ

@<list>{typedef_contract}の@<code>{StringableVector}型のように、型パラメタを使って定義された型のメソッドの定義において、レシーバの定義に用いられた型パラメタを用いることは可能です。

しかし、メソッドを定義する際に新しく型パラメタを設けることはできません。
これは型パラメタをもつメソッドがあるインタフェースを実装しているかどうかということを明確にするためです。
たとえば、@<list>{interface}のようなインタフェースがあった場合を考えてみましょう。

//list[interface][インタフェースの定義][go]{
type I interface {
  M(n int)
}
//}

次に@<list>{implements_m}のように型パラメタをもつメソッド@<code>{M}を定義できたと仮定します。

//list[implements_m][型パラメタをもつメソッドを定義できたと仮定した場合][go]{
type MyInt int
func (n MyInt) M(type T)(v T) {
  // ...
}
//}

このとき、@<code>{MyInt}型のメソッド@<code>{M}はインタフェース@<code>{I}を実装していることになるのでしょうか。
型引数として@<code>{int}型を指定すれば実装していると言っても良さそうですが、簡単にはいかなさそうです。
執筆時のDesign Docの定義ではメソッドに型パラメタを持たせることは不可能ですが、必要性が高まった場合には追加される可能性があります。

== コントラクトの埋め込み

定義済みのコントラクトを別のコントラクトに埋め込むことが可能です。
たとえば、@<list>{contract_embedded}のように@<code>{PrintStringer}コントラクトに@<code>{stringer}コントラクトを埋め込んでいます。

//list[contract_embedded][コントラストの埋め込み]{
contract PrintStringer(X) {
	stringer(X)
	X Print()
}
//}

これは@<list>{contract_embedded_same}のように定義した場合と同じです。

//list[contract_embedded_same][コントラストの埋め込み]{
contract PrintStringer(X) {
	X string() string
	X Print()
}
//}

埋め込んだ@<code>{stringer}コントラクトに型引数のように@<code>{X}を指定しています。
これはDesign Docには明記してありませんが、@<list>{contract_embedded_typeparam}のように複数の型パラメタに対するコントラクトを定義する場合にどの型パラメタに対するコントラクトなのかを見分けるためでしょう。

//list[contract_embedded_typeparam][コントラストの埋め込みにおける型パラメタの指定][go]{
contract C2(X, Y) {
  C1(X)
  Y M()
}
//}

== コントラストの定義におけるレシーバ型の利用

コントラクトの定義に出てくるレシーバ型をそのメソッドの引数や戻り値に用いることができます。
たとえば、@<list>{contract_receiver_type}のようにコントラクトの定義で@<code>{T}型をレシーバとする@<code>{Equal}メソッドを記述している場合に、
そのメソッドの引数として@<code>{T}型を取ることができます。

//list[contract_receiver_type][コントラストの定義におけるレシーバ型の利用][go]{
package compare

// equalコントラストはレシーバ型と同じ引数を取る
// Equalメソッドをもつコントラクトです。
contract equal(T) {
  T Equal(T) bool
}
//}

@<code>{equal}コントラクトを用いると、@<list>{Index}のような関数を定義できます。
@<code>{Index}関数は@<code>{T}型のスライス@<code>{s}に@<code>{T}型の変数@<code>{e}と同じ値の要素があればそのインデックスを返す関数です。
スライス@<code>{s}の要素と変数@<code>{e}の値の比較には@<code>{==}ではなく、@<code>{Equal}メソッドを用いています。
@<code>{T}型には@<code>{equal}コントラクトが制約として加わっており、かつスライス@<code>{s}の要素も変数@<code>{e}も@<code>{T}型であるため
@<code>{Equal}メソッドによる比較が行えます。

//list[Index][Index型の定義][go]{
// Index関数はT型のスライスsにT型の変数eと
// 同じ値の要素があればそのインデックスを返す関数です。
func Index(type T equal)(s []T, e T) int {
  for i, v := range s {
    // eとvはともにT型であるため、e.Equal(v)と呼び出しても問題ありません
    if e.Equal(v) {
      return i
    }
  }
  return -1
}
//}

@<code>{equal}コントラクトを満たす型は@<list>{equal_int}のように定義できます。
@<code>{EqualInt}型は@<code>{Equal}メソッドを実装しているため、
@<code>{EqualInt}型は@<code>{equal}コントラクトを満たしています。
なお、@<code>{EqualInt}型の@<code>{Equal}メソッドの引数が@<code>{EqualInt}型である点に注意してください。

//list[equal_int][equalコントラクトを満たす型の定義][go]{
import "compare"

type EqualInt int

// Equalメソッドを定義しているので
// EqualInt型はcompare.equalコントラクトを満たしています。
func (a EqualInt) Equal(b EqualInt) bool {
  return a == b
}

func Index(s []EqualInt, e EqualInt) int {
  return compare.Index(EqualInt)(s, e)
}
//}

== 型パラメタの相互参照

コントラクトの各型パラメタへのメソッドの記述において、それぞれのメソッドの引数や戻り値でコントラクトが使用している別の型パラメタを参照しても問題ありません。

たとえば、グラフ理論のグラフを表す型を定義したい場合にコントラクトを用いると@<list>{graph}のように定義ができます。

//list[graph][コントラクトを用いたグラフ型の定義][go]{
package graph

contract G(Node, Edge) {
  Node Edges() []Edge
  Edge Nodes() (from Node, to Node)
}

type Graph(type Node, Edge G) struct { ... }
func New(type Node, Edge G)(nodes []Node) *Graph(Node, Edge) { ... }
func (g *Graph(Node, Edge)) ShortestPath(from, to Node) []Edge { ... }
//}

@<code>{G}コントラクトは2つの型パラメタ@<code>{Node}と@<code>{Edge}に対するコントラクトです。
@<code>{Node}型は@<code>{Edges}という、そのノードに接続されているエッジを取得するためのメソッドを持ちます。
一方で@<code>{Edge}型は@<code>{Nodes}という、そのエッジの始端と終端のノードを取得するためのメソッドを持ちます。

@<code>{G}コントラクトを用いて@<code>{Graph}型という構造体型を定義できます。
@<code>{Graph}型は@<code>{G}コントラクトで規定されている@<code>{Node}型と@<code>{Edge}型をそれぞれノードとエッジを表す型として用いるものです。
@<code>{Graph}型の値は@<code>{New}関数のように@<code>{Node}型のスライスがあれば生成できるでしょう（エッジは@<code>{Node}型から取得できます。）。

@<code>{G}コントラストで表現された制約は単純なグラフ構造を表現するには十分です。
これらを使って@<code>{ShortestPath}メソッドのような最短経路を求めるような処理が行えます。

@<code>{Node}型や@<code>{Edge}型はあくまで型パラメタとして定義されているため、実際には@<code>{G}コントラクトを満たす型が型引数で指定されます。
たとえば、@<list>{vertex}のように@<code>{Node}型パラメタに対応する@<code>{Vertex}型や@<code>{Edge}パラメタに対応する@<code>{FromTo}型を定義することで@<code>{G}コントラクトを満たすことができます(厳密にはメソッドを実装しているのはそれぞれのポインタ型です）。

//list[vertex][Gコントラクトを満たす型の定義][go]{
type Vertex struct { ... }
func (v *Vertex) Edges() []*FromTo { ... }
type FromTo struct { ... }
type (ft *FromTo) Nodes() (*Vertex, *Vertex) { ... }
//}

つまり、@<code>{*Vertex}型と@<code>{*FromTo}型を@<code>{graph_new}のように@<code>{New}関数に型引数として渡すことができます。

//list[graph_new][Graph型の値の生成][go]{
var g = graph.New(*Vertex, *FromTo)([]*Vertex{ ... })
//}

グラフ構造をもつようなデータは数多く存在するでしょう。
たとえば、ディレクトリ構造や静的解析で用いる抽象構文木や制御フローグラフなどがあります。
これまではそれらのグラフ構造をインタフェースを用いて抽象化し型を定義することが多かったでしょう。
そうすることでグラフに対するアルゴリズムを適用できます。
しかし、型パラメタとコントラクトを用いることでグラフ自体の性質をうまく定義し、実際にグラフを扱う際には具象型として扱えます。

もちろん、@<code>{G}コントラクトを満たす型として@<list>{graph_interface}のようにインタフェースを定義しても問題ありません。
しかし、インタフェース型を定義する利点はほとんどないでしょう。

//list[graph_interface][Gコントラクトを満たすインタフェース][go]{
type NodeInterface interface { Edges() []EdgeInterface }
type EdgeInterface interface { Nodes() (NodeInterface, NodeInterface) }
//}

== コントラクトに対する型パラメタの指定

@<code>{func F(type T1, T2 C)(v T1)T2{...}}のように関数を定義した場合、@<code>{C}コントラクトは型パラメタ@<code>{T1}と@<code>{T2}に対して適用されます。これは@<code>{func F(type T1, T2 C(T1, T2))(v T1)T2{...}}と記述した場合と同じように扱われます。

一方、@<list>{MapAndPrint}のように関数定義で複数の型パラメタが指定されており、そのうちの片方にコントラクトを指定したい場合には@<code>{stringer(M)}のように記述できます。
このように記述することで型パラメタ@<code>{M}にだけ@<code>{stringer}コントラクトが適用されます。

//list[MapAndPrint][コントラクトの型パラメタの指定][go]{
func MapAndPrint(type E, M stringer(M))(s []E, f(E) M) []string {
  r := make([]string, len(s))
  for i, v := range s {
    r[i] = f(v).String()
  }
  return r
}
//}

== コントラクト構文の詳細

コントラクトの定義はパッケージのトップレベルでしか行うことができません。
関数内でコントラクトを定義できるようにしても良さそうですが、メソッド定義と同様にできません。
トップレベルでしか定義できないように制限することで、Go1の後方互換性を保ったままにできます。

キーワードが単純に増えてしまうと既存のコードにその文字列が変数名などの識別子として用いられていた場合はコンパイルエラーになります。
そこで次のようなルールを設けることにより、既存のコードへの影響を取り除くことができます。

* トップレベルのコントラクトの定義を行った場合のみ@<code>{contract}はキーワードとして扱われ、それ以外は識別子として扱われる。
* コントラクトを定義した場合、そのパッケージでは@<code>{contract}はキーワードとして扱われる。
* @<code>{contract}をキーワードとして扱う場合は互換性が崩れてしまうが、それは意図した変更になる。

コントラクトは他のトップレベルで定義した型や関数と同様に、名前が大文字で始まる場合はエクスポートされ、そうでない場合はエクスポートされません。

== 型パラメタの値はボックス化されない

Goの執筆時点の実装ではインタフェース値は常にポインタを保持します。
そのため、インタフェース変数に非ポインタ値を入れると値がボックス化されます。
つまり、実際の値はヒープまたはスタックの別の場所に格納され、
インタフェース値にはその場所へのポインタが保持されます。

一方で新コントラクトのDesign Docでは、ジェネリクスの値はボックス化されないようになっています。
たとえば、@<list>{setter_Strings}のように文字列を引数に取る@<code>{Set}メソッドを有するような型に限定する@<code>{setter}コントラクトを定義したとします。
そして、引数に文字列のスライスを取り、その要素を@<code>{Set}メソッドで各要素に設定した@<code>{[]T}型の値とエラーを返す@<code>{Strings}関数を定義します。

//list[setter_Strings][setterコントラクトとStrings関数の定義][go]{
package from
  contract setter(T) {
    T Set(string) error
  }

  func Strings(type T setter)(s []string) ([]T, error) {
    ret := make([]T, len(s))
    for i, v := range s {
      if err := ret[i].Set(v); err != nil {
        return nil, err
      }
    }
    return ret, nil
  }
//}

一方、別のパッケージで@<list>{Settable}のように型@<code>{Settable}が定義されていたとします。

//list[Settable][ボックス化しない例][go]{
type Settable int
func (p *Settable) Set(s string) (err error) {
  *p, err = strconv.Atoi(s)
  return err
}

func F() {
  // numsの型は[]Settable
  nums, err := from.Strings(Settable)([]string{"1", "2"})
  if err != nil { ... }

  // Settable型の値はint型に直接キャストできる
  // 変数firstには1がセットされる
  first := int(nums[0])
  ...
}
//}

@<code>{Settable}型で@<code>{from.Strings}関数を呼び出すと@<code>{[]Settable}型の値（とエラー）が返されます。
@<code>{Settable}型は単に@<code>{int}型をベースにして定義された整数型です。
そのため、@<code>{from.Strings}関数のような型パラメタを用いた関数を経由したとしてもポインタとしてボックス化される訳ではありません。

同様に型パラメタを用いて定義された型がインスタンス化（型引数として具体的な型が指定された時）されると、それらの具体的な型を保持する型となります。
たとえば、@<list>{Pair}のように@<code>{Pair}型を定義した場合、@<code>{pair.Pair(int, string)}のように型引数を指定すると、ボックス化されず、@<code>{struct { f1 int; f2 string }}として定義した場合と同様に扱われます。

//list[Pair][Pair型の定義][go]{
package pair
type Pair(type carT, cdrT) struct {
  f1 carT
  f2 cdrT
}
//}

== 関数の引数における型推論

多くの場合、型パラメタを使用して関数を呼び出す場合、型推論を行うことで明示的に型引数を書く必要がありません。
たとえば、@<list>{call_Print_int}のように型引数で指定した型のスライス表示する関数@<code>{Print}について考えてみます。

//list[call_Print_int][Print関数に型引数としてint型を指定した場合][go]{
Print(int)([]int{1, 2, 3})
//}

この場合の型引数@<code>{int}は、引数@<code>{[]int{1, 2, 3}}から推論できます。
型引数の推論は関数のすべての型パラメタが引数として使用されている場合に限ります。
戻り値として使用されていたり、関数本体で使用されているだけでは推論することはできません。
たとえば、前述した@<code>{from.Strings}関数は、型パラメタを引数ではなく戻り値に用いているため、関数呼び出し時に推論することはできません。

関数の型引数の推論を行うために、型の単一化というプロセスが行われます。
型の単一化では2つの型のリストを用います。
関数呼び出し側では実引数（型引数ではない）の型のリストと関数定義側の仮引数（型パラメタではない）の型のリストです。
@<code>{Print}関数の例では、前者は@<code>{[]int}、後者は@<code>{[]T}となります。
なお、単一化では関数定義側の型のリストのうち、型パラメタを用いていない型は不要であるため、リストから取り除いておきます。

型の単一化は2パスアルゴリズムです。
最初のパスでは呼び出し側の型なしの定数とそれに対応する関数定義側の型を無視します。

関数呼び出し側と関数定義側の型リストがそれぞれ対応しているかチェックしていきます。
関数定義側で複数同じ型パラメタが現れる場合には、関数呼び出し元でも対応する型が複数回現れる必要があります。
たとえば、@<code>{func compare(T)(a, b []T){/* 略 */}}のような関数定義の場合、引数@<code>{a}と@<code>{b}の型として型パラメタ@<code>{T}が2回登場しています。
その際、関数呼び出し側でも@<code>{f([]int{10}, []int{20})}のように引数として対応する型の値を指定する必要があります。
なお、この場合は@<code>{int}型が@<code>{T}型に対応しています。
型のリストが対応していない場合には、型の単一化は失敗しエラーが報告されます。

2つめのパスでは、無視していた型なしの定数についてチェックします。
型なしの定数がない場合や対応する型が見つからない型パラメタがない場合は型の単一化が終了します。
そうでない場合は、型なしの定数のデフォルトの型（@<code>{100}の場合は@<code>{int}型など）を用いて、最初のパスと同じアルゴリズムで型パラメタを対応させていきます。

たとえば、@<list>{type_inference_Print}のように@<code>{Print}関数を呼び出した場合を考えます。
@<code>{[]int}型と@<code>{[]T}型が比較され、@<code>{int}型と型パラメタ@<code>{T}が対応させることができるので型の単一化に成功します。
そのため、@<code>{Print}関数を呼び出す際に型推論が働くため、型引数として@<code>{Print(int)(s1)}のように@<code>{int}型を指定する必要がありません。

//list[type_inference_Print][Print関数の型推論][go]{
s1 := []int{1, 2, 3}
// func Print(s []T) { ... }
Print(s1)
//}

@<list>{transform_Slice}のようにもっと複雑な例を見てみましょう。

//list[transform_Slice][スライスを変換するtransform.Slice関数の定義][go]{
package transform
func Slice(type From, To)(s []From, f func(From) To) []To {
  r := make([]To, len(s))
  for i, v := range s {
    r[i] = f(v)
  }
  return r
}
//}

2つの型パラメタ@<code>{From}と@<code>{To}は入力引数として用いられているため、@<list>{type_inference_Slice}のように型推論を行うことが可能です。

//list[type_inference_Slice][Slice関数の型推論を伴った呼び出し][go]{
strs := transform.Slice([]int{1, 2, 3}, strconv.Itoa)
//}

型の単一化により、@<code>{[]int}型と@<code>{[]From}から@<code>{int}型と型パラメタ@<code>{From}が対応します。
また、@<code>{strconv.Itoa}、つまり@<code>{func(int) string}型と@<code>{func(From) To}から@<code>{int}型と型パラメタ@<code>{From}および@<code>{string}と型パラメタ@<code>{To}が対応することが分かります。

型パラメタ@<code>{From}については、型の単一化で2回登場しますが、ともに@<code>{int}型と対応しているため問題ありません。そのため、@<code>{type_inference_Slice}の関数呼び出しは、型推論により型引数として@<code>{int}と@<code>{string}を指定したことになります。

次に実引数が型なしの定数の場合の型推論について考えてみましょう。
@<list>{pair_New}のように@<code>{pair.New}関数が定義されているとします。

//list[pair_New][pair.New関数][go]{
package pair
func New(type T)(f1, f2 T) *Pair(T) { ... }
//}

@<code>{pair.New(1, 2)}のように呼び出した場合、それぞれの実引数は型なしの定数です。
そのため、型の単一化の最初のパスでは処理されません。
そこで、定数@<code>{1}と{2}のデフォルトの型である@<code>{int}型を用いて、2つめのパスが行われます。
@<code>{int}型が型パラメタ@<code>{T}と対応しているため、@<code>{pair.New(int)(1, 2)}のように呼び出している場合と同等だと判断されます。

@<code>{pair.New(1, int64(2))}のように呼び出した場合、それぞれの実引数は@<code>{1}が型なしの定数、@<code>{int64(2)}が@<code>{int64}型の定数となります。
型の単一化の最初のパスでは、@<code>{int64(2)}から@<code>{int64}型と型パラメタ@<code>{T}が対応することが分かります。
最初のパスが終了した後、対応する型が見つからない型パラメタがないため、型の単一化は終了します。
そのため、@<code>{pair.New(1, int64(2))}は@<code>{pair.New(int64)(1, int64(2))}と呼び出した場合と同様であると判断されます。

@<code>{pair.New(1, 2.5)}のように呼び出した場合、それぞれの実引数は型なしの定数です。
そのため、型の単一化の最初のパスでは処理されません。
そこで、定数@<code>{1}と{2.5}のデフォルトの型である@<code>{int}型と@<code>{float64}型を用いて、2つめのパスが行われます。
@<code>{int}型と@<code>{float64}型が型パラメタ@<code>{T}と対応しているため、矛盾が発生し型の単一化は失敗し、コンパイルエラーになります。

型推論は型パラメタにコントラクトが指定しているかどうかにかかわらずに行われます。
まず型推論が行われ型引数として暗黙的に指定される型が決定され、その後その型がコントラクトを満たしているか調べられます。

型推論に成功したとしてもコンパイラは実引数が仮引数に割り当て可能かチェックします。
これは型なしの定数を実引数として取った場合に限る話ではありません。

型推論は非常に便利な機能ですが、型パラメタやコントラクトの設計に影響を与えるものではなく、単に利便性のために存在します。そのため、型パラメタやコントラクトに関する実装（ランタイムやコンパイラ）の初期段階では省略される可能性があります。

関数呼び出し時の型推論のほかに@<list>{type_inference_composite_literal}のようなコンポジットリテラルにおける型推論を考えることもできます。しかし、このようなコードがどのくらいの頻度で必要になるのかは分からないため、実装されない可能性があります。

//list[type_inference_composite_literal][コンポジットリテラルにおける型推論][go]{
type Pair(type T) struct { f1, f2 T }
var V = Pair{1, 2} // Pair(int){1, 2}のように推論される
//}


