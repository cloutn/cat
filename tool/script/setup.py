import sys
sys.path.append("./script")
#import os
#print(os.path.abspath("./"))

import subprocess
import os
import shutil
from mytool import cmd

class G:
    arch64 = False


def unzip_cmake():
    # "-aoa"  # ask overwrite mode = all #   "-bso0"  # stadard output stream disabled #   "-bd"   # disable progress indicator
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
    arch = "64" if G.arch64 else ""
    src_path = "../free/shaderc/"
    build_path = f"../free/shaderc/build{arch}/"
    if not os.path.exists(build_path):
        os.mkdir(build_path)
    cmake_arch = "x64" if G.arch64 else "Win32"
    cmd(['./cmake/bin/cmake.exe', "-G", "Visual Studio 17 2022", "-A", cmake_arch, "-DSHADERC_ENABLE_SHARED_CRT:INT=1", "-DSHADERC_SKIP_TESTS=ON", "-DSHADERC_SKIP_INSTALL=ON", "-DPYTHON_EXECUTABLE=./python/python.exe", "-Wno-dev", "-S", src_path, "-B", build_path])
    print(['./cmake/bin/cmake.exe', "-G", "Visual Studio 17 2022", "-A", cmake_arch, "-DSHADERC_ENABLE_SHARED_CRT:INT=1", "-DSHADERC_SKIP_TESTS=ON", "-DSHADERC_SKIP_INSTALL=ON", "-DPYTHON_EXECUTABLE=./python/python.exe", "-Wno-dev", "-S", src_path, "-B", build_path])

    cmd(['./cmake/bin/cmake.exe', "--build", build_path, "--config", "Debug"])
    cmd(['./cmake/bin/cmake.exe', "--build", build_path, "--config", "Release"])

    lib_path = f"../free/lib{arch}/"
    if not os.path.exists(lib_path):
        os.mkdir(lib_path)
    shutil.copy(build_path + "/libshaderc/Debug/shaderc_combined.lib", f"../free/lib{arch}/shaderc_combined_d.lib")
    shutil.copy(build_path + "/libshaderc/Release/shaderc_combined.lib", f"../free/lib{arch}/shaderc_combined.lib")

def build_all():
    funcs = []
    funcs.extend(globals())
    for f in funcs:
        if not f.startswith("build_") or f == "build_all":
            continue
        globals()[f]()

def generate_testCat():
    arch = "64" if G.arch64 else ""
    src_path = "../testCat/"
    build_path = f"../testCat/build{arch}/"
    cmake_arch = "x64" if G.arch64 else "Win32"
    cmd(['./cmake/bin/cmake.exe', "-G", "Visual Studio 17 2022", "-A", cmake_arch, "-Wno-dev", "-S", src_path, "-B", build_path])

def all():
    G.arch64 = True
    unzip_all()
    build_all()
    generate_testCat()

def all32():
    G.arch64 = False
    unzip_all()
    build_all()
    generate_testCat()

all_funcs = []
all_funcs.extend(locals())

argc = len(sys.argv)

if argc >= 2:
    op = sys.argv[1]
    if op in all_funcs:
        locals()[op]()
    else:
        print("operation %s not found." % op)


