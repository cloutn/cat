@echo off

cd "tool"

"./7z/7z.exe" x "../archive/python.7z" "-o../tool/python" -aoa -bso0 -bd

"./python/python" "script/setup.py" all

cd ..

echo complete.

pause
