import subprocess
import os
import re
import shutil
import requests
import zipfile


def delete(s: str):
    if os.path.isfile(s):
        os.remove(s)
    if os.path.isdir(s):
        shutil.rmtree(s)


def reset_dir(s: str):
    delete(s)
    os.mkdir(s)


def insert_github_link(root: str, url_prefix: str):
    """root 以下の各ファイルに GitHub へのリンクを挿入する

    oj-bundle で展開したとき、リンクの直後がそのファイル自身のコードになるように、
    最後の #include "..." の直後(なければ #pragma once の直後、それもなければ先頭)に挿入する。
    """
    for dirpath, _, filenames in os.walk(root):
        for filename in filenames:
            path = os.path.join(dirpath, filename)
            rel = os.path.relpath(path, root).replace(os.sep, "/")
            with open(path, encoding="utf-8") as f:
                lines = f.readlines()
            link = f"// {url_prefix}/{rel}\n"
            pos = 0
            for i, line in enumerate(lines):
                if line.strip() == "#pragma once" or re.match(r'\s*#\s*include\s*"', line):
                    pos = i + 1
            lines.insert(pos, link)
            with open(path, mode="w", encoding="utf-8") as f:
                f.writelines(lines)


def download_atcoder():
    reset_dir("./tmp")
    url = "https://github.com/atcoder/ac-library/archive/refs/heads/master.zip"
    zipfilename = "tmp/master.zip"
    urlData = requests.get(url).content
    with open(zipfilename, mode="wb") as f:  # wb でバイト型を書き込める
        f.write(urlData)
    subprocess.run(["unzip", "-d", "./tmp/", zipfilename])
    delete("./atcoder")
    subprocess.run(["mv", "./tmp/ac-library-master/atcoder", "./atcoder"])
    insert_github_link(
        "./atcoder", "https://github.com/atcoder/ac-library/blob/master/atcoder")


def download_shogo_library():
    reset_dir("./tmp")
    url = "https://github.com/shogo314/shogo-library/archive/refs/heads/main.zip"
    zipfilename = "tmp/main.zip"
    urlData = requests.get(url).content
    with open(zipfilename, mode="wb") as f:  # wb でバイト型を書き込める
        f.write(urlData)
    subprocess.run(["unzip", "-d", "./tmp/", zipfilename])
    delete("./shogo-library")
    subprocess.run(["mv", "./tmp/shogo-library-main/src", "./shogo-library"])
    insert_github_link(
        "./shogo-library", "https://github.com/shogo314/shogo-library/blob/main/src")


def main():
    # download_atcoder()
    download_shogo_library()
    delete("./tmp")


if __name__ == "__main__":
    main()
