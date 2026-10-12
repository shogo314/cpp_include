import subprocess
import os
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


def main():
    # download_atcoder()
    download_shogo_library()
    delete("./tmp")


if __name__ == "__main__":
    main()
