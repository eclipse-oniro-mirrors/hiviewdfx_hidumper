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

def CheckSAList(output):
    ret = "System ability list:" in output
    return ret

def CheckSAHelp(output):
    result1 = re.search(r"-+HiviewService-+\n([^\n]+){4,}", output)
    result2 = re.search(r"-+HiDumperService-+", output, re.DOTALL)
    ret = all([result1, result2])
    return ret 

def CheckSAInterface(output):
    result = re.search(r"-+WindowManagerService-+\n([^\n]+){4,}", output)
    return result is not None

def CheckHiviewServiceFaultloggerPlugin(output):
    result1 = re.search(r"-+HiviewService-+\n([^\n]+){4,}", output)
    result2 = re.search("log", output)
    ret = all([result1, result2])
    return ret

def CheckWindowManagerService(output):
    result1 = re.search(r"-+WindowManagerService-+\n([^\n]+){4,}", output)
    result2 = re.search("WindowName", output)
    ret = all([result1, result2])
    return ret

def CheckRsAllInfo(output):
    result1 = re.search(r"-+RenderService-+\n([^\n]+){4,}", output)
    result2 = re.search("ScreenInfo", output)
    ret = all([result1, result2])
    return ret

def CheckRsHelp(output):
    result1 = re.search(r"-+RenderService-+\n([^\n]+){4,}", output)
    result2 = re.search("Graphic", output)
    ret = all([result1, result2])
    return ret

def CheckWorkSchedule(output):
    result1 = re.search(r"-+WorkSchedule-+\n([^\n]+){4,}", output)
    result2 = re.search("Work", output)
    ret = all([result1, result2])
    return ret

def CheckAmsL(output):
    result1 = re.search(r"-+AbilityManagerService-+\n([^\n]+){4,}", output)
    result2 = re.search("User", output)
    ret = all([result1, result2])
    return ret

def CheckAmsA(output):
    result1 = re.search(r"-+AbilityManagerService-+\n([^\n]+){4,}", output)
    result2 = re.search("AppRunningRecord", output)
    ret = all([result1, result2])
    return ret

def CheckDmsSA(output):
    result1 = re.search(r"-+DisplayManagerService-+\n([^\n]+){4,}", output)
    result2 = re.search("Screen", output)
    ret = all([result1, result2])
    return ret

def CheckMultimodalInputW(output):
    result1 = re.search(r"-+MultimodalInput-+\n([^\n]+){4,}", output)
    result2 = re.search("Windows", output)
    ret = all([result1, result2])
    return ret

