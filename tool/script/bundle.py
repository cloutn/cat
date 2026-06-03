import sys
sys.path.append("./script")

import os
import shutil
import io
from mytool import clear_dir, exec_cmd
from datetime import datetime
from pathlib import Path

datetime_str = datetime.now().strftime("%Y_%m%d_%H%M%S")

dir = "../archive/%s/" % datetime_str

os.mkdir(dir)

extra_path = Path("./7z").resolve()
print(extra_path)
os.environ['PATH'] = f"{extra_path}{os.pathsep}{os.environ['PATH']}"

# free.7z 的语义：仅打包【未入 git】的第三方依赖，给别人重建仓库时一并解出。
# 列表与 .gitignore 中 /free/* 的忽略项一一对齐：
#   .gitignore 已忽略 → 这里必须打包  (cgltf / gles / jpeg_turbo / libktx /
#                                    libpng / libtga / shaderc / spirv_cross /
#                                    vld_runtime / zlib / vulkan)
#   .gitignore 未忽略 → 已入库, 不需要打包 (jolt / glm / imgui / libimg /
#                                         rapidyaml / scl)
# 新增 free/<dep> 时, 务必先确认 .gitignore 是否忽略, 再决定是否加入这里。
exec_cmd([
    '7z',
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
    "-xr!.vs",
    "-xr!build",
    "-xr!build32",
    "-xr!build64",
    "-xr!build64_visualstudio",
    "-xr!build64_ninja",
    "-xr!build64_linux",
    "-xr!CMakeFiles",
    "-xr!xcuserdata",
    "-x!lib",
    "-x!lib64",
    "-x!shaderc/build",
    "-xr!VkLayer_utils.lib",
    "-aoa",     # ask overwrite mode = all
    "-bso0",    # stadard output stream disabled
    "-bd",      # disable progress indicator
    ])


#print("compressing bin.7z" % dir)
exec_cmd([
    '7z',
    'a',
    dir + "/bin.7z",
    "../testCat/bin/driver",
    "../testCat/bin/*.dll",
    "-x!*.7z",
    "-aoa",
    "-bso0",
    "-bd",
    ])


#print("compressing bin64.7z" % dir)
exec_cmd([
    '7z',
    'a',
    dir + "bin64.7z",
    "../testCat/bin64/driver",
    "../testCat/bin64/*.dll",
    "-x!*.7z",
    "-aoa",
    "-bso0",
    "-bd",
    ])

#print("compressing art.7z" % dir)
exec_cmd([
    '7z',
    'a',
    dir + "art.7z",
    "../testCat/art/*",
    "-x!*.7z",
    "-aoa",
    "-bso0",
    "-bd",
    ])

#print("compressing tool_bin.7z" % dir)
exec_cmd([
    '7z',
    'a',
    dir + "tool_bin.7z",
    "../tool/bin/*",
    "-x!*.7z",
    "-aoa",
    "-bso0",
    "-bd",
    ])

