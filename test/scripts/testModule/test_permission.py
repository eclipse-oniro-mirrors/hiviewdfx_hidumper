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

import pytest
import re
import time
import subprocess
from utils import *

PSS_INDEX = 2
SWAP_PSS_INDEX = 8

def ParseSmapsOutput(output):
    memory_data = {}
    for line in output.split("\n"):
        if re.search(r"\d", line) is None:
            continue
        key = re.search(r'(/|\[|[a-zA-Z])(.*$)', line)
        key = key.group().strip()
        memory_data[key] = [int(val) for val in re.findall(r'\d+', line)]
    return memory_data

def IsLogVersion():
    output = subprocess.check_output("hdc shell param get const.product.software.version", shell=True, text=True, encoding="utf-8").strip()
    return "log" in output

def IsRootVersion():
    output = subprocess.check_output("hdc shell param get const.debuggable", shell=True, text=True, encoding="utf-8").strip()
    return output == "1"

def IsOpenHarmonyVersion():
    output = subprocess.check_output("hdc shell param get const.product.software.version", shell=True, text=True, encoding="utf-8").strip()
    return "OpenHarmony" in output

@print_check_result
def CheckSmapsTotalPss(memory_data):
    PSS_SUMMARY = memory_data["Summary"][PSS_INDEX]
    SWAPPSS_SUMMARY = memory_data["Summary"][SWAP_PSS_INDEX]
    return sum([val[PSS_INDEX] for key, val in memory_data.items() if key != "Summary"]) + SWAPPSS_SUMMARY == PSS_SUMMARY

@print_check_result
def CheckMemAddrss(output):
    ret = re.search(r"\s([a-zA-Z0-9]{8,})\s", output)
    return ret is not None

@print_check_result
def CheckPseft(output):
    result = re.search("UID\s+PID\s+TID  PPID TCNT STIME TTY\s+TIME CMD\n([^\n]+\n){1,}", output)
    return result is not None

@print_check_result
def CheckMountInfo(output):
    result = re.search("/proc/\d+/mountinfo\n\n([^\n]+\n){4,}", output)
    return result is not None

def CheckSmaps(output):
    result = re.search(r"/proc/\d+/maps", output)
    return result is not None

def CheckMemSmapsWithRoot(output):
    memory_data = ParseSmapsOutput(output)
    ret = all(check(memory_data) for check in [CheckSmapsTotalPss])
    return ret

def CheckMemSmapsVWithRoot(output):
    memory_data = ParseSmapsOutput(output)
    return CheckSmapsTotalPss(memory_data) and CheckMemAddrss(output)

def CheckHelpOutput(output):
    return "usage:" in output

def CheckMemJsheap(output):
    command = None
    if IsRootVersion():
        command = "hdc shell \"ls -l /data/log/faultlog/temp |grep jsheap\""
    else:
        command = "hdc shell \"ls -l /data/log/reliability/resource_leak/memory_leak |grep jsheap\""
    output = subprocess.check_output(command, shell=True, text=True, encoding="utf-8").strip()
    return output != ""

class TestHidumperPermission:
    @classmethod
    def setup_class(cls):
        if not IsRootVersion():
            subprocess.check_call("hdc install testModule/resource/entry-default-signed.hap", shell=True)
            time.sleep(1)
            subprocess.check_call("hdc shell aa start -a EntryAbility -b com.example.myapplication", shell=True)
    
    @classmethod
    def teardown_class(cls):
        if not IsRootVersion():
            subprocess.check_call("hdc uninstall com.example.myapplication", shell=True)

    @pytest.mark.L0
    def test_mem_smaps(self):
        processName = "render_service"
        pid = GetPidByProcessName(processName)
        if (IsRootVersion()):
            CheckFunc = CheckMemSmapsWithRoot
        else:
            CheckFunc = CheckHelpOutput
        command = f"hidumper --mem-smaps {pid}"
        # 校验命令行输出
        CheckCmd(command, CheckFunc)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckFunc)
        # 校验命令行输出到zip文件
        if IsRootVersion():
            CheckCmdZip(command, CheckFunc)

    @pytest.mark.L0
    def test_mem_smaps_v(self):
        processName = "render_service"
        pid = GetPidByProcessName(processName)
        if (IsRootVersion()):
            CheckFunc = CheckMemSmapsVWithRoot
        else:
            CheckFunc = CheckHelpOutput
        command = f"hidumper --mem-smaps {pid} -v"
        # 校验命令行输出
        CheckCmd(command, CheckFunc)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckFunc)
        # 校验命令行输出到zip文件
        if IsRootVersion():
            CheckCmdZip(command, CheckFunc)

    @pytest.mark.L0
    def test_process_all(self):
        if IsRootVersion():
            CheckFunc = lambda x : all([CheckSmaps(x), CheckPseft(x), CheckMountInfo(x)])
        else:
            CheckFunc = lambda x : all([not CheckSmaps(x), CheckPseft(x), CheckMountInfo(x)])
            pid = GetPidByProcessName("com.example.myapplication")
            if pid == "":
                pytest.skip("test application not found")
        command = f"hidumper -p"
        # 校验命令行输出
        CheckCmd(command, CheckFunc)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckFunc)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckFunc)

    @pytest.mark.L0
    def test_process_pid(self):
        command = None
        pid = 1
        if IsRootVersion():
            CheckFunc = lambda x : all([CheckSmaps(x), CheckPseft(x), CheckMountInfo(x)])
        else:
            CheckFunc = lambda x : all([not CheckSmaps(x), CheckPseft(x), CheckMountInfo(x)])
            pid = GetPidByProcessName("com.example.myapplication")
            if pid == "":
                pytest.skip("test application not found")
        command = f"hidumper -p {pid}"
        # 校验命令行输出
        CheckCmd(command, CheckFunc)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckFunc)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckFunc)

    @pytest.mark.L0
    def test_mem_jsheap(self):
        pid = None
        if IsOpenHarmonyVersion():
            pid = GetPidByProcessName("com.ohos.launcher")
        elif IsRootVersion():
            pid = GetPidByProcessName("com.ohos.sceneboard")
        else:
            pid = GetPidByProcessName("com.example.myapplication")
            if pid == "":
                pytest.skip("test application not found")
        command = f"hidumper --mem-jsheap {pid}"
        # 校验命令行输出
        CheckCmd(command, CheckMemJsheap)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckMemJsheap)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckMemJsheap)