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
import os
from utils import *

OUTPUT_PATH = "testModule/output"

@print_check_result
def CheckBuildId(output) -> bool:
    ret = re.search("BuildId: (.+?) [\d].[\d].[\d].[\d]", output)
    return ret is not None

@print_check_result
def CheckOsVersion(output) -> bool:
    ret = re.search("OsVersion: (.+?)[\d].[\d].[\d].[\d]", output)
    return ret is not None

@print_check_result
def CheckProcVersion(output) -> bool:
    ret = re.search("/proc/version\n\n([^\n]+\n)+\n", output)
    return ret is not None

@print_check_result
def CheckCmdline(output) -> bool:
    ret = re.search("/proc/cmdline\n\n([^\n]+\n)+\n", output)
    return ret is not None

@print_check_result
def CheckWakeUpSource(output) -> bool:
    ret = re.search("/sys/kernel/debug/wakeup_sources\n\n([^\n]+\n){4,}", output)
    return ret is not None

@print_check_result
def CheckUpTime(output) -> bool:
    ret = re.search("cmd is: uptime -p\n(\n[^\n]+)\n", output)
    return ret is not None

@print_check_result
def CheckPrintEnv(output) -> bool:
    ret = re.search("cmd is: printenv\n\n([^\n]+){4,}\n", output)
    return ret is not None

@print_check_result
def CheckProcModules(output) -> bool:
    ret = re.search("/proc/modules\n\n([^\n]+)\n", output)
    return ret is not None

@print_check_result
def CheckLsmod(output) -> bool:
    ret = re.search("cmd is: lsmod\n\n([^\n]+)\n", output)
    return ret is not None

@print_check_result
def CheckSlabinfo(output) -> bool:
    ret = re.search("/proc/slabinfo\n\n([^\n]+){4,}\n", output)
    return ret is not None

@print_check_result
def CheckZoneinfo(output) -> bool:
    ret = re.search("/proc/zoneinfo\n\n([^\n]+){4,}\n", output)
    return ret is not None

@print_check_result
def CheckVmstat(output) -> bool:
    ret = re.search("/proc/vmstat\n\n([^\n]+)\n", output)  
    return ret is not None

@print_check_result
def CheckVmallocinfo(output) -> bool:
    ret = re.search("/proc/vmallocinfo\n\n([^\n]+)\n", output)
    return ret is not None

def check_hidumper_c_output(output):
    results = [check(output) for check in [CheckBuildId, CheckOsVersion, CheckProcVersion, CheckCmdline, CheckWakeUpSource, 
                                      CheckUpTime, CheckPrintEnv, CheckLsmod, CheckSlabinfo, CheckZoneinfo, CheckVmstat, CheckVmallocinfo]]
    return all(results)

class TestBaseCommand:
    @classmethod
    def setup_class(cls):
        cls.commands = []
        cls.re_patterns = []
        with open('./testModule/resource/testdata.csv', encoding="utf-8", newline='') as csvfile:
                reader = csv.DictReader(csvfile)
                for row in reader:
                    cls.commands.append(row["command"].strip())
                    cls.re_patterns.append(row["re_pattern"].strip())
        
    @pytest.mark.L0
    def test_base_command(self):
        for cmd,pattern in zip(self.commands,self.re_patterns):
            try:
                # 执行命令并捕获输出
                output = subprocess.check_output(cmd, shell=True, text=True, encoding="utf-8")
                # 检查输出是否包含断言字符串
                if output and re.search(pattern, output):
                    print(f"Test passed for command: {cmd}")
                else:
                    raise AssertionError(f"Test failed for command: {cmd}")
            except subprocess.CalledProcessError as e:
                print(f"Command failed: {cmd}\nError: {e}")
            except AssertionError as e:
                print(e)

    @pytest.mark.L0
    def test_hidumper_c_all(self):
        # 校验命令行输出
        output = subprocess.check_output(f"hdc shell \"hidumper -c\"", shell=True, text=True, encoding="utf-8")
        assert check_output(output, check_function = check_hidumper_c_output)

        # 校验命令行重定向输出
        redirect_file = f"{OUTPUT_PATH}/hidumper_c_redirect.txt"
        subprocess.check_output(f"hdc shell \"hidumper -c\" > {redirect_file}", shell=True, text=True, encoding="utf-8")
        assert check_file(redirect_file, check_function = check_hidumper_c_output)

        # 校验命令行输出到zip文件
        output = subprocess.check_output(f"hdc shell \"hidumper -c --zip\"", shell=True, text=True, encoding="utf-8")
        zip_source_file = re.search("The result is:(.+)", output).group(1)
        zip_target_file = f"{OUTPUT_PATH}/" + os.path.basename(zip_source_file)
        subprocess.check_output(f"hdc file recv {zip_source_file} {zip_target_file}", shell=True, text=True, encoding="utf-8")
        assert check_zip_file(zip_target_file, check_function = check_hidumper_c_output)

