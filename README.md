# 説明
shogo314が競技プログラミングで使っているヘッダファイル置き場です。

| ディレクトリ | 取得元 | 備考 |
| --- | --- | --- |
| `atcoder/` | [atcoder/ac-library](https://github.com/atcoder/ac-library) | 自動更新は無効化中(`update.py`でコメントアウト) |
| `shogo-library/` | [shogo314/shogo-library](https://github.com/shogo314/shogo-library) の `src/` | |
| `boost/` | [Boost](https://www.boost.org/) のヘッダ | git 管理外。各自で配置(下記) |

以前は`ou-library`と`sh-library`も置いていましたが、内容は`shogo-library`に統合されたため廃止しました。

# セットアップ
このフォルダをホームディレクトリに置いて、`.bash_profile`に

```bash
export CPLUS_INCLUDE_PATH="$HOME/cpp_include"
```

と書いています。これで次のようにインクルードできます。

```cpp
#include <atcoder/all>
#include <shogo-library/base/all>
#include <shogo-library/data_structures/segtree.hpp>
#include <boost/multiprecision/cpp_int.hpp>
```

# 更新
- `python3 update.py`:`shogo-library`をリポジトリの最新版で置き換えます。
  - `requests`(Python パッケージ)と`unzip`コマンドが必要です。
  - 作業用に`./tmp`を作り直し、最後に削除するので、`./tmp`に置いたものは消えます。
  - `atcoder`も更新したい場合は`main()`の`download_atcoder()`のコメントアウトを外してください。
- `bash update.sh`:`update.py`を実行したあと、変更をすべて日時をメッセージにしてコミットし、push します。

# Boost の配置
Boost はサイズが大きいので git では管理していません(`.gitignore`済み)。clone した後やバージョンを上げるときは、以下のようにして`boost/`を配置します。

```bash
mkdir -p tmp && cd tmp
wget https://archives.boost.io/release/1.87.0/source/boost_1_87_0.tar.gz
tar xzf boost_1_87_0.tar.gz
rm -rf ../boost && mv boost_1_87_0/boost ../boost
cd .. && rm -rf tmp
```
