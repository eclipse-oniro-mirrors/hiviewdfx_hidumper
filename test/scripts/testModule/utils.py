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
import time
import json

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
        pid = int(pid.strip().split()[0])
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

def CheckCmd(command, checkFunction, hidumperTmpCmd = ""):
    if len(hidumperTmpCmd) != 0:
        hisyseventOutput = GetHisyseventTmpFile()
        lastWriteDay = GetLastWriteDay()
        currentTime = GetDate()
    output = subprocess.check_output(f"hdc shell \"{command}\"", shell=True, text=True, encoding="utf-8")
    assert checkFunction(output)
    if len(hidumperTmpCmd) != 0 and not IsOpenHarmonyVersion() and IsRootVersion():
        JudgeHisyseventReport(command, hisyseventOutput, hidumperTmpCmd, currentTime, lastWriteDay)

def CheckCmdRedirect(command, checkFunction, filePath = None, hidumperTmpCmd = ""):
    if len(hidumperTmpCmd) != 0:
        hisyseventOutput = GetHisyseventTmpFile()
        lastWriteDay = GetLastWriteDay()
        currentTime = GetDate()
    filePath = f"{OUTPUT_PATH}/hidumper_redirect.txt" if filePath is None else filePath
    subprocess.check_output(f"hdc shell \"{command}\" > {filePath}", shell=True, text=True, encoding="utf-8")
    assert checkFile(filePath, checkFunction = checkFunction)
    if len(hidumperTmpCmd) != 0 and not IsOpenHarmonyVersion() and IsRootVersion():
        JudgeHisyseventReport(command, hisyseventOutput, hidumperTmpCmd, currentTime, lastWriteDay)

def CheckCmdZip(command, checkFunction):
    output = subprocess.check_output(f"hdc shell \"{command} --zip\"", shell=True, text=True, encoding="utf-8")
    zipSourceFile = re.search("The result is:(.+)", output).group(1).strip()
    zipTargetFile = f"{OUTPUT_PATH}/" + os.path.basename(zipSourceFile)
    subprocess.check_output(f"hdc file recv {zipSourceFile} {zipTargetFile}", shell=True, text=True, encoding="utf-8")
    assert checkZipFile(zipTargetFile, checkFunction = checkFunction)

def IsLogVersion():
    output = subprocess.check_output("hdc shell param get const.product.software.version", shell=True, text=True, encoding="utf-8").strip()
    return "log" in output

def IsRootVersion():
    output = subprocess.check_output("hdc shell param get const.debuggable", shell=True, text=True, encoding="utf-8").strip()
    return output == "1"

def IsOpenHarmonyVersion():
    output = subprocess.check_output("hdc shell param get const.product.software.version", shell=True, text=True, encoding="utf-8").strip()
    return "OpenHarmony" in output

def WaitUntillLogAppear(command,targetLog, second):
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=True,
        text=True
    )
    start = time.time()
    while True:
        output = process.stdout.readline()
        if targetLog in output:
            process.kill()
            return True
        now = time.time()
        if now - start > second:
            process.kill()
            return False

def IsLogAppearInCmdOutput(command, targetLog, second):
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=True,
        text=True
    )
    output = process.stdout.readline()

    try:
        process.wait(timeout=5)
        process.kill()
        return True
    except subprocess.TimeoutExpired:
        output = process.stdout
        process.kill()
        return False


def GetHisyseventTmpFile():
    # 获取/data/log/hidumper/hisysevent.tmp文件内容
    get_hisysevent_tmp_txt = "cat /data/log/hidumper/hisysevent.tmp"
    output = subprocess.check_output(f"hdc shell \"{get_hisysevent_tmp_txt}\"", shell=True, text=True, encoding="utf-8")
    if "No such file or directory" in output:
        return ""
    output = output.strip('\n')
    return output

def GetDate():
    formatted_time = subprocess.check_output(f"hdc shell date +%Y-%m-%d\ %H:%M:%S", shell=True, text=True, encoding="utf-8")
    formatted_time = formatted_time.strip('\n')
    return formatted_time

def GetDateArray(formatted_time):
    date_part = formatted_time.split(' ')[0]  # 先获取日期部分，按空格分割取第一个元素
    dateArray = date_part.split('-')  # 再按 - 分割日期部分
    return dateArray

def GetLastWriteDay():
    lastWriteTime = subprocess.check_output(f"hdc shell stat -c %y /data/log/hidumper/hisysevent.tmp", shell=True, text=True, encoding="utf-8")
    if "No such file or directory" in lastWriteTime:
        return ""
    lastWriteTime = lastWriteTime.strip(' ')
    lastWriteDay = GetDateArray(lastWriteTime)[2]
    return lastWriteDay

def UpdateDay():
    currentTime = GetDate()
    year = GetDateArray(currentTime)[0]
    month = GetDateArray(currentTime)[1]
    day = GetDateArray(currentTime)[2]
    newDay = str(int(day) + 1)
    date = year + "-" + month + "-" + newDay
    dateCmd = f"hdc shell date \"{date}\""
    output = subprocess.check_output(dateCmd, shell=True, encoding="utf-8", text=True)
    assert "00:00:00" in output

# 校验是否上报hisysevent
def JudgeHisyseventReport(command, output, hidumperTmpCmd, currentTime, lastWriteDay):
    currentDay = GetDateArray(currentTime)[2]
    # 执行hisysevent命令
    hisyseventCmd = f"hisysevent -l -S \"{currentTime}\" |grep CMD_USAGE"
    hisyseventOutput = subprocess.check_output(f"hdc shell \"{hisyseventCmd}\"", shell=True, text=True, encoding="utf-8")
    if output == "":
        print(f"hisysevent.tmp is not exist")
        return
    hisyseventFile = GetHisyseventTmpFile()
    hisyseventFileArray = hisyseventFile.split('\n')
    count = sum(string == hidumperTmpCmd for string in hisyseventFileArray)
    assert count == 1

def GetPathByAttribute(tree, key, value):
    attributes = tree['attributes']
    if attributes is None:
        print("tree contains no attributes")
        return None
    path = []
    if attributes.get(key) == value:
        return path
    for index, child in enumerate(tree['children']):
        child_path = path + [index]
        result = GetPathByAttribute(child, key, value)
        if result is not None:
            return child_path + result
    return None

def GetElementByPath(tree, path):
    if len(path) == 1:
        return tree['children'][path[0]]
    return GetElementByPath(tree['children'][path[0]], path[1:])

def GetLocationByText(tree, text):
    path = GetPathByAttribute(tree, "text", text)
    if path is None or len(path) == 0:
        print(f"text not find in layout file")
    element = GetElementByPath(tree, path)
    locations = element['attributes']['bounds'].replace('[', '').replace(']', ' ').replace(',',' ').strip().split()
    return int((int(locations[0]) + int(locations[2])) / 2), int((int(locations[1]) + int(locations[3])) / 2)

def GetLayoutTree():
    output = subprocess.check_output("hdc shell uitest dumpLayout", text=True)
    path = output.strip().split(":")[-1]
    output = subprocess.check_output(f"hdc file recv {path} {OUTPUT_PATH}/layout.json")
    with open(f"{OUTPUT_PATH}/layout.json", encoding="utf-8") as f:
        tree = json.load(f)
    return tree

def TouchButtonByText(text):
    layoutTree = GetLayoutTree()
    location = GetLocationByText(layoutTree, text)
    output = subprocess.check_output(f"hdc shell uitest uiInput click {location[0]} {location[1]}")