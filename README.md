# 説明
shogo314が使っているライブラリです。[atcoder/ac-library](https://github.com/atcoder/ac-library)と[rainbou-kpr/library](https://github.com/rainbou-kpr/library)と[shogo314/sh-library](https://github.com/shogo314/sh-library)を使っています。

`update.py`を実行すると`ou-library`と`sh-library`を最新版にできます。このとき`./tmp`が消されることに注意してください。

このフォルダをホームディレクトリに置いて、
`.bash_profile`に
```bash
export CPLUS_INCLUDE_PATH="$HOME/cpp_include"

```

と書いています。

# その他外部ライブラリ
## boost1.87.0
```
cd tmp
wget -O https://archives.boost.io/release/1.87.0/source/boost_1_87_0.tar.gz
```
