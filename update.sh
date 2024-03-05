# python3 update.py
datedata=`date --iso-8601="minutes"`
git add .
git commit -m "${datedata}"
