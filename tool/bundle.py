import os
import shutil
import io
from mytool import clear_dir, cmd
from datetime import datetime

datetime_str = datetime.now().strftime("%Y_%m%d_%H%M%S")

#clear_dir(
#    "d:/cat/free/",
#    ["Debug", "release", "x64", "make", "ipch"],
#    [".suo", ".sdf", ".pdb", ".ilk", ".user"])

#exclude_exts = [".suo", ".sdf", ".pdb", ".ilk", ".user", ".7z", ""]
#exclude_dirs = ["lib", "shaderc/build"]
#
#exclude_exts_param = []
#for ext in exclude_exts:
#	exclude_exts_param.append("-xr!*" + ext)
#
#exclude_dirs_param = []
#for dir in exclude_dirs:
#	exclude_dirs_param.append("-x!" + dir)
#
#
##param = ['./7z.exe', 'a', "./test1.7z", "./test1/*"]
#param = ['./7z.exe', 'a', "../free/free_%s.7z" % datetime_str, "../free/*"]
#param.extend(exclude_exts_param)
#param.extend(exclude_dirs_param)
#cmd(param)
#cmd([
#	'./7z.exe',
#	'a',
#	"./test1.7z",
#	"./test1/*",
#	"-xr!*.suo",
#	"-xr!*.bat",
#	"-xr!*.info",
#	"-xr!*.pdb",
#	"-x!lib",
#	"-x!sub",
#	"-x!shaderc/build"])


#print("aabb")
#
print("compressing ../free/free_%s.7z" % datetime_str)
cmd([
	'./7z.exe',
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
	"-x!lib",
	"-x!shaderc/build"])

print("compressing ../testCat/bin/bin_%s.7z" % datetime_str)
cmd([
	'./7z.exe',
	'a',
	"../testCat/bin/bin_%s.7z" % datetime_str,
	"../testCat/bin/art",
	"../testCat/bin/driver",
	"../testCat/bin/*.dll",
	"-x!*.7z"])


