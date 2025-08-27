import sys
sys.path.append("./script")

import subprocess
import os
import shutil
from mytool import cmd
import argparse

class G:
    arch64 = False


def unzip_cmake():
    # Linux下使用系统安装的cmake，不需要解压
    print("Using system cmake (no extraction needed)")

def unzip_free():
    cmd(['7z', 'x', '../archive/free.7z', '-o../free/', '-aoa', '-bso0', '-bd'])

def unzip_bin():
    cmd(['7z', 'x', '../archive/bin.7z', '-o../testCat/bin', '-aoa', '-bso0', '-bd'])

def unzip_bin64():
    cmd(['7z', 'x', '../archive/bin64.7z', '-o../testCat/bin64', '-aoa', '-bso0', '-bd'])

def unzip_art():
    cmd(['7z', 'x', '../archive/art.7z', '-o../testCat/art', '-aoa', '-bso0', '-bd'])


def build_shaderc():
    arch = "64" if G.arch64 else ""
    src_path = "../free/shaderc/"
    build_path = f"../free/shaderc/build{arch}_linux/"
    
    if not os.path.exists(build_path):
        os.makedirs(build_path, exist_ok=True)
    
    # Linux下使用Unix Makefiles生成器，不需要指定架构
    cmake_cmd = [
        'cmake', 
        "-G", "Unix Makefiles",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DSHADERC_SKIP_TESTS=ON", 
        "-DSHADERC_SKIP_INSTALL=ON", 
        "-DPYTHON_EXECUTABLE=python3",
        "-Wno-dev", 
        "-S", src_path, 
        "-B", build_path
    ]
    
    cmd(cmake_cmd)
    print(f"CMake command: {' '.join(cmake_cmd)}")

    # 构建Debug和Release版本
    cmd(['cmake', "--build", build_path, "--config", "Debug"])
    cmd(['cmake', "--build", build_path, "--config", "Release"])

    # 创建lib目录
    lib_path = f"../free/lib{arch}/"
    if not os.path.exists(lib_path):
        os.makedirs(lib_path, exist_ok=True)
    
    # 复制生成的静态库文件（Linux下是.a文件）
    shaderc_lib_debug = os.path.join(build_path, "libshaderc", "libshaderc_combined.a")
    shaderc_lib_release = os.path.join(build_path, "libshaderc", "libshaderc_combined.a")
    
    if os.path.exists(shaderc_lib_debug):
        shutil.copy(shaderc_lib_debug, f"../free/lib{arch}/libshaderc_combined.a")
    if os.path.exists(shaderc_lib_release):
        shutil.copy(shaderc_lib_release, f"../free/lib{arch}/libshaderc_combined.a")


def generate_testCat():
    arch = "64" if G.arch64 else ""
    src_path = "../testCat/"
    build_path = f"../testCat/build{arch}_linux/"
    
    if not os.path.exists(build_path):
        os.makedirs(build_path, exist_ok=True)
    
    cmake_cmd = [
        'cmake', 
        "-G", "Unix Makefiles",
        "-DCMAKE_BUILD_TYPE=Release",
        "-Wno-dev", 
        "-S", src_path, 
        "-B", build_path
    ]
    cmd(cmake_cmd)

def generate_tests():
    arch = "64" if G.arch64 else ""

    filelist = os.listdir("../test")
    for filename in filelist:
        test_dir = f"../test/{filename}"
        if not os.path.isdir(test_dir):
            continue
        
        cmake_file = os.path.join(test_dir, "CMakeLists.txt")
        if not os.path.exists(cmake_file):
            continue
            
        print(f"generating : {filename}")
        
        src_path = f"../test/{filename}"
        build_path = f"../test/{filename}/build{arch}_linux/"
        
        if not os.path.exists(build_path):
            os.makedirs(build_path, exist_ok=True)
        
        cmake_cmd = [
            'cmake', 
            "-G", "Unix Makefiles",
            "-DCMAKE_BUILD_TYPE=Release",
            "-Wno-dev", 
            "-S", src_path, 
            "-B", build_path
        ]
        cmd(cmake_cmd)

def generate_test1():
    print("generate_test1")

def build_scl():
    """构建scl库"""
    arch = "64" if G.arch64 else ""
    src_path = "../free/scl/"
    build_path = f"../free/scl/build{arch}_linux/"
    
    if not os.path.exists(build_path):
        os.makedirs(build_path, exist_ok=True)
    
    cmake_cmd = [
        'cmake', 
        "-G", "Unix Makefiles",
        "-DCMAKE_BUILD_TYPE=Release",
        "-Wno-dev", 
        "-S", src_path, 
        "-B", build_path
    ]
    cmd(cmake_cmd)
    
    # 构建库
    cmd(['cmake', "--build", build_path])

def build_all_libs():
    """构建所有必需的库"""
    build_scl()
    # 可以添加其他库的构建

####################################################
# composite functions for convenient
####################################################
def all():
    unzip_all()
    build_all()
    build_all_libs()  # 构建必需的库
    generate_tests()
    generate_testCat()

def build_all():
    funcs = []
    funcs.extend(globals())
    for f in funcs:
        if not f.startswith("build_") or f in ["build_all", "build_all_libs"]:
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


parser = argparse.ArgumentParser(description = "Execute functions in Linux tool script.\nIf you want to build project fast, use 'all'")
parser.add_argument('operation', help=f'Execute operation function in setup_linux.py. All functions = {all_funcs}')
parser.add_argument("-arch", default=64, type=int, choices=[32, 64], help='Architecture: -arch=32 means 32-bit, -arch=64 means 64-bit (default)')
args = parser.parse_args()


op = args.operation
G.arch64 = (args.arch==64)
print("operation function = %s, arch64 = %r" % (op, G.arch64))

# 检查必要的命令是否存在
required_commands = ['cmake', '7z']
missing_commands = []

for cmd_name in required_commands:
    if not shutil.which(cmd_name):
        missing_commands.append(cmd_name)

if missing_commands:
    print(f"Error: Missing required commands: {', '.join(missing_commands)}")
    print("Please install them using:")
    print("sudo dnf install epel-release")
    print("sudo dnf install cmake p7zip p7zip-plugins python")
    sys.exit(1)

if op in all_funcs:
    locals()[op]()
else:
    print("operation %s not found." % op)
    print("Available operations:", all_funcs)
