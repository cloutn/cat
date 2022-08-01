import os
import shutil
import io
from mytool import clear_dir, cmd
from datetime import datetime

datetime_str = datetime.now().strftime("%Y_%m%d_%H%M%S")

print("compressing ../free/free_%s.7z" % datetime_str)
cmd([
    'bin/7z.exe',
    'a',
    "../free/free_%s.7z" % datetime_str,
    "../free/*",
    "-x!*.7z",
    "-x!*.suo",
    "-x!*.sdf",
    "-x!*.pdb",
    "-x!*.ilk",
    "-x!*.user",
    "-xr!Debug",
    "-xr!Release",
    "-xr!x64",
    "-xr!make",
    "-xr!ipch",
    "-xr!.vs",
    "-xr!build",
    "-x!lib",
    "-x!lib64",
    "-x!shaderc/build",
    "-aoa",     # ask overwrite mode = all
    "-bso0",    # stadard output stream disabled
    "-bd",      # disable progress indicator
    ])


print("compressing ../../testCat/bin/bin_%s.7z" % datetime_str)
cmd([
    'bin/7z.exe',
    'a',
    "../testCat/bin/bin_%s.7z" % datetime_str,
    "../testCat/bin/art",
    "../testCat/bin/driver",
    "../testCat/bin/*.dll",
    "-x!*.7z",
    "-aoa",
    "-bso0",
    "-bd",
    ])


