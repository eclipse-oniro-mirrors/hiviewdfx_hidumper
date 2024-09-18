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

def check_ipc_stat(output):
    result = re.search("CurrentPid:\d+\nTotalCount:\d+\nTotalTimeCost:\d+", output)
    return result is not None

class TestHidumperIpc:

    @pytest.mark.L0
    def test_ipc_start_pid(self):
        # 校验命令行输出
        pid = get_pid_by_process_name("samgr")
        output = subprocess.check_output(f"hdc shell hidumper --ipc {pid} --start-stat", shell=True, text=True, encoding="utf-8")
        assert "success" in output

        output = subprocess.check_output(f"hdc shell hidumper --ipc {pid} --stat", shell=True, text=True, encoding="utf-8")
        assert check_output(output, check_function = check_ipc_stat)

        output = subprocess.check_output(f"hdc shell hidumper --ipc {pid} --stop-stat", shell=True, text=True, encoding="utf-8")
        assert "success" in output

