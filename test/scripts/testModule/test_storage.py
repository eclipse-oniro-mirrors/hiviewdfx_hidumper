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
def check_storaged_u_p(output):
    result = re.search("cmd is: storaged -u -p\n\n", output)
    return result is not None

@print_check_result
def check_df_k(output):
    result = re.search("cmd is: df -k\n\n([^\n]+\n){4,}", output)
    return result is not None

@print_check_result
def check_lsof(output):
    result = re.search("cmd is: lsof\n\n([^\n]+\n){4,}", output)
    return result is not None

@print_check_result
def check_iotop(output):
    result = re.search("cmd is: iotop -n 1 -m 100\n([^\n]+\n){4,}", output)
    return result is not None
    
@print_check_result
def check_proc_mount(output):
    result = re.search("/proc/mounts\n\n([^\n]+\n){4,}", output)
    return result is not None

def CheckStorageWithoutPid(output):
    ret = all(check(output) for check in [check_storaged_u_p, check_df_k, check_lsof, check_iotop, check_proc_mount])
    assert re.search("Filesystem", output) is not None
    assert re.search("tmpfs", output) is not None
    assert re.search("block", output) is not None
    assert re.search("dev", output) is not None
    return ret

def CheckStorageWithPid(output):
    result = re.search("storage io", output)
    return result is not None

class TestHidumperStorage:
    @pytest.mark.L0
    def test_storage_all(self):
        command = "hidumper --storage"
        hidumperTmpCmd = "OPT:storage SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckStorageWithoutPid, hidumperTmpCmd)

    @pytest.mark.L0
    def test_storage_pid(self):
        command = f"hidumper --storage 1"
        hidumperTmpCmd = "OPT:storage SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckStorageWithPid, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckStorageWithPid, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckStorageWithPid)

    @pytest.mark.L3
    def test_storage_error_pid(self):
        command = f"hidumper --storage 2147483647;hidumper --storage -2147483647"
        hidumperTmpCmd = "OPT:storage SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, lambda output : "hidumper: No such process: 2147483647\nhidumper: option pid missed. 2" in output, hidumperTmpCmd)
        command = f"hidumper --storage 2147483648;hidumper --storage -2147483648"
        CheckCmd(command, lambda output : "hidumper: option pid missed. 2" in output, hidumperTmpCmd)