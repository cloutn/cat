import sys
sys.path.append("./script")
#import os
#print(os.path.abspath("./"))

import subprocess
import os
from mytool import cmd

#	"-aoa"  	# ask overwrite mode = all
#	"-bso0"  	# stadard output stream disabled
#	"-bd"		# disable progress indicator

def unzip_cmake():
    cmd(['./7z/7z.exe', 'x', '../archive/cmake.7z', '-o../tool/cmake', '-aoa', '-bso0', '-bd'])

def unzip_free():
    cmd(['./7z/7z.exe', 'x', '../archive/free.7z', '-o../free/', '-aoa', '-bso0', '-bd'])

def unzip_bin():
    cmd(['./7z/7z.exe', 'x', '../archive/bin.7z', '-o../testCat/bin', '-aoa', '-bso0', '-bd'])

def unzip_bin64():
    cmd(['./7z/7z.exe', 'x', '../archive/bin64.7z', '-o../testCat/bin64', '-aoa', '-bso0', '-bd'])

def unzip_art():
    cmd(['./7z/7z.exe', 'x', '../archive/art.7z', '-o../testCat/art', '-aoa', '-bso0', '-bd'])

def unzip_all():
    funcs = []
    funcs.extend(globals())
    for f in funcs:
        if not f.startswith("unzip_") or f == "unzip_all":
            continue
        globals()[f]()

def build_shaderc():
    src_path = "../free/shaderc/"
    build_path = "../free/shaderc/build64/"
    if not os.path.exists(build_path):
        os.mkdir(build_path)
    cmd(['./cmake/bin/cmake.exe', "-G", "Visual Studio 17 2022", "-DSHADERC_ENABLE_SHARED_CRT:INT=1", "-DSHADERC_SKIP_TESTS=ON", "-DSHADERC_SKIP_INSTALL=ON", "-DPYTHON_EXECUTABLE=./python/python.exe", "-Wno-dev", "-S", src_path, "-B", build_path])
    #cmd(['./cmake/bin/cmake.exe', "--version"])
    cmd(['./cmake/bin/cmake.exe', "--build", build_path, "--config", "Debug"])

def build_all():
    funcs = []
    funcs.extend(globals())
    for f in funcs:
        if not f.startswith("build_") or f == "build_all":
            continue
        globals()[f]()

def all():
    unzip_all()
    build_all()

all_funcs = []
all_funcs.extend(locals())

argc = len(sys.argv)

if argc >= 2:
    op = sys.argv[1]
    if op in all_funcs:
        #print("call function %s" % op)
        locals()[op]()
    else:
        print("operation %s not found." % op)
    
