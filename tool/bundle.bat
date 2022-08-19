@echo off

::set datetime_str=%date:~0,4%_%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%
::rem echo %datetime_str%
::
::rem "./svnclear.bat"
::
::echo compressing ../free/free_%datetime_str%.7z
::"7z.exe" a "../free/free_"%datetime_str%".7z" "../free/*" -x!*.7z -bd -bso0
::
::echo compressing ../testCat/bin/bin_%datetime_str%.7z
::"7z.exe" a "../testCat/bin/bin_"%datetime_str%".7z" "../testCat/bin/art" "../testCat/bin/driver" "../testCat/bin/*.dll" -x!*.7z -bd -bso0

"./python/python.exe" "script/bundle.py"

echo completed.
pause
