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

class TestHidumperCpu:

    @pytest.mark.L0
    def test_cpuusage_all(self):
        command = "hidumper --cpuusage"
        # 校验命令行输出
        CheckCmd(command, CheckCpuUsageOutput)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckCpuUsageOutput)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckCpuUsageOutput)

    @pytest.mark.L0
    def test_cpuusage_pid(self):
        command = "hidumper --cpuusage 1"
        # 校验命令行输出
        CheckCmd(command, CheckCpuUsageWithPidOutput)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckCpuUsageWithPidOutput)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckCpuUsageWithPidOutput)

    @pytest.mark.L0
    def test_cpufreq(self):
        command = "hidumper --cpufreq"
        # 校验命令行输出
        CheckCmd(command, CheckCpufreqOutput)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckCpufreqOutput)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckCpufreqOutput)