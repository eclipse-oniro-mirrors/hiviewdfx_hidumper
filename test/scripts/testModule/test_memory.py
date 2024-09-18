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
PSS_TOTAL_INDEX = 0
SWAP_PSS_INDEX = 6
COLUMN_NUM = 10

def parse_memory_output(output):
    memory_data = {}
    for line in output.split("\n"):
        if re.search(r"\d", line) is None:
            continue
        key = re.search(r'\s*([a-zA-Z.]+(?:\s+[a-zA-Z.]+)*)\s*', line).group(1);
        memory_data[key] = [int(val) for val in re.findall(r'\d+', line)]
    return memory_data

@print_check_result
def check_total_pss(memory_data):
    pss_sum = sum([val[0] for key,val in memory_data.items() if key in ["GL", "Graph", "guard", "native heap", "AnonPage other", "stack", ".so", "dev", "dmabuf", "FilePage other"]])
    return memory_data["Total"][PSS_TOTAL_INDEX] == (pss_sum + memory_data["Total"][SWAP_PSS_INDEX])

@print_check_result
def check_data_length(memory_data):
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
    dmabuf = memory_data.get("dmabuf", [])
    assert len(dmabuf) == COLUMN_NUM, "dmabuf memory data error"
    FilePage_other = memory_data.get("FilePage other", [])
    assert len(FilePage_other) == COLUMN_NUM, "FilePage other memory data error"
    Total_mem = memory_data.get("Total", [])
    assert len(Total_mem) == COLUMN_NUM, "Total memory data error"
    return True

@print_check_result
def check_dma_graph_mem(memory_data):
    return memory_data["Dma"][0] == memory_data["Graph"][PSS_TOTAL_INDEX]

def check_hidumper_mem_pid_output(output):
    memory_data = parse_memory_output(output)
    results = [check(memory_data) for check in [check_dma_graph_mem, check_total_pss]]
    return all(results)

class TestHidumperMemory:
    @pytest.mark.L0
    def test_memory_all(self):
        pass

    @pytest.mark.L0
    def test_memory_pid(self):
        processName = "render_service"
        pid = get_pid_by_process_name(processName)
        if pid is None:
            pytest.skip(f"Can not find process {processName}")
        # 校验命令行输出
        output = subprocess.check_output(f"hdc shell hidumper --mem {pid}", shell=True, text=True, encoding="utf-8")
        assert check_output(output, check_function = check_hidumper_mem_pid_output)

        # 校验命令行重定向输出
        redirect_file = f"{OUTPUT_PATH}/redirect.txt"
        subprocess.check_output(f"hdc shell \"hidumper --mem {pid} \"  > {redirect_file}", shell=True, text=True, encoding="utf-8")
        assert check_file(redirect_file, check_function = check_hidumper_mem_pid_output)

        # 校验命令行输出到zip文件
        output = subprocess.check_output(f"hdc shell \"hidumper --mem {pid} --zip\"", shell=True, text=True, encoding="utf-8")
        zip_source_file = re.search("The result is:(.+)", output).group(1)
        zip_target_file = f"{OUTPUT_PATH}/" + os.path.basename(zip_source_file)
        subprocess.check_output(f"hdc file recv {zip_source_file} {zip_target_file}", shell=True, text=True, encoding="utf-8")
        assert check_zip_file(zip_target_file, check_function = check_hidumper_mem_pid_output)