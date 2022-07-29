import os
import shutil
import io
from mytool import clear_dir, cmd
from datetime import datetime


#clear_dir(
#    "d:/cat/free/",
#    ["Debug", "release", "x64", "make", "ipch"],
#    [".suo", ".sdf", ".pdb", ".ilk", ".user"])


print("aabb")
datetime_str = datetime.now().strftime("%Y_%m%d_%H%M%S")

print("compressing ../free/free_%s.7z" % datetime_str)
#"7z.exe" a "../free/free_"%datetime_str%".7z" "../free/*" -x!*.7z -bd -bso0
cmd(['./7z.exe', 'a', "../free/free_%s.7z" % datetime_str, "../free/*", "-x!*.7z", "-x!lib", "-x!shaderc/build"])
#
print("compressing ../testCat/bin/bin_%s.7z" % datetime_str)
##"7z.exe" a "../testCat/bin/bin_"%datetime_str%".7z" "../testCat/bin/art" "../testCat/bin/driver" "../testCat/bin/*.dll" -x!*.7z -bd -bso0
cmd(['./7z.exe', 'a', "../testCat/bin/bin_%s.7z" % datetime_str, "../testCat/bin/art", "../testCat/bin/driver", "../testCat/bin/*.dll", "-x!*.7z"])

