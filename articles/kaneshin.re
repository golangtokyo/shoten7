= Secure Coding Practices in Go

== はじめに

株式会社エウレカのkaneshin@<fn>{kaneshin}です。普段はエウレカの技術戦略やエンジニア組織のマネジメントについて策定や執行をしつつ、Goをメインとしたツールやアプリケーションを開発しています。

//footnote[kaneshin][@<href>{https://twitter.com/kaneshin0120}]

本章では、GoでWebアプリケーションを開発している人向けにセキュリティ上の問題を発生させないようにするためのセキュアコーディングプラクティスを紹介します。

== セキュアコーディングプラクティスとは

セキュアコーディングプラクティスにはCERT（Computer Emergency Response Team）の「Top 10 Secure Coding Practices@<fn>{cert_scp}」やOWASP（Open Web Application Security Project）の「OWASP Secure Coding Practices@<fn>{owasp_scp}」が代表的にあげられます。

//footnote[cert_scp][@<href>{https://wiki.sei.cmu.edu/confluence/display/seccode/Top+10+Secure+Coding+Practices}]
//footnote[owasp_scp][@<href>{https://www.owasp.org/index.php/OWASP_Secure_Coding_Practices_-_Quick_Reference_Guide}]

セキュアコーディングプラクティスを知らない方でもWebアプリケーションを開発してきた方は、SQLインジェクションやクロスサイト・スクリプティングといった脆弱性の温床とならないようなプログラム設計やコーディングをしているはずです。
このようなセキュリティの問題が発生するたびに対応をしていては、リスクの高い問題が起こった場合に発生する被害も甚大なものとなってしまうため、問題が発生してから対応するといったリアクティブ（受動的）なセキュリティ対策を行うのではなく、プロアクティブ（能動的）にセキュリティリスクを軽減していくことがプログラムを設計する開発者に必要なスキルです。
セキュアコーディングプラクティスでは、プロアクティブにセキュリティリスクを軽減するためのコードの書き方や概念、重要な知識を体系的に知ることが出来ます。

本章ではCERTやOWASPの詳細なセキュアコーディングプラクティスの解説はしませんが、開発に関わる人は一般的かつ重要なWebアプリケーションセキュリティの弱点の影響について理解をするために目を通しておくことをおすすめします。

== インジェクション（OWASP A1）

WebアプリケーションではSQLインジェクションやスクリプトインジェクション、コマンドインジェクションといったコードの脆弱性を突いたコードインジェクションと、データに不正なデータを送信して意図しない操作や権限をコントロールするデータインジェクションが主なリスクとして認識されています。

インジェクションを防ぐためには様々な種類のインジェクションを知り、コーディングをしているときに入力や出力の値をしっかりとサニタイズし、検証することによってプログラムに損害を与えない値を使っていくことが重要です。

=== SQLインジェクション

SQLインジェクションは入力値のバリデーションが不足して生じることが多いインジェクションの一つです。特に、文字列連結にてクエリを生成することによってSQLインジェクションの脆弱性を組み込んでしまうことが多くあります。

//list[SQL_Injection_Login][文字列連結によるSQLインジェクション][go]{
func Login(ctx context.Context, email, password string) error {
  // Fetch the hashed password corresponding to the provided email
  query := "SELECT hash FROM accounts WHERE email = " + email + " LIMIT 1"
  row := db.QueryRowContext(ctx, query)

  /// ...
  return nil
}
//}

この@<code>{Login}関数に@<code>{email}の値を検証せずに文字列連結でクエリをビルドすることにより、意図しないクエリが生成されて解釈されてしまいます。

//list[SQL_Injection_Login_Invalid_Email][不正な値によるSQLインジェクション][go]{
func main() {
  /// ...
  email := "'' OR 1 = 1"
  password := "foobarbazqux"
  err := Login(ctx, email, password)
}

func Login(ctx context.Context, email, password string) error {
  // Fetch the hashed password corresponding to the provided email
  query := "SELECT hash FROM accounts WHERE email = " + email + " LIMIT 1"
  /// "SELECT hash FROM accounts WHERE email = '' OR 1 = 1 LIMIT 1"

  /// ...
}
//}

@<code>{SELECT hash FROM accounts WHERE email = '' OR 1 = 1 LIMIT 1}というクエリが生成されてしまい、@<code>{OR 1 = 1}によって@<code>{WHERE}文が必ず@<code>{TRUE}となります。このようなSQLインジェクションを発生させないために、プリペアドステートメントを使用してSQLインジェクションを回避します。

