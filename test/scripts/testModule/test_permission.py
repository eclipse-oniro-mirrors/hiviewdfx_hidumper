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
import time
import subprocess
from utils import *

def CheckRedirectOutput(path):
    command = "hdc shell \"hidumper --mem 1 > " + path + "\""
    subprocess.check_call(command, shell=True)
    output = subprocess.check_output("hdc shell cat " + path, shell=True).decode()
    return output

class TestHidumperPermission:
    @pytest.mark.L3
    def test_output_append(self):
        if IsRootVersion():
            assert "Graph" in CheckRedirectOutput("/data/log/hidumper/mem1.txt")
            assert "Graph" in CheckRedirectOutput("/data/log/mem2.txt")
            output = CheckRedirectOutput("/data/local/tmp/mem3.txt")
            assert output.strip('\n') == ""
        else:
            pytest.skip("test only in root mode")