class TestHidumperSA: 

    @pytest.mark.L0
    def test_sa_ls(self):
        command = "hidumper -ls"
        # 设置hisysevent相关信息
        hidumperTmpCmd = "OPT:ls SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckSAList, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckSAList, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckSAList)

    @pytest.mark.L0
    def test_sa_help(self):
        command = "hidumper -s 1201 1212"
        hidumperTmpCmd = "OPT:s SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckSAHelp, hidumperTmpCmd)
        # 校验命令行异常值输出输出
        CheckCmd("hidumper -s 1201 -123", lambda output : "option pid missed." in output, hidumperTmpCmd)

    @pytest.mark.L0
    def test_sa_interface(self):
        command = "hidumper -s WindowManagerService -a -h"
        hidumperTmpCmd = "OPT:s SUB_OPT:a"
        # 校验命令行输出
        CheckCmd(command, CheckSAInterface, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckSAInterface, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckSAInterface)

    @pytest.mark.L0
    def test_sa_hiview(self):
        command = "hidumper -s 1201 -a '-p Faultlogger'"
        hidumperTmpCmd = "OPT:s SUB_OPT:a"
        # 校验命令行输出
        CheckCmd(command, CheckHiviewServiceFaultloggerPlugin, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckHiviewServiceFaultloggerPlugin, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckHiviewServiceFaultloggerPlugin)

    @pytest.mark.L0
    def test_sa_wms(self):
        command = "hidumper -s WindowManagerService -a -a"
        hidumperTmpCmd = "OPT:s SUB_OPT:a"
        # 校验命令行输出
        CheckCmd(command, CheckWindowManagerService, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckWindowManagerService, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckWindowManagerService)
        # 校验命令行输出
        hidumperTmpCmd = "OPT:s SUB_OPT:"
        CheckCmd("hidumper -s WindowManagerService", lambda output : "Usage" in output, hidumperTmpCmd)
        CheckCmd("hidumper -s 4606", lambda output : "Usage" in output, hidumperTmpCmd)

    @pytest.mark.L0
    def test_sa_rs_allinfo(self):
        command = "hidumper -s 10 -a allInfo"
        hidumperTmpCmd = "OPT:s SUB_OPT:a"
        # 校验命令行输出
        CheckCmd(command, CheckRsAllInfo, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckRsAllInfo, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckRsAllInfo)

    @pytest.mark.L0
    def test_sa_rs_help(self):
        command = "hidumper -s 10 -a -h"
        hidumperTmpCmd = "OPT:s SUB_OPT:a"
        # 校验命令行输出
        CheckCmd(command, CheckRsHelp, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckRsHelp, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckRsHelp)

    @pytest.mark.L0
    def test_sa_workschedule(self):
        command = "hidumper -s 1904 -a -a"
        hidumperTmpCmd = "OPT:s SUB_OPT:a"
        # 校验命令行输出
        CheckCmd(command, CheckWorkSchedule, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckWorkSchedule, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckWorkSchedule)

    @pytest.mark.L0
    def test_sa_ams_l(self):
        command = "hidumper -s AbilityManagerService -a -l"
        hidumperTmpCmd = "OPT:s SUB_OPT:a"
        # 校验命令行输出
        CheckCmd(command, CheckAmsL, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckAmsL, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckAmsL)

    @pytest.mark.L0
    def test_sa_ams_a(self):
        command = "hidumper -s AbilityManagerService -a '-a'"
        hidumperTmpCmd = "OPT:s SUB_OPT:a"
        # 校验命令行输出
        CheckCmd(command, CheckAmsA, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckAmsA, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckAmsA)

    @pytest.mark.L0
    def test_sa_dms_s_a(self):
        command = "hidumper -s DisplayManagerService -a '-s -a'"
        hidumperTmpCmd = "OPT:s SUB_OPT:a"
        # 校验命令行输出
        CheckCmd(command, CheckDmsSA, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckDmsSA, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckDmsSA)

    @pytest.mark.L0
    def test_sa_multimodalinput_w(self):
        command = "hidumper -s MultimodalInput -a -w"
        hidumperTmpCmd = "OPT:s SUB_OPT:a"
        # 校验命令行输出
        CheckCmd(command, CheckMultimodalInputW, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckMultimodalInputW, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckMultimodalInputW)

    @pytest.mark.L0
    def test_sa(self):
        command = "hidumper -s"
        hidumperTmpCmd = "OPT:s SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, lambda output : "HiDumperManagerService" in output, hidumperTmpCmd)

    @pytest.mark.L0
    def test_sa_error_option(self):
        command = "hidumper -s abc;hidumper -s -123;hidumper -s 123456789123456789123456789123456789"
        hidumperTmpCmd = "OPT:s SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, lambda output : "hidumper: invalid arg: abc\nhidumper: option pid missed. 1\nhidumper: invalid arg: 123456789123456789123456789123456789" in output, hidumperTmpCmd)

    @pytest.mark.L0
    def test_sa_screenshot(self):
        if IsOpenHarmonyVersion():
            pytest.skip("this testcase is only support in HO")
        else:
            process_hilog = subprocess.Popen(['hdc', 'shell', 'hilog | grep Dumper > /data/local/tmp/screenshot.txt'])
            time.sleep(1)
            # 唤醒屏幕
            subprocess.check_call("hdc shell power-shell wakeup", shell=True)
            # 设置屏幕常亮
            subprocess.check_call("hdc shell power-shell setmode 602", shell=True)
            time.sleep(3)
            # 解锁屏幕
            subprocess.check_call("hdc shell uinput -T -g 100 100 500 500", shell=True)
            # 触发截图
            subprocess.check_call("hdc shell uinput -T -m 1000 0 1000 1000 500 && hdc shell uinput -T -c 300 2300", shell=True)
            time.sleep(3)
            process_hilog.terminate()
            output = subprocess.check_output(f"hdc shell cat /data/local/tmp/screenshot.txt", text=True, encoding="utf-8")
            assert "dump fail!ret" not in output
            assert "dump success, cmd" in output

