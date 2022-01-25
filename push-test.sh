git config --global -l

#设置代理
git config --global http.proxy http://127.0.0.1:1080
git config --global https.proxy http://127.0.0.1:1080

git config --global -l

#取消代理：
git config --global --unset http.proxy
git config --global --unset https.proxy

git config --global -l

git push -u origin master -4
