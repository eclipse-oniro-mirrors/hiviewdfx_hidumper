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

class TestScreenshot:

    @pytest.mark.L0
    def test_screenshot(self):
        if IsOpenHarmonyVersion():
            pytest.skip("this testcase is only support in HO")
        else:
            process_hilog = subprocess.Popen(['hdc', 'shell', 'hilog | grep Dumper > /data/local/tmp/screenshot.txt'])
            time.sleep(1)
            # 唤醒屏幕
            subprocess.check_call("hdc shell power-shell wakeup", shell=True)
            # 设置屏幕常亮
            subprocess.check_call("hdc shell power-shell setmode 602", shell=True)
            time.sleep(3)
            # 解锁屏幕
            subprocess.check_call("hdc shell uinput -T -g 100 100 500 500", shell=True)
            # 触发截图
            subprocess.check_call("hdc shell uinput -T -m 1000 0 1000 1000 500 && hdc shell uinput -T -c 300 2300", shell=True)
            time.sleep(3)
            process_hilog.terminate()
            if process_hilog.poll() is None:
                process_name = "hilog | grep Dumper"
                CloseProcess(process_name)
            time.sleep(3)
            output = subprocess.check_output(f"hdc shell cat /data/local/tmp/screenshot.txt", text=True, encoding="utf-8")
            assert "dump fail!ret" not in output
            assert "dump success, cmd" in output