rem @echo off
rem !!!Only for Windows!!!!!!

set del_dir=../free
set svnclear_dir=../tool

cd %del_dir%
del /S *.sdf
del /S *.ncb
del /S *.pdb
rem del /S *.ilk
rem del /S *.user
del /S /AH *.suo
rem del /S *~
rd /S /Q "lib" || rem
echo %errorlevel%

rd /S /Q "shaderc/build" || rem

echo %errorlevel%
if %errorlevel% NEQ 0 goto ERROR
rem for /r %del_dir% %a in (debug\) do @if exist "%a" rd /s/q "%a"


"%svnclear_dir%/svnclear.exe" "%del_dir%" debug
"%svnclear_dir%/svnclear.exe" "%del_dir%" release
"%svnclear_dir%/svnclear.exe" "%del_dir%" x64
"%svnclear_dir%/svnclear.exe" "%del_dir%" make
"%svnclear_dir%/svnclear.exe" "%del_dir%" ipch

goto SUCCESS

:ERROR
echo failed.
echo %errorlevel%

:SUCCESS
echo complete.

pause
