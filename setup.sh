#!/bin/bash

echo "Linux Setup Script (Rocky Linux 9.6)"

# 安装依赖
sudo dnf install -y epel-release cmake p7zip p7zip-plugins

cd "tool"

# 解压并运行
7z x "../archive/python.7z" "-o../tool/python" -aoa -bso0 -bd
chmod +x "./python/python"
"./python/python" "script/setup_linux.py" all -arch=64
"./python/python" "script/find_gbk.py" ./

cd ..

echo "Complete."
