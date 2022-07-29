import subprocess
import os
from mytool import cmd



#print(os.getcwd())

print("uncrompressing free/free.7z")
cmd(['./7z.exe', 'x', '../free/free.7z', '-o../free/', '-aoa', '-bso0', '-bd'])

print("uncrompressing testCat/bin/bin.7z")
cmd(['./7z.exe', 'x', '../testCat/bin/bin.7z', '-o../testCat/bin', '-aoa', '-bso0', '-bd'])