//list[Solve_SQL_Injection_Login][プリペアドステートメントを使用][go]{
func Login(ctx context.Context, email, password string) error {
  // Fetch the hashed password corresponding to the provided email
  query := "SELECT hash FROM accounts WHERE email = ? LIMIT 1"
  row := db.QueryRowContext(ctx, query, email)

  /// ...
  return nil
}
//}

プレースホルダによりクエリをビルドする方法で、パラメータとなる@<code>{?}に値をバインドしてクエリを解釈していきます。このプリペアドステートメントではSQLの構文が確定しているため、プレースホルダに対応するパラメータの値がリテラルから外れて解釈されることはありません。

=== ファイルパスによるコードインジェクション

ファイルパスの結合不備による脆弱性を突いたコードインジェクションです。ファイルやディレクトリのパスを結合するのに加算演算子を用いて結合しているのをよく見かけますがGoには@<code>{path, path/filepath}というパッケージが存在します。このパッケージを適切に利用してサニタイズすることにより、意図しないファイルアクセスを制御することが可能なのとLinuxとWindowsの互換性も保つことができます。

//list[Filepath_Injection][ファイルパス結合不備による脆弱性][go]{
func main() {
  name := "foobar"
  fp := filepath.Join("/tmp", name)
  body, err := ioutil.ReadFile(fp)
  if err != nil {
    log.Fatal(err)
  }
  fmt.Print(string(body))
}
//}

このコードは@<code>{/tmp}ディレクトリに存在するファイルを出力するプログラムです。@<code>{filename}を@<code>{"foobar"}としているため@<code>{/tmp/foobar}のファイルを出力することができます。しかし、@<code>{filename}を@<code>{"../etc/shells"}のようにすることによって@<code>{/tmp}配下ではない@<code>{/etc/shells}のファイルを参照することが可能になっています。

//list[Solve_Filepath_Injection][path/filepathを使用][go]{
func main() {
  name := "../etc/foobar"
  p, err := filepath.Abs(name)
  if err != nil {
    log.Fatal(err)
  }
  fp := filepath.Join("/tmp", filepath.FromSlash(filepath.Clean(p)))
  body, err := ioutil.ReadFile(fp)
  if err != nil {
    log.Fatal(err)
  }
  fmt.Print(string(body))
}
//}

@<code>{path/filepath}パッケージを活用してLinuxとWindowsで互換性を持たせつつ、@<code>{filepath.Abs(name)}をすることによって一度絶対パスに置き換えてから@<code>{filepath.Join}をしています。

== 認証の不備（OWASP A2）

ユーザーを認証させるWebアプリケーションは基本機能といっても過言でないくらい実装されていることが増えています。アカウント名やメールアドレスとパスワードによる認証や、ソーシャルアカウントと連携してアカウント登録させることも非常に多くなっています。この認証について不適切に実装されていることによって、攻撃者はユーザーのパスワードやセッショントークを侵害したり、ユーザーの認証情報を一時的または永続的に取得します。

=== パスワードの生成と照合

パスワードを用いた認証はよく用いられます。パスワード認証を実装する開発者は非推奨のハッシュ化アルゴリズムの@<code>{MD5}や@<code>{SHA-1}を用いないことと、自身が開発したハッシュ化アルゴリズムは使用しないことがセキュリティリスクを軽減させることに繋がります。OWASPでは@<code>{bcrypt, PDKDF2, Argon2 and scrypt}のハッシュ化アルゴリズムを使用することとソルトを用いて強度を向上させることを推進しています。

Goでは@<code>{golang.org/x/crypto}パッケージが提供されているので@<code>{crypto}を用いてパスワードをハッシュ化させて照合用に保存します。下記は@<code>{crypto/bcrypt}を用いてセキュアにアカウント作成する例です。

//list[bcrypto_signup][セキュアなアカウント作成][go]{
import "golang.org/x/crypto/bcrypt"

func Signup(ctx context.Context, email, password string) error {
  // Hash the password with bcrypt
  hashedPassword, err := bcrypt.GenerateFromPassword(
    []byte(password),
    bcrypt.DefaultCost,
  )
  if err != nil {
    return err
  }

  query := "INSERT INTO accounts SET hash = ?, email = ?"
  stmt, err := db.PrepareContext(ctx, query)
  if err != nil {
    return err
  }

  _, err = stmt.ExecContext(ctx, hashedPassword, email)
  if err != nil {
    return err
  }
  return nil
}
//}

