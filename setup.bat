@echo off

cd "tool"

"./7z/7z.exe" x "../archive/python.7z" "-o../tool/python" -aoa -bso0 -bd

::genereate x64
"./python/python" "script/setup.py" all64

::genereate win32
::"./python/python" "script/setup.py" all32

cd ..

echo complete.

pause
