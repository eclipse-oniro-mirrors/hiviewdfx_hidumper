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
import re

OUTPUT_PATH = "testModule/output"

def checkFile(filePath, checkFunction):
    """
    description: 首先检查文件是否存在，然后检查文件内容是否符合要求
    file_path: str, the path of the file to be checked
    check_function: 校验函数，校验file_path文件内容是否符合要求，是则返回True，否则返回False
    """
    assert os.path.exists(filePath), f"The file {filePath} does not exist."
    with open(filePath, "r", encoding="utf-8") as f:
        output = f.read()
    return checkFunction(output)

def checkZipFile(zipFilePath, checkFunction):
    """
    description: 首先检查zip文件是否存在，然后解压文件，并检查解压后的log.txt文件内容是否符合要求
    check_function: 校验函数，校验解压后的log.txt文件内容是否符合要求，是则返回True，否则返回False
    """
    assert os.path.exists(zipFilePath), f"The file {zipFilePath} does not exist."
    with zipfile.ZipFile(zipFilePath, 'r') as zip_ref:
        # 解压所有文件到指定目录
        dirname = os.path.dirname(zipFilePath)
        zip_ref.extractall(dirname)
        with open(f"{dirname}/log.txt", "r", encoding="utf-8") as f:
            output = f.read()
    return checkFunction(output)

def print_check_result(func):
    def wrapper(*args, **kwargs):
        ret = func(*args, **kwargs)
        if (ret):
            print(f"func {func.__name__} success")
        else:
            print(f"func {func.__name__} failed")
        return ret
    return wrapper

def GetPidByProcessName(processName):
    pid = None
    cmd = f"hdc shell \"pidof {processName}\""
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

def CheckCmd(command, checkFunction):
    output = subprocess.check_output(f"hdc shell \"{command}\"", shell=True, text=True, encoding="utf-8")
    assert checkFunction(output)

def CheckCmdRedirect(command, checkFunction, filePath = None):
    filePath = f"{OUTPUT_PATH}/hidumper_redirect.txt" if filePath is None else filePath
    subprocess.check_output(f"hdc shell \"{command}\" > {filePath}", shell=True, text=True, encoding="utf-8")
    assert checkFile(filePath, checkFunction = checkFunction)

def CheckCmdZip(command, checkFunction):
    output = subprocess.check_output(f"hdc shell \"{command} --zip\"", shell=True, text=True, encoding="utf-8")
    zipSourceFile = re.search("The result is:(.+)", output).group(1).strip()
    zipTargetFile = f"{OUTPUT_PATH}/" + os.path.basename(zipSourceFile)
    subprocess.check_output(f"hdc file recv {zipSourceFile} {zipTargetFile}", shell=True, text=True, encoding="utf-8")
    assert checkZipFile(zipTargetFile, checkFunction = checkFunction)