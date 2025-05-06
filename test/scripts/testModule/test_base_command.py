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
import time
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
        hidumperTmpCmd = "ERROR_MESSAGE:parse cmd fail"
        # 校验命令行输出
        CheckCmd("hidumper -h", lambda output : "mem" in output, hidumperTmpCmd)
        command = "hdc shell \"hidumper -h |grep mem\""
        output = subprocess.check_output(command, shell=True, encoding="utf-8", text=True)
        assert "mem" in output

    @pytest.mark.L0
    def test_hidumper_lc(self):
        command = "hidumper -lc"
        # 设置hisysevent相关信息
        hidumperTmpCmd = "OPT:lc SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, lambda output : "base                             system" in output, hidumperTmpCmd)
        # 校验-l拼接不存在字符输出
        CheckCmd("hidumper -ld", lambda output : "option pid missed." in output, hidumperTmpCmd)
        CheckCmd("hidumper -lcd", lambda output : "option pid missed." in output, hidumperTmpCmd)
        CheckCmd("hidumper -l", lambda output : "invalid arg: -1" in output, hidumperTmpCmd)

    @pytest.mark.L0
    def test_hidumper_c_all(self):
        hidumperTmpCmd = "OPT:c SUB_OPT:"
        CheckFunc = lambda output : all([check(output) for check in [CheckBuildId, CheckOsVersion, CheckProcVersion,
                                                                    CheckCmdline, CheckUpTime, CheckPrintEnv, CheckLsmod,
                                                                    CheckSlabinfo, CheckZoneinfo, CheckVmstat, CheckVmallocinfo]])
        # 校验命令行输出
        CheckCmd("hidumper -c", CheckFunc, hidumperTmpCmd)

    @pytest.mark.L0
    def test_hidumper_c_base(self):
        command = "hidumper -c base"
        hidumperTmpCmd = "OPT:c SUB_OPT:base"
        CheckFunc = lambda output : all([check(output) for check in [CheckBuildId, CheckOsVersion, CheckProcVersion, CheckCmdline, CheckUpTime]])
        # 校验命令行输出
        CheckCmd(command, CheckFunc, hidumperTmpCmd)

    @pytest.mark.L0
    def test_hidumper_c_system(self):
        command = "hidumper -c system"
        hidumperTmpCmd = "OPT:c SUB_OPT:system"
        CheckFunc = lambda output : all([check(output) for check in [CheckPrintEnv, CheckLsmod, CheckSlabinfo, CheckZoneinfo, CheckVmstat, CheckVmallocinfo]])
        # 校验命令行输出
        CheckCmd(command, CheckFunc, hidumperTmpCmd)

    @pytest.mark.L0
    def test_hidumper_e(self):
        command = "hidumper -e"
        hidumperTmpCmd = "OPT:e SUB_OPT:"
        CheckFunc = lambda output : "faultlog" in output
        # 校验命令行输出
        CheckCmd(command, CheckFunc, hidumperTmpCmd)

    @pytest.mark.L0
    def test_hidumper_error_option(self):
        command = "hdc shell \"hidumper safsadf -h\""
        output = subprocess.check_output(command, shell=True, encoding="utf-8", text=True)
        assert "option pid missed." in output

        command = "hdc shell \"hidumper -h -D\""
        output = subprocess.check_output(command, shell=True, encoding="utf-8", text=True)
        assert "usage:" in output

        command = "hdc shell \"hidumper -habc\""
        output = subprocess.check_output(command, shell=True, encoding="utf-8", text=True)
        assert "usage:" in output

    @pytest.mark.L2
    def test_hidumper_service_exit(self):
        command = "hdc shell \"hidumper -h\""
        output = subprocess.check_output(command, shell=True, encoding="utf-8", text=True)
        command = "hdc shell \"pidof hidumper_service\""
        output = subprocess.check_output(command, shell=True, encoding="utf-8", text=True)
        assert int(output.strip()) > 0
        time.sleep(120) # 120s
        command = "hdc shell \"pidof hidumper_service\""
        output = subprocess.check_output(command, shell=True, encoding="utf-8", text=True)
        output = output.strip('\n')
        assert output == ""

