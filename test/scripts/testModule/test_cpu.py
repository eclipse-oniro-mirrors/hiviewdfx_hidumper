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

def check_cpuusage_output(output):
    ret = re.search("PID   Total Usage	   User Space    Kernel Space    Page Fault Minor    Page Fault Major    Name\n([^\n]+\n){4,}", output)
    return ret is not None

class TestHidumperCpu:

    @pytest.mark.L0
    def test_cpuusage_all(self):
        # 校验命令行输出
        output = subprocess.check_output(f"hdc shell \"hidumper --cpuusage\"", shell=True, text=True, encoding="utf-8")
        assert check_output(output, check_function = check_cpuusage_output)

        # 校验命令行重定向输出
        redirect_file = f"{OUTPUT_PATH}/redirect.txt"
        subprocess.check_output(f"hdc shell \"hidumper --cpuusage\" > {redirect_file}", shell=True, text=True, encoding="utf-8")
        assert check_file(redirect_file, check_function = check_cpuusage_output)

        # 校验命令行输出到zip文件
        output = subprocess.check_output(f"hdc shell hidumper --cpuusage --zip", shell=True, text=True, encoding="utf-8")
        zip_source_file = re.search("The result is:(.+)", output).group(1)
        zip_target_file = f"{OUTPUT_PATH}/" + os.path.basename(zip_source_file)
        subprocess.check_output(f"hdc file recv {zip_source_file} {zip_target_file}", shell=True, text=True, encoding="utf-8")
        assert check_zip_file(zip_target_file, check_function = check_cpuusage_output)