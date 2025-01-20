#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (C) 2025 Huawei Device Co., Ltd.
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

@print_check_result
def CheckPseft(output):
    assert re.search("cmd is: ps -efT", output) is not None
    result = re.search("UID\s+PID\s+TID  PPID TCNT STIME TTY\s+TIME CMD\n([^\n]+\n){1,}", output)
    return result is not None

@print_check_result
def CheckMountInfo(output):
    result = re.search("/proc/\d+/mountinfo\n\n([^\n]+\n){4,}", output)
    return result is not None

def CheckSmaps(output):
    result = re.search(r"/proc/\d+/maps", output)
    return result is not None

@print_check_result
def CheckWchan(output):
    result = re.search("/proc/\d+/wchan", output)
    return result is not None

@print_check_result
def CheckPsOTime(output):
    result = re.search("cmd is: ps -o", output)
    return result is not None


class TestHidumperProcess:
    @classmethod
    def setup_class(cls):
        if not IsRootVersion():
            subprocess.check_call("hdc shell aa start -a EntryAbility -b com.example.jsleakwatcher", shell=True)

    @classmethod
    def teardown_class(cls):
        if not IsRootVersion():
            subprocess.check_call("hdc shell aa force-stop -b com.example.jsleakwatcher", shell=True)

    @pytest.mark.L0
    def test_process_all(self):
        if IsRootVersion():
            CheckFunc = lambda x : all([CheckSmaps(x), CheckPseft(x), CheckMountInfo(x), CheckPsOTime(x), CheckWchan(x)])
        else:
            CheckFunc = lambda x : all([not CheckSmaps(x), CheckPseft(x), CheckMountInfo(x), CheckPsOTime(x), CheckWchan(x)])
            pid = GetPidByProcessName("com.example.myapplication")
            if pid == "":
                pytest.skip("test application not found")
        command = f"hidumper -p"
        hidumperTmpCmd = "OPT:p SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckFunc, hidumperTmpCmd)

    @pytest.mark.L0
    def test_process_pid(self):
        command = None
        pid = 1
        if IsRootVersion():
            CheckFunc = lambda x : all([CheckSmaps(x), CheckPseft(x), CheckMountInfo(x), CheckPsOTime(x), CheckWchan(x)])
        else:
            CheckFunc = lambda x : all([not CheckSmaps(x), CheckPseft(x), CheckMountInfo(x), CheckPsOTime(x), CheckWchan(x)])
            pid = GetPidByProcessName("com.example.myapplication")
            if pid == "":
                pytest.skip("test application not found")
        command = f"hidumper -p {pid}"
        hidumperTmpCmd = "OPT:p SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckFunc, hidumperTmpCmd)

    @pytest.mark.L0
    def test_process_with_non_debug_pid(self):
        if not IsRootVersion():
            output = subprocess.check_output("hdc shell \"hidumper -p 1\"", shell=True, text=True, encoding="utf-8")
            assert "only support debug application" in output

    @pytest.mark.L3
    def test_process_error_pid(self):
        command = f"hidumper -p 2147483647;hidumper -p -2147483647"
        hidumperTmpCmd = "OPT:p SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, lambda output : "hidumper: No such process: 2147483647\nhidumper: option pid missed. 2" in output, hidumperTmpCmd)
        command = f"hidumper -p 2147483648;hidumper -p -2147483648"
        CheckCmd(command, lambda output : "hidumper: option pid missed. 2" in output, hidumperTmpCmd)

