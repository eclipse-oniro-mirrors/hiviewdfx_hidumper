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

class TestHidumperSA: 

    @pytest.mark.L0
    def test_sa_ls(self):
        command = "hidumper -ls"
        # 校验命令行输出
        CheckCmd(command, CheckSAList)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckSAList)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckSAList)

    @pytest.mark.L0
    def test_sa_help(self):
        command = "hidumper -s 1201 1212"
        # 校验命令行输出
        CheckCmd(command, CheckSAHelp)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckSAHelp)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckSAHelp)

    @pytest.mark.L0
    def test_sa_interface(self):
        command = "hidumper -s WindowManagerService -a -h"
        # 校验命令行输出
        CheckCmd(command, CheckSAInterface)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckSAInterface)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckSAInterface)