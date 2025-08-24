import sys
sys.path.append("./script")

import os
import shutil
import io
from mytool import clear_dir, cmd
from datetime import datetime

datetime_str = datetime.now().strftime("%Y_%m%d_%H%M%S")

dir = "../archive/%s/" % datetime_str

os.makedirs(dir, exist_ok=True)

# 使用Linux系统的7z命令而不是Windows的7z.exe
print("compressing %sfree.7z" % dir)
cmd([
    '7z',  # Linux系统命令
    'a',
    dir + "free.7z" ,
    "../free/cgltf",
    "../free/gles", 
    "../free/jpeg_turbo",
    "../free/libktx",
    "../free/libpng",
    "../free/libtga",
    "../free/shaderc",
    "../free/spirv_cross",
    "../free/vld_runtime",
    "../free/zlib",
    "../free/vulkan",
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
    "-xr!.vs",         # Visual Studio目录
    "-xr!build",
    "-xr!build32",
    "-xr!build64",
    "-xr!build64_linux",  # Linux构建目录
    "-x!lib",
    "-x!lib64",
    "-x!shaderc/build",
    "-aoa",     # ask overwrite mode = all
    "-bso0",    # standard output stream disabled
    "-bd",      # disable progress indicator
    ])

print("compressing %sbin.7z" % dir)
# Linux下可能没有.dll文件，改为查找.so或其他文件
cmd([
    '7z',
    'a',
    dir + "/bin.7z",
    "../testCat/bin/driver", 
    "../testCat/bin/*.so",    # Linux共享库
    "../testCat/bin/*.dll",   # Windows DLL (如果存在)
    "-x!*.7z",
    "-aoa",
    "-bso0", 
    "-bd",
    ])

print("compressing %sbin64.7z" % dir)
cmd([
    '7z',
    'a',
    dir + "bin64.7z",
    "../testCat/bin64/driver",
    "../testCat/bin64/*.so",   # Linux共享库
    "../testCat/bin64/*.dll",  # Windows DLL (如果存在)
    "-x!*.7z",
    "-aoa",
    "-bso0",
    "-bd",
    ])

print("compressing %sart.7z" % dir)
cmd([
    '7z',
    'a', 
    dir + "art.7z",
    "../testCat/art/*",
    "-x!*.7z",
    "-aoa",
    "-bso0",
    "-bd",
    ])

print("Bundle complete. Archive created in: %s" % dir)
