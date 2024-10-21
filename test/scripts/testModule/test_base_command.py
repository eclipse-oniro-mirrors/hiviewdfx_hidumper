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
import subprocess
from utils import *

@print_check_result
def CheckBuildId(output) -> bool:
    ret = re.search("BuildId: (.+?) [\d].[\d].[\d].[\d]", output)
    return ret is not None

@print_check_result
def CheckOsVersion(output) -> bool:
    ret = re.search("OsVersion: (.+?)[\d].[\d].[\d].[\d]", output)
    return ret is not None

@print_check_result
def CheckProcVersion(output) -> bool:
    ret = re.search("/proc/version\n\n([^\n]+\n)+\n", output)
    return ret is not None

@print_check_result
def CheckCmdline(output) -> bool:
    ret = re.search("/proc/cmdline\n\n([^\n]+\n)+\n", output)
    return ret is not None

@print_check_result
def CheckWakeUpSource(output) -> bool:
    ret = re.search("/sys/kernel/debug/wakeup_sources\n\n([^\n]+\n){4,}", output)
    return ret is not None

@print_check_result
def CheckUpTime(output) -> bool:
    ret = re.search("cmd is: uptime -p\n(\n[^\n]+)\n", output)
    return ret is not None

@print_check_result
def CheckPrintEnv(output) -> bool:
    ret = re.search("cmd is: printenv\n\n([^\n]+){4,}\n", output)
    return ret is not None

@print_check_result
def CheckProcModules(output) -> bool:
    ret = re.search("/proc/modules\n\n([^\n]+)\n", output)
    return ret is not None

@print_check_result
def CheckLsmod(output) -> bool:
    ret = re.search("cmd is: lsmod\n\n([^\n]+)\n", output)
    return ret is not None

@print_check_result
def CheckSlabinfo(output) -> bool:
    ret = re.search("/proc/slabinfo\n\n([^\n]+){4,}\n", output)
    return ret is not None

@print_check_result
def CheckZoneinfo(output) -> bool:
    ret = re.search("/proc/zoneinfo\n\n([^\n]+){4,}\n", output)
    return ret is not None

@print_check_result
def CheckVmstat(output) -> bool:
    ret = re.search("/proc/vmstat\n\n([^\n]+)\n", output)  
    return ret is not None

@print_check_result
def CheckVmallocinfo(output) -> bool:
    ret = re.search("/proc/vmallocinfo\n\n([^\n]+)\n", output)
    return ret is not None


def CheckHidumperHelpOutput(output):
    return "usage:" in output

class TestBaseCommand:

    @pytest.mark.L0
    def test_hidumper_help(self):
        # 校验命令行输出
        CheckCmd("hidumper -h", lambda output : "usage:" in output)
        # 校验命令行重定向输出
        CheckCmdRedirect("hidumper -h", lambda output : "usage:" in output)

    @pytest.mark.L0
    def test_hidumper_c_all(self):
        CheckFunc = lambda output : all([check(output) for check in [CheckBuildId, CheckOsVersion, CheckProcVersion,
                                                                    CheckCmdline, CheckUpTime, CheckPrintEnv, CheckLsmod,
                                                                    CheckSlabinfo, CheckZoneinfo, CheckVmstat, CheckVmallocinfo]])
        # 校验命令行输出
        CheckCmd("hidumper -c", CheckFunc)
        # 校验命令行重定向输出
        CheckCmdRedirect("hidumper -c", CheckFunc)
        # 校验命令行输出到zip文件
        CheckCmdZip("hidumper -c", CheckFunc)

    @pytest.mark.L0
    def test_hidumper_c_base(self):
        command = "hidumper -c base"
        CheckFunc = lambda output : all([check(output) for check in [CheckBuildId, CheckOsVersion, CheckProcVersion, CheckCmdline, CheckUpTime]])
        # 校验命令行输出
        CheckCmd(command, CheckFunc)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckFunc)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckFunc)

    @pytest.mark.L0
    def test_hidumper_c_system(self):
        command = "hidumper -c system"
        CheckFunc = lambda output : all([check(output) for check in [CheckPrintEnv, CheckLsmod, CheckSlabinfo, CheckZoneinfo, CheckVmstat, CheckVmallocinfo]])
        # 校验命令行输出
        CheckCmd(command, CheckFunc)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckFunc)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckFunc)

    @pytest.mark.L0
    def test_hidumper_e(self):
        command = "hidumper -e"
        CheckFunc = lambda output : "faultlog" in output
        # 校验命令行输出
        CheckCmd(command, CheckFunc)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckFunc)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckFunc)

    @pytest.mark.L0
    def test_hidumper_error_option(self):
        command = "hdc shell \"hidumper -D -h\""
        output = subprocess.check_output(command, shell=True, encoding="utf-8", text=True)
        assert "option pid missed." in output

        command = "hdc shell \"hidumper -h -D\""
        output = subprocess.check_output(command, shell=True, encoding="utf-8", text=True)
        assert "usage:" in output
    
    

