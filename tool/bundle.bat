rem @echo off

set datetime_str=%date:~0,4%_%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%
echo %datetime_str%

rem "7z.exe" a "../free/free_"%datetime_str%".7z" "../free/*" -x!*.7z
"7z.exe" a "../testCat/bin/bin_"%datetime_str%".7z" "../testCat/bin/art" "../testCat/bin/driver" "../testCat/bin/*.dll" -x!*.7z
