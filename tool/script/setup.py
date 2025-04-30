import sys
sys.path.append("./script")
#import os
#print(os.path.abspath("./"))

import subprocess
import os
import shutil
from mytool import cmd
import argparse

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


def generate_testCat():
    arch = "64" if G.arch64 else ""
    src_path = "../testCat/"
    build_path = f"../testCat/build{arch}/"
    cmake_arch = "x64" if G.arch64 else "Win32"
    cmd(['./cmake/bin/cmake.exe', "-G", "Visual Studio 17 2022", "-A", cmake_arch, "-Wno-dev", "-S", src_path, "-B", build_path])

def generate_tests():
    arch = "64" if G.arch64 else ""
    cmake_arch = "x64" if G.arch64 else "Win32"

    filelist = os.listdir("../test")
    for filename in filelist:
        print("generating : %s" % (filename))
        #filepath = os.path.join(path, filename)
        src_path = "../test/" + filename
        build_path = f"../test/{filename}/build{arch}/"
        cmd(['./cmake/bin/cmake.exe', "-G", "Visual Studio 17 2022", "-A", cmake_arch, "-Wno-dev", "-S", src_path, "-B", build_path])
    #src_path = "../testMath/"
    #build_path = f"../testCat/build{arch}/"
    #cmake_arch = "x64" if G.arch64 else "Win32"
    #cmd(['./cmake/bin/cmake.exe', "-G", "Visual Studio 17 2022", "-A", cmake_arch, "-Wno-dev", "-S", src_path, "-B", build_path])

def generate_test1():
    print("generate_test1")

####################################################
# composite functions for convenient
####################################################
def all():
    unzip_all()
    build_all()
    generate_testCat()

def build_all():
    funcs = []
    funcs.extend(globals())
    for f in funcs:
        if not f.startswith("build_") or f == "build_all":
            continue
        globals()[f]()

def unzip_all():
    funcs = []
    funcs.extend(globals())
    for f in funcs:
        if not f.startswith("unzip_") or f == "unzip_all":
            continue
        globals()[f]()



all_locals = []
all_locals.extend(locals())

all_funcs = []
for func in all_locals:
    is_add = False
    if func.startswith("unzip_"):
        is_add = True
    elif func.startswith("build_"):
        is_add = True
    elif func.startswith("generate_"):
        is_add = True
    elif func == "all":
        is_add = True

    if is_add:
       all_funcs.append(func)


parser = argparse.ArgumentParser(description = "Excute functions in tool script.\nIf you want to build Nanolit fast, use")
parser.add_argument('operation', help=f'Execute operation function in setup.py. All functions = {all_funcs}')
parser.add_argument("-arch", default=64, type=int, choices=[32, 64], help='Architecture is arm64 or x86-32, -a=32 means win32, -a=64 means x64')
args = parser.parse_args()


op = args.operation
G.arch64 = (args.arch==64)
print("operation function = %s, arch64 = %r" % (op, G.arch64))
if op in all_funcs:
    locals()[op]()
else:
    print("operation %s not found." % op)

#all_funcs = []
#all_funcs.extend(locals())
#argc = len(sys.argv)
#str_all_functions = ""
#for f in all_funcs:
#    if f.startswith("__"):
#        all_funcs.remove(f)
#if argc >= 2:
#    op = sys.argv[1]
#    if op in all_funcs:
#        locals()[op]()
#    else:
#        print("operation %s not found." % op)
