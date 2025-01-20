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
import subprocess
import re
from utils import *

def CheckIpcStat(output):
    result = re.search("CurrentPid:\d+\nTotalCount:\d+\nTotalTimeCost:\d+", output)
    return result is not None

def CheckIpcStartAll(output):
    pass

class TestHidumperIpc:

    @pytest.mark.L0
    def test_ipc_stat(self):
        # 校验命令行输出
        pid = GetPidByProcessName("samgr")
        output = subprocess.check_output(f"hdc shell hidumper --ipc {pid} --start-stat", shell=True, text=True, encoding="utf-8")
        assert "success" in output

        command = f"hidumper --ipc {pid} --stat"
        hidumperTmpCmd = "OPT:ipc SUB_OPT:stat"
        CheckCmd(command, CheckIpcStat, hidumperTmpCmd)
        CheckCmdRedirect(command, CheckIpcStat, None, hidumperTmpCmd)

        output = subprocess.check_output(f"hdc shell hidumper --ipc {pid} --stop-stat", shell=True, text=True, encoding="utf-8")
        assert "success" in output

    @pytest.mark.L3
    def test_ipc_error_pid(self):
        command = f"hidumper --ipc 2147483647 --stat;hidumper --ipc -2147483647 --stat"
        hidumperTmpCmd = "OPT:ipc SUB_OPT:stat"
        # 校验命令行输出
        CheckCmd(command, lambda output : "hidumper: No such process: 2147483647\nhidumper: option pid missed. 2" in output, hidumperTmpCmd)
        command = f"hidumper --ipc 2147483648 --stat;hidumper --mem -2147483648 --stat"
        CheckCmd(command, lambda output : "hidumper: option pid missed. 2" in output, hidumperTmpCmd)
