import os
import argparse

def is_utf8(file_path):
    """检查文件是否可以被 UTF-8 编码解析"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            f.read()
        return True
    except UnicodeDecodeError:
        return False

def contains_gbk_chars(file_path):
    """检查文件是否包含 GBK 编码字符"""
    try:
        with open(file_path, 'r', encoding='gbk') as f:
            content = f.read()
            # 检查是否包含非 ASCII 字符
            return any(ord(char) > 127 for char in content)
    except UnicodeDecodeError:
        return False

def main():
    parser = argparse.ArgumentParser(description='递归扫描目录，查找包含 GBK 编码字符的 C/C++ 源文件')
    parser.add_argument('directory', help='要扫描的目录路径')
    args = parser.parse_args()

    target_extensions = {'.cpp', '.c', '.h', '.hpp'}
    gbk_files = []

    for root, _, files in os.walk(args.directory):
        for file in files:
            ext = os.path.splitext(file)[1].lower()
            if ext in target_extensions:
                file_path = os.path.join(root, file)
                # 先检查是否为 UTF-8 编码
                if not is_utf8(file_path):
                    # 再检查是否包含 GBK 编码字符
                    if contains_gbk_chars(file_path):
                        gbk_files.append(file_path)

    if gbk_files:
        print("找到以下包含 GBK 编码字符的文件：")
        for file_path in gbk_files:
            print(file_path)
    else:
        print("未找到包含 GBK 编码字符的文件。")

if __name__ == "__main__":
    main()    


