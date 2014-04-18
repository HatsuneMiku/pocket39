Pocket39
========

Pocket Miku
-----------

* ポケット・ミク [NSX-39](http://otonanokagaku.net/nsx39/)

なにこれ？ / AEFF480R
---------------------

CEG[C240]R

ポケット・ミク ( [NSX-39](http://otonanokagaku.net/nsx39/) )
さんにひらがなで書いたテキストを読んで(喋って)もらうソフトです。

楽譜を一行ごとに並列に書き込んでおくことで、テキストファイルの歌詞を歌ってもらうことも出来ます。

標準機能である「歌詞のプリセット」の文字数制限(64文字)を気にせずに、ほぼ無限に歌ってもらえます。

ファイルれい / GGGDCR
---------------------

F#A#[C#F#240]R

曲ファイル hotarunohikari.p39 の一部です。

    ほたるのひかり / ]B=240[E=360E=E=240G240F360=0=F240
        まどのゆき / G120=0=E=240E=240G240#0#0#240[C600]R
    # (中略)
    あけてぞけさは / [C240]B=360GG240E=240F360=0=F240
        わかれゆく / G120=0=E=360=0=C240]B=240[E=600R
    # (中略)
    つくしのきわみ / E=240A=360A=A=240[C240]B=360=B=R
        みちのおく / [C120=0=]A=240A=240[C240#0#0#240F600]R
    # (中略)
    ひとつにつくせ / [F240E=360CC240]A=240B=360=B=240
        くにのため / [C120=0=]A=360=0=F240E=240A=600R
    # (中略)
    ちしまのおくも / E=240A=360A=A=240[C240]B=360=B=R
        おきなわも / [C120=0=]A=240A=240[C240#0#0#240F600]R
    # (中略)
    つとめよわがせ / [F240E=360CC240]A=240B=360=B=240
        つつがなく / [C120=0=]A=360=0=F240E=240A=600R

インストール / [C]B=A[F240]B=R
------------------------------

E#G#B#[E#240]R

※プロジェクトの zip ファイルの方に、最新の exe が置いてあります。(必要に応じて md5 の確認も行ってください。)

コンパイルには [dmc](http://www.digitalmars.com/d/download.html) が必要です。

開発は dmc 8.42n / Windows 8 で動作確認していますが、 Windows XP / Vista / 7 / 8.1 でも動くと思います。

(今は Windows 用のコードしかありませんが、将来別の環境でも作るかも。)

※ cmd.exe を UTF-8 が使える状態で起動してください。参照 ( [コマンドプロンプトでUTF-8を表示](http://nazochu.blogspot.jp/2011/08/blog-post_26.html) )

うたいかた / GGBBA360R
----------------------

E=G=B=[E=240]R

    > pocket39
    [CDEFG
    きょうは、ぽけっとみくのつかいかたをせつめいします。
    ぽけみくは、おおえすのひょうじゅんにゅうりょくを、よみあげます。
    つまりこういうことです。
    ^Z
    > echo ミクダヨー / [CEG[C480]R | pocket39
    > pocket39
    おんていもつけられます。 / CEG[CEG[C]]GC]G[C480R
    うたうこともできます。 / CFAF[CF]B=FE=]B=480[R
    ^Z
    > pocket39 < fuji3.p39

※こういうことはしないでください。

    > pocket39 < README.md

もくひょう / DGCG480R
---------------------

EA[C#E#240]R

- マニュアル作る (楽譜は MML もどきですが今のところ互換性は無いです。検討中。)
- パラメータの最大値最小値チェック
- 初期化時の MIDI Message (テンポ・ボリューム等)
- 開始前の無音期間指定
- コーラスとかハモりとか
- RPN とか CC 送る？ (ミクの声は pitch bend sensitivity のデフォルト値が違う)
- ベロシティも各音または任意の位置で指定出来るようにしたい
- ビブラートとかポルタメントとかかけたい モジュレーションでOK?
- pitch bend の時の前の音との繋がりを考慮して envelope 出力する
- Sleep() でタイミングが適当なので FIFO 経由して timer thread で出力する予定
- note off は毎回ちゃんと送るべき？
- 歌詞データは一音ずつ送るより休止期間中にある程度まとめて送った方が良いらしい
- 子音の発声は他のチャンネルよりもタイミングを早める方が良いらしい
- SMF ファイルの出力 あるいは .SMF <-> .p39 コンバータ造る
- .p39 ファイルにテンポ指定とかその他(発声以外の楽器指定とかコーラスとか)
- 声以外の楽器にも対応したい (ここまでするなら素直に MIDI コントローラで・・・)
- オフセット指定サポートで曲全体のキーを変えられるような・・・
- 歌詞と楽譜を一音ずつ混在する記述「あC240いE120うG60」をサポート予定
- 音の長さを「ー」の数で指定するとか
- あとで com サーバーにしてみる
- VST プラグインにも出来る？
- PC から時刻を拾って時報を 歌わせる うたってもらう
- NSX-39 が接続されていないときの発声(CH1)が単調なので適当なリズム付けたい
- たぶんバグいっぱい

びこうらん / EG240D120=0=360R
-----------------------------

[C]GEC240R

- pocket39.py はテスト用です。参照 ( [OpenMIDI project](http://openmidiproject.sourceforge.jp/) )
- BSD license
