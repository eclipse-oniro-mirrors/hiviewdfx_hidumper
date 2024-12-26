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

@print_check_result
def CheckNetTraffic(output):
    result = re.search("Received Bytes:\d+\nSent Bytes:\d+\n", output)
    return result is not None

@print_check_result
def CheckNetstat(output):
    result = re.search("Proto RefCnt Flags\s+Type\s+State\s+I-Node Path\n([^\n]+\n){4,}", output)
    return result is not None

@print_check_result
def CheckNetDev(output):
    result = re.search("face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compresse\n([^\n]+\n){4,}", output)
    return result is not None

@print_check_result
def CheckIfconfig(output):
    result = re.search("cmd is: ifconfig -a\n\n([^\n]+){4,}\n", output)
    return result is not None

def CheckNetAllOutput(output):
    ret = all([CheckNetTraffic(output), CheckNetstat(output), CheckNetDev(output), CheckIfconfig(output)])
    return ret

class TestHidumperNet:

    @pytest.mark.L0
    def test_net_all(self):
        command = f"hidumper --net"
        hidumperTmpCmd = "OPT:net SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckNetAllOutput, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckNetAllOutput, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckNetAllOutput)

    @pytest.mark.L0
    def test_net_pid(self):
        command = f"hidumper --net `pidof samgr`"
        hidumperTmpCmd = "OPT:net SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckNetTraffic, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckNetTraffic, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckNetTraffic)