@echo off

"./python/python" "script/setup.py" generate_tests -g=ninja -arch=64

echo complete.

pause

