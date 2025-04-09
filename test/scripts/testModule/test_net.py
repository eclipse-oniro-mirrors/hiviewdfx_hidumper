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
    netstat = re.search("cmd is: netstat -nW\n", output)
    result = re.search("Proto RefCnt Flags\s+Type\s+State\s+I-Node Path\n([^\n]+\n){4,}", output)
    return result is not None and netstat is not None

@print_check_result
def CheckNetDev(output):
    proc_net_dev = re.search("/proc/net/dev\n", output)
    result = re.search("face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compresse\n([^\n]+\n){4,}", output)
    return result is not None and proc_net_dev is not None

@print_check_result
def CheckIfconfig(output):
    result = re.search("cmd is: ifconfig -a\n\n([^\n]+){4,}\n", output)
    assert re.search("Loopback", output) is not None
    assert re.search("inet", output) is not None
    return result is not None

@print_check_result
def CheckIptables(output):
    result1 = re.search("cmd is: iptables -L -nvx\n\ncmd is: ip6tables -L -nvx\n\ncmd is: iptables -t nat -L -nvx", output)
    result2 = re.search("cmd is: iptables -t mangle -L -nvx\n\ncmd is: ip6tables -t mangle -L -nvx\n\ncmd is: iptables -t raw -L -nvx", output)
    result3 = re.search("cmd is: ip6tables -t raw -L -nvx", output)
    return result1 is not None and result2 is not None and result3 is not None

@print_check_result
def CheckNetDev(output):
    assert re.search("/sys/kernel/debug/binder/failed_transaction_log", output) is not None
    assert re.search("call  from", output) is not None
    assert re.search("/sys/kernel/debug/binder/transaction_log\n", output) is not None
    assert re.search("context binder", output) is not None
    assert re.search("/sys/kernel/debug/binder/transactions\n", output) is not None
    assert re.search("binder transactions:", output) is not None
    assert re.search("/sys/kernel/debug/binder/stats\n", output) is not None
    assert re.search("binder stats:", output) is not None
    assert re.search("/sys/kernel/debug/binder/state\n", output) is not None
    lines = output.split(': u')
    isShowAddress = False
    for line in lines:
        if "0000000000000000" not in line and " c0" in line:
            print(line)
            isShowAddress = True
            break
    if IsRootVersion():
        assert isShowAddress
    else:
        assert not isShowAddress
    result = re.search("binder state:", output) is not None
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

    @pytest.mark.L3
    def test_net_error_pid(self):
        command = f"hidumper --net 2147483647;hidumper --net -2147483647"
        hidumperTmpCmd = "OPT:net SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, lambda output : "hidumper: No such process: 2147483647\nhidumper: option pid missed. 2" in output, hidumperTmpCmd)
        command = f"hidumper --net 2147483648;hidumper --net -2147483648"
        CheckCmd(command, lambda output : "hidumper: option pid missed. 2" in output, hidumperTmpCmd)