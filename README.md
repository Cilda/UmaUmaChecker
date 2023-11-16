# ウマウマチェッカー
[![MSBuild](https://github.com/Cilda/UmaUmaChecker/actions/workflows/msbuild.yml/badge.svg)](https://github.com/Cilda/UmaUmaChecker/actions/workflows/msbuild.yml)
[![license](https://img.shields.io/github/license/Cilda/UmaUmaChecker)](https://github.com/Cilda/UmaUmaChecker/blob/master/LICENSE)
[![download](https://img.shields.io/badge/download-all%20releases-brightgreen)](https://github.com/Cilda/UmaUmaChecker/releases)
[![release-version](https://img.shields.io/github/v/release/Cilda/UmaUmaChecker)](https://github.com/Cilda/UmaUmaChecker/releases)
[![downloads](https://img.shields.io/github/downloads/Cilda/UmaUmaChecker/total)](https://github.com/Cilda/UmaUmaChecker)
[![CodeFactor](https://www.codefactor.io/repository/github/cilda/umaumachecker/badge)](https://www.codefactor.io/repository/github/cilda/umaumachecker)

ウマウマチェッカーは、ウマ娘プリティーダービーで画像認識によって選択肢のステータスを表示するツールです。  
Tesseract OCRを使用して画面のスクリーンショットからイベントを認識します。

## 目次
* [ウマウマチェッカー](#ウマウマチェッカー)
  * [使い方](#使い方)
    * [ウマ娘の自動選択](docs/umamusume-auto-choise.md)
    * [イベントの認識](docs/recognize-event.md)
    * [レシート因子撮影](docs/merge-umamusume-detail.md)
    * [設定画面](docs/settings.md)
  * [インストール](#インストール)
  * [実行](#実行)
  * [サポート](#サポート)
  * [ライセンス](#ライセンス)
  * [依存関係](#依存関係)
  * [ビルド](#ビルド)
  * [免責事項](#免責事項)
  * [更新履歴](#更新履歴)


## 使い方

使い方は以下を参照してください。  
- [ウマ娘の自動選択](docs/umamusume-auto-choise.md)  
- [イベントの認識](docs/recognize-event.md)  
- [レシート因子撮影](docs/merge-umamusume-detail.md)  
- [設定画面](docs/settings.md)  

## インストール
[リリースページ](https://github.com/Cilda/UmaUmaChecker/releases)から最新バージョンのUmaUmaChecker_vX.X.X_x64.zipをダウンロードします。  

## 実行
ZIPファイルを展開してできたフォルダ内のUmaUmaChecker.exeを実行します。

## サポート
不具合・機能の要望は[Issues](https://github.com/Cilda/UmaUmaChecker/issues)からIssueを作成してください。  
また、イベントが認識しない場合はツールのスクリーンショットで撮影された画像を添付してください。

## ライセンス
このソースコードはMITライセンスの元ライセンスされています。  
詳しくはLICENSEをご覧ください。  
__注__: このソフトウェアは異なるライセンスのパッケージに依存しています。

## 依存関係
このソースコードは以下のオープンソースソフトウェアを利用しています。
- OpenCV
- nlohmann-json
- simstring
- Tesseract OCR
- wxWidgets
- Boost.Log
- Boost.Locale

またライブラリ管理に __vcpkg__ を利用しています。

## ビルド
前提条件:  
- Visual Studio 2019以降
- vcpkg
  
vcpkgのインストールについては、https://github.com/microsoft/vcpkg をご覧ください。  
Visual Studioでvcpkgを利用するために以下を実行してください。
<pre>
> vcpkg integrate install
</pre>

プロジェクトをビルドするために以下でリポジトリをクローンするか、ダウンロードします。
<pre>
> git clone https://github.com/Cilda/UmaUmaChecker
</pre>

UmaUmaChecker.slnを開いてビルドを行います。  
必要なライブラリはvcpkgがダウンロードするので操作は必要ありません。  
ライブラリのビルドを行うため、環境によってはビルド時間が長くなることがあります。

## 免責事項  
本ソフトウェアを利用によって生じたすべての障害・損害・不具合等に関しては本ソフトウェアの作成者は一切の責任を負いません。  
各自の責任においてご使用ください。  
また、作成者は本ソフトウェアの開発をいつでも停止できるものとします。

## 更新履歴
<pre>
v1.6.1
[Update]
・リファクタリング
・スキル・継承タブの結合処理改善
[Add]
・ウィンドウの幅を変更できるように
・ステータスバーにウマ娘のウィンドウサイズを表示
[Fix]
・ステータスバーが正しく動作するように修正
 
v1.6.0
[Update]
・ウマ娘詳細のスキル・継承タブを撮影して結合する機能を追加

v1.5.1
[Fix]
・イベントの選択肢が特定のウィンドウサイズで読み取れない問題を修正

v1.5.0
[Change]
・「output.log にデバッグログを出力する」を有効の場合、識別されたイベント名をログに書き込む
・イベント認識精度向上
・ダークモードをすべてのウィンドウに反映
[Fix]
・キャライベントを選択肢から識別時に共通イベントが選択中の育成ウマ娘から取得されるよう修正

v1.4.6
[Change]
・イベントライブラリの管理方式を変更
[Fix]
・イベントを識別する際に選択肢を優先するよう修正

v1.4.5
[Fix]
・イベント選択肢の識別率を向上

v1.4.4
[Fix]
・シナリオイベントが認識できない不具合を修正

v1.4.3
[Update]
・イベント識別のngramを3から2に変更
・シナリオイベントの読み込み処理を修正
・識別精度向上
[Fix]
・終了時に例外が発生していた問題を修正
・イベント選択肢を識別する際のしきい値を追加
・テーマクラスのメモリリークを修正

v1.4.2
[Update]
・育成ウマ娘自動認識の精度を向上

v1.4.1
[Update]
・デバッグの設定を変更
・スタートと停止の動作を高速化
[Fix]
・キャプチャ方式「Windows 10(1903以降)」でキャプチャする際、ウマ娘を終了して再度起動するとウマウマチェッカーが強制終了していた問題を修正
・育成ウマ娘の共通イベントの選択肢名がすべて読み込まれていなかった問題を修正

v1.4.0
[Add]
・テーマを実装
・キャプチャ方式を追加
・起動時にイベントデータを自動更新
・スクリーンショットボタンを右クリックすると保存先を開く
・ログを保存するように
[Update]
・設定のフォントをコンボボックスに変更
[Fix]
・識別率修正
・更新確認ボタンで開くダイアログが閉じられたときにフォーカスが別ウィンドウに移動する問題を修正

v1.3.6
[Fix]
・ナリタブライアンの育成イベント「憧憬」が識別できない問題を修正

v1.3.5
[Add]
・OCRの速度に関する設定を追加
・スクリーンショットの画像タイプを設定できるように(PNG、JPEGを選択可能)
・起動時に更新がある場合に通知を行うように
・起動時の更新通知の設定を追加
・イベント情報更新の対象に「ScenarioEvents.json、Skills.json、ReplaceText.json」を追加
・イベントで取得できるスキルの効果を表示するように
[Update]
・シナリオイベントのタイトルからイベントを識別できない場合に選択肢からイベントを識別するように
・Aboutダイアログに使用ライブラリを記載

v1.3.4
[Add]
・育成ウマ娘のオートコンプリート追加
[Update]
・プレビューに保存されている画像を保存できるように
・効果の表示行数を変更可能
・オプションに表示行数の設定を追加
・イベント更新時に更新が不要な場合は「更新はありません」と出るように
[Fix]
・サポートカードイベントの選択肢名から識別できない問題を修正

v1.3.3
[Add]
・ステータスバーを追加(メモリ使用率、CPU使用率を表示)
・イベントを自動キャプチャする機能を追加
[Update]
・イベント識別アルゴリズムを改良  
[Fix]
・メモリリーク問題を修正

v1.3.2
[Update]
・イベント識別アルゴリズムを改良  
[Fix]
・メモリ使用量(メモリリーク)の改善  

v1.3.1
[Update]
・分岐のない選択肢を非表示にする設定の追加
・育成ウマ娘の自動認識  
　※育成開始画面を「スタート」押した状態で認識させないと機能しません  
[Fix]
・メモリ使用量の改善  
・イベント認識精度改善  

v1.3.0
[Update]
・フォント設定の追加
・ウィンドウのレイアウトを調整
[Fix]
・カスタム値が存在しない時にデフォルト値に初期化するように修正

v1.2.3
[Update]
・フォントをYu Gothic UIに変更
・選択肢のテキストボックスを4行表示に変更

v1.2.2
[Update]
・プレビューウィンドウにイベントを識別したらキャプチャ画像を表示
・アプリウィンドウの座標を記憶し再度アプリを起動したら前回と同じ座標に表示する機能を追加
[Fix]
・高DPI時に表示が崩れる現象を修正
・ダイワスカーレットでの育成イベントの不具合を修正

v1.2.1
[Update]
・バージョンチェック追加
・5択表示を追加
[Fix]
・認識精度向上

v1.2
[Update]
・GUIにwxWidgetsを使うように変更
・イベント名識別率の向上
[Add]
・Aboutダイアログ追加
・効果のテキストを色分けするように
・育成ウマのコンボボックスを☆順、ウマ娘順で並び替えるように
・アプリアイコン追加
[Fix]
・ポップアップ表示の修正
・イベント名「お疲れさまです……！」から「お疲れ様です……！」に修正
・キャラ共通イベントで関係のない選択肢名が表示される問題を修正

v1.1
[Update]
・各シナリオイベントを識別できるように追加
・イベント効果説明のエリア表示を調整(カーソルを重ねることで表示が拡張されるように変更)
[Fix]
・識別できないイベントを識別するように修正

beta(v1.0)
新規作成
</pre>
