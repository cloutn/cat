@echo off

"./python/python" "script/setup.py" build_jolt -arch=64
if errorlevel 1 (
    echo build_jolt failed.
    pause
    exit /b 1
)

echo complete.

pause

