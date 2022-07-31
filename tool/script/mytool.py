import os
import shutil
import subprocess


def clear_dir(path, target_dirs, target_exts, recursive=True):
    filelist = os.listdir(path)
    for filename in filelist:
        filepath = os.path.join(path, filename)
        if os.path.isdir(filepath):
            is_target = False
            for dir in target_dirs:
                if filename.lower() == dir.lower():
                    print(filepath)
                    shutil.rmtree(filepath)
                    is_target = True
                    break;
            if not is_target and recursive:
                clear_dir(filepath, target_dirs, target_exts)
        else:
            is_target = False
            cur_ext = os.path.splitext(filename)[-1].lower()
            #print(cur_ext)
            for ext in target_exts:
                if cur_ext.lower() == ext.lower():
                    print(filepath)
                    os.remove(filepath)
                    is_target = True
                    break;
            #file_handler(filename, ext, filepath)

def cmd(cmd):
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError:
        print("Execute failed. %s" % cmd)
