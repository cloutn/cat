::@echo off

::@echo uncrompressing free/free.7z
::"tool/7z.exe" x "free/free.7z" -o"free/" -aoa -bso0 -bd
::
::@echo uncrompressing testCat/bin/bin.7z
::"tool/7z.exe" x "testCat/bin/bin.7z" -o"testCat/bin" -aoa -bso0 -bd

cd "tool"

"./7z.exe" x "../archive/python.7z" "-o../tool/python" -aoa -bso0 -bd

"./python/python" "script/setup.py"

echo complete.

pause
