python3 update.py
datedata=`date --iso-8601="seconds"`
git add .
git commit -m "${datedata}"
git push
