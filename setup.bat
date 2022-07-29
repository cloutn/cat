::@echo off

::@echo uncrompressing free/free.7z
::"tool/7z.exe" x "free/free.7z" -o"free/" -aoa -bso0 -bd
::
::@echo uncrompressing testCat/bin/bin.7z
::"tool/7z.exe" x "testCat/bin/bin.7z" -o"testCat/bin" -aoa -bso0 -bd

cd tool
python setup.py
::echo completed.
pause
