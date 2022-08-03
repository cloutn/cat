import subprocess
import os
from mytool import cmd



print(os.getcwd())

#	"-aoa",  	# ask overwrite mode = all
#	"-bso0",  	# stadard output stream disabled
#	"-bd"])		# disable progress indicator

#print("uncrompressing archive/free.7z")
cmd(['./bin/7z.exe', 'x', '../archive/free.7z', '-o../free/', '-aoa', '-bso0', '-bd'])

#print("uncrompressing testCat/bin/bin.7z")
cmd(['./bin/7z.exe', 'x', '../archive/bin.7z', '-o../testCat/bin', '-aoa', '-bso0', '-bd'])

#print("uncrompressing testCat/bin64/bin64.7z")
cmd(['./bin/7z.exe', 'x', '../archive/bin64.7z', '-o../testCat/bin64', '-aoa', '-bso0', '-bd'])

#print("uncrompressing testCat/art/art.7z")
cmd(['./bin/7z.exe', 'x', '../archive/art.7z', '-o../testCat/art', '-aoa', '-bso0', '-bd'])
