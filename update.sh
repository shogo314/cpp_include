python3 update.py
cp -rf ignore/* ou-library
datedata=`date --iso-8601="seconds"`
git add .
git commit -m "${datedata}"
git push
