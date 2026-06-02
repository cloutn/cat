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

def exec_cmd(cmd, print_cmd=True, shell=False, check=True):
    if print_cmd:
        if isinstance(cmd, list):
            print(" ".join(cmd))
        else:
            print(cmd)

    # check=True 时让 CalledProcessError 自然抛到调用方；之前 try/except 只 print 不
    # raise，等于把 check=True 的语义抹平成 check=False，会造成 build 步骤失败但脚本
    # 继续往下跑、最终 .bat 仍然 echo "complete." 的"假成功"。
    subprocess.run(cmd, shell=shell, check=check)
