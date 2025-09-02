import sys
sys.path.append("./script")

import subprocess
import os
import shutil
from mytool import cmd
import argparse

class G:
    arch64 = False
    generator = ""
    build_suffix = ""
    arch_param = ""
    vcvars = ""
    call_vcvars = ""

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

def unzip_tool_bin():
    cmd(['./7z/7z.exe', 'x', '../archive/tool_bin.7z', '-o../tool/bin', '-aoa', '-bso0', '-bd'])


def build_shaderc():
    arch = "64" if G.arch64 else ""
    src_path = "../free/shaderc/"
    build_path = f"../free/shaderc/build{arch}_{G.buld_suffix}/"
    if not os.path.exists(build_path):
        os.mkdir(build_path)
    cmd(['./cmake/bin/cmake.exe', "-G", G.generator, G.arch_param, "-DSHADERC_ENABLE_SHARED_CRT:INT=1", "-DSHADERC_SKIP_TESTS=ON", "-DSHADERC_SKIP_INSTALL=ON", "-DPYTHON_EXECUTABLE=./python/python.exe", "-Wno-dev", "-S", src_path, "-B", build_path])

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
    build_path = f"../testCat/build{arch}_{G.build_suffix}/"
    cmd(['./cmake/bin/cmake.exe', "-G", G.generator, G.arch_param, "-Wno-dev", "-S", src_path, "-B", build_path])

def generate_tests():
    arch = "64" if G.arch64 else ""

    filelist = os.listdir("../test")
    for filename in filelist:
        print("generating : %s" % (filename))
        src_path = "../test/" + filename
        build_path = f"../test/{filename}/build{arch}_{G.build_suffix}/"

        cmd_str = f'{G.call_vcvars} .\\cmake\\bin\\cmake.exe -G {G.generator} {G.arch_param} -Wno-dev -S {src_path}, -B {build_path}'
        cmd(cmd_str, shell=True)
        #cmd([G.call_vcvars, '.\\cmake\\bin\\cmake.exe', "-G", G.generator, G.arch_param, "-Wno-dev", "-S", src_path, "-B", build_path], shell=True)
        #if G.build_suffix == "ninja":
        #    vcvars = find_vcvars()
        #    #cmd(vcvars)
        #    mycmd = f'call "{vcvars}" && ' \
        #          f'.\\cmake\\bin\\cmake.exe -G {G.generator} {G.arch_param} -Wno-dev -S "{src_path}" -B {build_path}'

        #    cmd(mycmd, shell=True)

        #    #cmd(["call", vcvars, "&&", './cmake/bin/cmake.exe', "-G", G.generator, G.arch_param, "-Wno-dev", "-S", src_path, "-B", build_path])
        #    cmd(['./cmake/bin/cmake.exe', "-G", G.generator, G.arch_param, "-Wno-dev", "-S", src_path, "-B", build_path])
        #else:
        #    cmd(['./cmake/bin/cmake.exe', "-G", G.generator, G.arch_param, "-Wno-dev", "-S", src_path, "-B", build_path])

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


def find_vcvars():
    cmd = ["./bin/vswhere.exe", "-latest", "-products", "*", "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-find", "VC/Auxiliary/Build/vcvars64.bat"]
    #cmd = ["./bin/vswhere.exe", "-latest", "-products", "*", "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-find", "Common7\Tools\VsDevCmd.bat"]
    out = subprocess.check_output(cmd, text=True).strip()
    return out if out else None

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
parser.add_argument("-generator", "-g", default="vs", choices=["vs", "ninja"], help='Build generator: vs=Visual Studio 2022, ninja=Ninja')
args = parser.parse_args()


op = args.operation
G.arch64 = (args.arch==64)
if args.generator == "vs":
    G.generator = "Visual Studio 17 2022"
    G.build_suffix = "visualstudio"
    G.arch_param = "-A " + ("x64" if G.arch64 else "Win32")
elif args.generator == "ninja":
    G.vcvars = find_vcvars()
    print(f"vcvars = {G.vcvars}")
    G.call_vcvars = f'call "{G.vcvars}" &&';
    G.generator = "Ninja"
    G.build_suffix = "ninja"

print("operation function = %s, arch64 = %r" % (op, G.arch64))
if op in all_funcs:
    locals()[op]()
else:
    print("operation %s not found." % op)