@<code>{bcrypt.GenerateFromPassword}関数で平文のパスワードをハッシュ化しています。ここで保存したハッシュをアカウントログイン時に比較して問題ないかの確認を行います。

//list[bcrypto_signup][アカウントのログイン検証][go]{
import "golang.org/x/crypto/bcrypt"

var ErrMismatchedCredentials = errors.New("wrong email or password given")

func Login(ctx context.Context, email, password string) error {
  // Fetch the hashed password corresponding to the provided email
  query := "SELECT hash FROM accounts WHERE email = ? LIMIT 1"
  row := db.QueryRowContext(ctx, query, email)

  var hashedPassword string
  err := row.Scan(&hashedPassword)
  if err == sql.ErrNoRows {
    return ErrMismatchedCredentials
  }
  if err != nil {
    return err
  }

  err = bcrypt.CompareHashAndPassword(
    []byte(hashedPassword),
    []byte(password),
  )
  if err == bcrypt.ErrMismatchedHashAndPassword {
    return ErrMismatchedCredentials
  }
  if err != nil {
    return err
  }
  return nil
}
//}

効率的に照合を行うために、DBからアカウント情報を@<code>{email}のみで取得しています。それからパスワードを@<code>{bcrypt.CompareHashAndPassword}関数で比較して認証を行います。

==== 認証時のロギング

認証フローにおいて、ロギングする際に誤って平文のパスワードをログに残してしまうといった誤ったロギングなどには気をつけましょう。また、エラーハンドリングも今回は最小限に留めていますが、振る舞いの違いからプログラムの脆弱性を突かれてしまうこともあるため、適切なエラーハンドリングとロギングを行いましょう。

==== ランダム値の生成

ランダムな数値や文字列を生成することを認証処理時に行うことがたまにあります。プログラムにおいて本来の意味の「ランダム」を提供することは非常に難しく「偽りのランダム」とも言われるほどです。事実、プログラミングにおけるランダムとは予測不可能な値を提供することが目的とされています。そのためプログラムのランダムのロジックを知ってしまえばランダムが予測可能となります。

//list[pseudo_random_1][ランダムではないランダム値][go]{
package main

import (
  "fmt"
  "math/rand"
)

func main() {
    fmt.Println(rand.Intn(1234))
}
//}

このプログラムは何度実行しても同じ値の@<code>{603}を返却します。

//cmd{
$ go run main.go
603
$ go run main.go
603
//}

これを防ぐためにランダム値を生成するときに@<code>{rand.Reader}という値を元にしてランダム値を生成するように修正します。

//list[pseudo_random_2][偽りのランダム値][go]{
package main

import (
  "crypto/rand"
  "fmt"
  "math/big"
)

func main() {
  r, err := rand.Int(rand.Reader, big.NewInt(1234))
  if err != nil {
    panic(err)
  }
  fmt.Println(r)
}
//}

//cmd{
$ go run main.go
724
$ go run main.go
1623
//}

プログラミングにおけるランダムを理解している人は@<code>{Seed}という生成の種となる値を設定してランダムのような値を生成していることを知っています。しかし、それを知らずに用いてしまうと「ランダム値と思ったら固定値を使ってしまっている」なんてことになってしまいます。

== 不十分なロギングとモニタリング（OWASP A10）

Webアプリケーションに限らず、ロギングとモニタリングは様々なインシデントを解決するために非常に重要なデータであり指標となります。ロギングやモニタリングが非効率な状態だと攻撃者がシステムを攻撃やデータを改ざん・破壊していても検知できないとことになってしまいます。

=== エラーハンドリング

Goのエラーハンドリングは何度か話題にあがっています。Go2に向けて「Proposal: Go 2 Error Inspection@<fn>{go2_error}」が提案されていますが、Go1向けのExperimental Packageとして@<code>{xerrors "golang.org/x/xerrors"}がリリースされています。

//footnote[go2_error][@<href>{https://go.googlesource.com/proposal/+/master/design/29934-error-values.md}]

