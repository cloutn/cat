@echo off

"./python/python" "script/setup.py" generate_all -g=ninja -arch=64

"./python/python" "script/merge_compile_commands.py" "../"

echo complete.

pause

