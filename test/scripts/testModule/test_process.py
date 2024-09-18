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
import csv
import subprocess
import re
from utils import *

OUTPUT_PATH = "testModule/output"

@print_check_result
def check_ps_eft(output):
    result = re.search("UID\s+PID\s+TID  PPID TCNT STIME TTY\s+TIME CMD\n([^\n]+\n){4,}", output)
    return result is not None

@print_check_result
def check_proc_maps(output):
    result = re.search("([\da-fA-F]{8}-[\da-fA-F]{8}.*/system/bin/init\n){3,}", output)
    return result is not None

@print_check_result
def check_mountinfo(output):
    result = re.search("/proc/1/mountinfo\n\n([^\n]+\n){4,}", output)
    return result is not None

def check_p_all(output):
    results = [check(output) for check in [check_ps_eft, check_proc_maps]]
    return all(results)

class TestHidumperProcess:

    @pytest.mark.L0
    def test_process_all(self):
        output = subprocess.check_output(f"hdc shell \"hidumper -p\"", shell=True, text=True, encoding="utf-8")
        assert check_output(output, check_function = check_p_all)

        # 校验命令行重定向输出
        redirect_file = f"{OUTPUT_PATH}/redirect.txt"
        subprocess.check_output(f"hdc shell \"hidumper -p\" > {redirect_file}", shell=True, text=True, encoding="utf-8")
        assert check_file(redirect_file, check_function = check_p_all)

        # 校验命令行输出到zip文件
        output = subprocess.check_output(f"hdc shell \"hidumper -p --zip\"", shell=True, text=True, encoding="utf-8")
        zip_source_file = re.search("The result is:(.+)", output).group(1)
        zip_target_file = f"{OUTPUT_PATH}/" + os.path.basename(zip_source_file)
        subprocess.check_output(f"hdc file recv {zip_source_file} {zip_target_file}", shell=True, text=True, encoding="utf-8")
        assert check_zip_file(zip_target_file, check_function = check_p_all)