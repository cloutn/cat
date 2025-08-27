#!/bin/bash

echo "Linux Setup Script (Rocky Linux 9.6)"

# 1. 先装并启用 EPEL
sudo dnf install -y epel-release

# 2. 再刷新缓存（可选但推荐）
sudo dnf makecache

# 3. 安装真正的包
sudo dnf install -y cmake p7zip p7zip-plugins python

cd "tool"

python "script/setup_linux.py" all -arch=64

python "script/find_gbk.py" ./

cd ..

echo "Complete."