コールスタックの実行中サブルーチンのエラーを呼び出し元へ情報を失わずにエラー情報を伝播させていくかが開発者の間で考えられていて、@<code>{errors "github.com/pkg/errors}パッケージなどの外部ライブラリが利用されていましたが、Go2では標準ライブラリとしてエラートレースできるように検討されています。

//list[usage_xerrors][xerrorsの基本的な使い方][go]{
package main

import (
  "fmt"

  "golang.org/x/xerrors"
)

func main() {
  err1 := do()
  if err1 != nil {
    err2 := xerrors.Errorf("error in main(): %v", err1)
    fmt.Printf("%+v\n", err2)
    // error in main():
    //     main.main
    //         /home/gopher/tmp/main.go:12
    //   - error in do():
    //     main.do
    //         /home/gopher/tmp/main.go:33

    err3 := xerrors.Errorf("error in main(): %w", err1)
    fmt.Printf("%+v\n", err3)
    // error in main():
    //     main.main
    //         /home/gopher/tmp/main.go:21
    //   - error in do():
    //     main.do
    //         /home/gopher/tmp/main.go:33
  }
}

func do() error {
  return xerrors.New("error in do()")
}
//}

新しくエラーを定義するには@<code>{xerrors.New("..")}で作成します。エラーをフォーマットするには@<code>{xerrors.Errorf("...: %v")}のように「@<code>{: %v}」を挿入してエラーの情報をフォーマットさせた上で上書きします。エラーをラップするには@<code>{xerrors.Errorf("...: %w")}のように「@<code>{: %w}」を挿入してエラーの情報をラップします。@<code>{fmt.Print}のVerbose表示ではフォーマットさせたエラーと同様の出力になるので、基本的には@<code>{: %w}によってエラーをラップします。

ラップさせたエラーをアンラップするには@<code>{Unwrap}関数を使用します。

//list[usage_xerrors][エラーのアンラップ][go]{
func main() {
  err1 := do()
  if err1 != nil {
    err2 := xerrors.Errorf("error in main(): %w", err1)
    fmt.Printf("%+v\n", xerrors.Unwrap(err2))
    // error in do():
    //     main.do
    //         /home/gopher/tmp/main.go:33
  }
}
//}

他にも@<code>{xerrors}に関数が定義されていますが、最低限のエラーハンドリングをするために上記のようにエラーをラップして呼び出し元で適切にエラーハンドリングをするように心がけましょう。

=== ロギング

ロギングは信頼性の高いシステムを提供するために必要不可欠です。しかし、パスワードやクレジットカード情報、個人情報にあたる機微な情報をログとして残してしまうことはデータを適切に扱うことができていないため、どのように使うのかを定義した上で必要不可欠なデータだけをログとして保存することが運用において重要になります。そしてログを仕込む場所も明確に定義して抜け漏れが発生しないようにロギングします。ロギングのタイミングは入力のバリデーションに失敗したときや認証の不備によるログインの失敗、アクセスコントロールや不正なセッショントークンを用いて接続をされたとき、管理者しかアクセスできない機能の監査ログなど多様なデータをログとして残しておくべきです。

Goのログパッケージで有名なものとして、様々なフォーマットをプラグインとしてサポートしている@<code>{logrus}@<fn>{logrus}や高速にログを書き込める@<code>{zap}@<fn>{zap}、ログレベルを提供したシンプルな@<code>{glog}@<fn>{glog}などがあります。開発しているアプリケーションの要件に合うログパッケージを選び効率良いロギングを行いましょう。

//footnote[logrus][@<href>{https://github.com/sirupsen/logrus}]
//footnote[zap][@<href>{https://github.com/uber-go/zap}]
//footnote[glog][@<href>{https://github.com/golang/glog}]

== おわりに

本章では、Webアプリケーションのセキュリティレベルを向上させるためのセキュアコーディングプラクティスをGoのコードを用いて解説しました。セキュアコーディングプラクティスの概念はGoに依存しない、全てのプログラミング言語で活用できるガイドラインです。セキュリティの対応は後手に回れば回るほど問題が発生したときに生じる被害が甚大なものになるので、コーディングをするときに少しでも本章を思い出して実装をしていってもらえればと思います。

また、GoのセキュアコーディングについてOWASPがまとめているリポジトリ@<fn>{owasp_go_scp}もありますので興味がある方はぜひご覧ください。

//footnote[owasp_go_scp][@<href>{https://github.com/OWASP/Go-SCP}]
