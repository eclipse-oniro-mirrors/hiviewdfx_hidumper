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
from utils import *

def CheckCpuUsageOutput(output):
    ret = re.search("PID   Total Usage	   User Space    Kernel Space    Page Fault Minor    Page Fault Major    Name\n([^\n]+\n){4,}", output)
    return ret is not None

def CheckCpuUsageWithPidOutput(output):
    ret = re.search("PID   Total Usage	   User Space    Kernel Space    Page Fault Minor    Page Fault Major    Name\n([^\n]+\n){1,}", output)
    return ret is not None

def CheckCpufreqOutput(output):
    ret = re.search("cmd is: cat /sys/devices/system/cpu/cpu\d/cpufreq/cpuinfo_cur_freq\n\n\d+", output)
    return ret is not None

def GetCpuUsageOutput():
    subprocess.check_call("hdc shell aa start -a EntryAbility -b com.example.jsleakwatcher", shell=True)
    process_hilog = subprocess.Popen(['hdc', 'shell', 'hilog | grep jsLeakWatcher_app_cpuUsage > /data/local/tmp/getcpuusage_interface.txt'])
    time.sleep(1)
    # 单击【GetCpuUsage】按钮
    TouchButtonByText("GetCpuUsage")
    time.sleep(3)
    process_hilog.terminate()
    time.sleep(3)
    output = subprocess.check_output(f"hdc shell cat /data/local/tmp/getcpuusage_interface.txt", text=True, encoding="utf-8")
    return output

def GetJsLeakWatcherCpuUsage(output):
    # 按行分割数据
    lines = output.splitlines()
    # 初始化一个空列表来存储提取的 CPU 使用率
    cpu_usages = []
    # 遍历每一行
    for line in lines:
        # 检查行中是否包含 "jsLeakWatcher_app_cpuUsage:"
        if "jsLeakWatcher_app_cpuUsage:" in line:
            # 分割行并提取 CPU 使用率
            parts = line.split(":")
            cpu_usage = parts[-1].strip()
            cpu_usages.append(cpu_usage)
    return float(cpu_usages[-1])

class TestHidumperCpu:

    @pytest.mark.L0
    def test_cpuusage_all(self):
        command = "hidumper --cpuusage"
        hidumperTmpCmd = "OPT:cpuusage SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckCpuUsageOutput, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckCpuUsageOutput, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckCpuUsageOutput)

    @pytest.mark.L0
    def test_cpuusage_pid(self):
        command = "hidumper --cpuusage 1"
        hidumperTmpCmd = "OPT:cpuusage SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckCpuUsageWithPidOutput, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckCpuUsageWithPidOutput, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckCpuUsageWithPidOutput)

    @pytest.mark.L0
    def test_getcpuusage_interface(self):
        output = GetCpuUsageOutput()
        assert "jsLeakWatcher_app_cpuUsage:" in output
        jsLeakWatcher_app_cpuUsage = GetJsLeakWatcherCpuUsage(output)
        assert jsLeakWatcher_app_cpuUsage > 0

    @pytest.mark.L3
    def test_cpuusage_error_pid(self):
        command = f"hidumper --cpuusage 2147483647;hidumper --cpuusage -2147483647"
        hidumperTmpCmd = "OPT:cpuusage SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, lambda output : "hidumper: No such process: 2147483647\nhidumper: option pid missed. 2" in output, hidumperTmpCmd)
        command = f"hidumper --cpuusage 2147483648;hidumper --cpuusage -2147483648"
        CheckCmd(command, lambda output : "hidumper: option pid missed. 2" in output, hidumperTmpCmd)

    @pytest.mark.L0
    def test_cpufreq(self):
        command = "hidumper --cpufreq"
        hidumperTmpCmd = "OPT:cpufreq SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckCpufreqOutput, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckCpufreqOutput, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckCpufreqOutput)