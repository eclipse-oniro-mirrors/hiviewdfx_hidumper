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

OUTPUT_PATH = "testModule/output"
LIB_PATH = "/system/lib"
ROM_BASE_SIZE = 1363
RAM_LIMIT_SIZE = 20 * 1024

def check_rom(output):
    result = re.findall("(/d+)K", output)
    return sum(int(x) for x in result) <= ROM_BASE_SIZE

def check_ram(output):
    result = re.findall("\s*Total\s*(\d+)\s*", output)[0]
    return int(result) <= RAM_LIMIT_SIZE

class TestHidumperPerformance:

    @pytest.mark.L0
    def test_rom(self):
        # 校验命令行输出
        output = subprocess.check_output(f"hdc shell ls -lh /system/lib/libhidumper*", shell=True, text=True, encoding="utf-8")
        assert check_output(output, check_function = check_rom)
    
    @pytest.mark.L0
    def test_ram(self):
        # 校验命令行输出
        subprocess.check_call(f"hdc shell hidumper --mem 1", shell=True, text=True, encoding="utf-8")
        output = subprocess.check_output(f"hdc shell hidumper --mem `pidof hidumper_service`", shell=True, text=True, encoding="utf-8")
        assert check_output(output, check_function = check_ram)

