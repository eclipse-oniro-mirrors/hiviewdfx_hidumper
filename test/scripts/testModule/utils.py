#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (C) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import zipfile
import subprocess

def check_output(output, check_function):
    """
    description: 检查输出是否符合要求
    output: str, the output of the program to be checked
    check_function: 校验函数，校验output输出是否符合要求，是则返回True，否则返回False
    """
    return check_function(output)

def check_file(file_path, check_function):
    """
    description: 首先检查文件是否存在，然后检查文件内容是否符合要求
    file_path: str, the path of the file to be checked
    check_function: 校验函数，校验file_path文件内容是否符合要求，是则返回True，否则返回False
    """
    assert os.path.exists(file_path), f"The file {file_path} does not exist."
    with open(file_path, "r", encoding="utf-8") as f:
        output = f.read()
    return check_output(output, check_function)

def check_zip_file(zip_file_path, check_function):
    """
    description: 首先检查zip文件是否存在，然后解压文件，并检查解压后的log.txt文件内容是否符合要求
    check_function: 校验函数，校验解压后的log.txt文件内容是否符合要求，是则返回True，否则返回False
    """
    assert os.path.exists(zip_file_path), f"The file {zip_file_path} does not exist."
    with zipfile.ZipFile(zip_file_path, 'r') as zip_ref:
        # 解压所有文件到指定目录
        dirname = os.path.dirname(zip_file_path)
        zip_ref.extractall(dirname)
        with open(f"{dirname}/log.txt", "r", encoding="utf-8") as f:
            output = f.read()
    return check_output(output, check_function)

def print_check_result(func):
    def wrapper(*args, **kwargs):
        ret = func(*args, **kwargs)
        if (ret):
            print(f"func {func.__name__} success")
        else:
            print(f"func {func.__name__} failed")
        return ret
    return wrapper

def get_pid_by_process_name(process_name):
    pid = None
    cmd = f"hdc shell \"pidof {process_name}\""
    try:
        pid = subprocess.check_output(cmd, shell=True, encoding="utf-8", text=True)
        pid = int(pid.strip())
    except subprocess.CalledProcessError as e:
        print(f"Command failed: {cmd}\nError: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")
    return pid

def convert_string_to_matrix(data : str) -> list:
    """
    description: 将字符串转换为矩阵
    string: str, 字符串
    """
    data = data.strip("-\n")
    lines = data.split('\n')
    matrix = []
    # 遍历每一行
    for line in lines:
        # 如果行是空的，跳过
        if not line.strip():
            continue
        # 分割每一列，去除空格，并转换为整数
        row = [int(col.strip()) for col in line.split()]
        matrix.append(row)
    return matrix