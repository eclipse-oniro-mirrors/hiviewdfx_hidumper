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

OUTPUT_PATH = "testModule/output"
PSS_TOTAL_INDEX = 0
SWAP_PSS_INDEX = 6
COLUMN_NUM = 10

def ParseMemoryOutput(output):
    memory_data = {}
    for line in output.split("\n"):
        if re.search(r"\d", line) is None:
            continue
        key = re.search(r'\s*([a-zA-Z.]+(?:\s+[a-zA-Z.]+)*)\s*', line).group(1).strip();
        memory_data[key] = [int(val) for val in re.findall(r'\d+', line)]
    return memory_data

@print_check_result
def CheckTotalPss(memory_data):
    pss_sum = sum([val[0] for key,val in memory_data.items() if key in ["GL", "Graph", "guard", "native heap", "AnonPage other", "stack", ".so", ".ttf", "dev", "dmabuf", "FilePage other"]])
    return memory_data["Total"][PSS_TOTAL_INDEX] == (pss_sum + memory_data["Total"][SWAP_PSS_INDEX])

@print_check_result
def CheckDataLength(memory_data):
    GL_mem = memory_data.get("GL", [])
    assert len(GL_mem) == COLUMN_NUM, "GL memory data error"
    Graph_mem = memory_data.get("Graph", [])
    assert len(Graph_mem) == COLUMN_NUM, "Graph memory data error"
    guard_mem = memory_data.get("guard", [])
    assert len(guard_mem) == COLUMN_NUM, "Guard memory data error"
    native_heap_mem = memory_data.get("native heap", [])
    assert len(native_heap_mem) == COLUMN_NUM, "native heap memory data error"
    AnonPage_other = memory_data.get("AnonPage other", [])
    assert len(AnonPage_other) == COLUMN_NUM, "AnonPage other memory data error"
    stack_mem = memory_data.get("stack", [])
    assert len(stack_mem) == COLUMN_NUM, "stack memory data error"
    so_mem = memory_data.get(".so", [])
    assert len(so_mem) == COLUMN_NUM, ".so memory data error"
    dev_mem = memory_data.get("dev", [])
    assert len(dev_mem) == COLUMN_NUM, "dev memory data error"
    FilePage_other = memory_data.get("FilePage other", [])
    assert len(FilePage_other) == COLUMN_NUM, "FilePage other memory data error"
    Total_mem = memory_data.get("Total", [])
    assert len(Total_mem) == COLUMN_NUM, "Total memory data error"
    return True

@print_check_result
def CheckDmaGraphMem(memory_data):
    return memory_data["Dma"][0] == memory_data["Graph"][PSS_TOTAL_INDEX]

def CheckHidumperMemoryWithPidOutput(output):
    memory_data = ParseMemoryOutput(output)
    ret = all(check(memory_data) for check in [CheckDmaGraphMem, CheckTotalPss, CheckDataLength])
    return ret

def CheckHidumperMemoryWithoutPidOutput(output):
    graph = re.search(r"Graph\((\d+) kB\)", output).group(1)
    dma = re.search(r"DMA\((\d+) kB\)", output).group(1)
    return int(graph) == int(dma)

class TestHidumperMemory:
    @pytest.mark.L0
    def test_memory_all(self):
        command = f"hidumper --mem"
        # 校验命令行输出
        CheckCmd(command, CheckHidumperMemoryWithoutPidOutput)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckHidumperMemoryWithoutPidOutput)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckHidumperMemoryWithoutPidOutput)

    @pytest.mark.L0
    def test_memory_pid(self):
        processName = "render_service"
        pid = GetPidByProcessName(processName)
        command = f"hidumper --mem {pid}"
        # 校验命令行输出
        CheckCmd(command, CheckHidumperMemoryWithPidOutput)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckHidumperMemoryWithPidOutput)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckHidumperMemoryWithPidOutput)

