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
import threading
import time
import json
from utils import *

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

# 要并发执行的任务函数
def ThreadTask(thread_name, delay):
    print(f"Thread {thread_name} starting")
    command = "hidumper --mem > /data/log/hidumper/" + thread_name
    output = subprocess.check_output(f"hdc shell \"{command}\"", shell=True, text=True, encoding="utf-8")
    print(f"Thread {thread_name} finished")

def IsRequestErrorInOutput():
    flag = 0
    for i in range(6):
        command = "cat /data/log/hidumper/Thread-" + str(i)
        output = subprocess.check_output(f"hdc shell \"{command}\"", shell=True, text=True, encoding="utf-8")
        if "request error" in output:
            command = "lsof |grep Thread-" + str(i)
            output = subprocess.check_output(f"hdc shell \"{command}\"", shell=True, text=True, encoding="utf-8")
            print(f"output:{output}\n")
            assert "Thread-" + str(i) not in output
            flag = 1
            break
    return flag

class TestHidumperMemory:
    @pytest.mark.L0
    def test_memory_all(self):
        command = f"hidumper --mem"
        hidumperTmpCmd = "OPT:mem SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckHidumperMemoryWithoutPidOutput, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckHidumperMemoryWithoutPidOutput, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckHidumperMemoryWithoutPidOutput)

    @pytest.mark.L0
    def test_memory_pid(self):
        processName = "render_service"
        pid = GetPidByProcessName(processName)
        command = f"hidumper --mem {pid}"
        hidumperTmpCmd = "OPT:mem SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, CheckHidumperMemoryWithPidOutput, hidumperTmpCmd)
        # 校验命令行重定向输出
        CheckCmdRedirect(command, CheckHidumperMemoryWithPidOutput, None, hidumperTmpCmd)
        # 校验命令行输出到zip文件
        CheckCmdZip(command, CheckHidumperMemoryWithPidOutput)

    @pytest.mark.L3
    def test_memory_error_pid(self):
        command = f"hidumper --mem 2147483647;hidumper --mem -2147483647"
        hidumperTmpCmd = "OPT:mem SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, lambda output : "hidumper: No such process: 2147483647\nhidumper: option pid missed. 2" in output, hidumperTmpCmd)
        command = f"hidumper --mem 2147483648;hidumper --mem -2147483648"
        CheckCmd(command, lambda output : "hidumper: option pid missed. 2" in output, hidumperTmpCmd)

    @pytest.mark.L1
    def test_memory_note(self):
        if not IsOpenHarmonyVersion():
            pytest.skip("this testcase is only support in OH")
        else:
            # 唤醒屏幕
            subprocess.check_call("hdc shell power-shell wakeup", shell=True)
            # 设置屏幕常亮
            subprocess.check_call("hdc shell power-shell setmode 602", shell=True)
            time.sleep(3)
            # 解锁屏幕
            subprocess.check_call("hdc shell uinput -T -g 100 100 500 500", shell=True)
            time.sleep(3)
            # 启动备忘录应用
            subprocess.check_call("hdc shell aa start -a MainAbility -b com.ohos.note", shell=True)
            time.sleep(3)
            output = subprocess.check_output(f"hdc shell pidof com.ohos.note", shell=True, text=True, encoding="utf-8")
            note_pid = output.strip('\n')
            assert len(note_pid) != 0
            # 新建备忘录
            subprocess.check_call("hdc shell uinput -S -c 620 100", shell=True)
            time.sleep(3)
            output = subprocess.check_output(f"hdc shell pidof com.ohos.note:render", shell=True, text=True, encoding="utf-8")
            note_render_pid = output.strip('\n')
            assert len(note_render_pid) != 0
            command = f"hidumper --mem {note_render_pid}"
            # 校验命令行输出
            CheckCmd(command, CheckHidumperMemoryWithPidOutput)
            # 校验命令行重定向输出
            CheckCmdRedirect(command, CheckHidumperMemoryWithPidOutput)
            # 校验命令行输出到zip文件
            CheckCmdZip(command, CheckHidumperMemoryWithPidOutput)

    @pytest.mark.L1
    def test_memory_mutilthread(self):
        # 创建线程列表
        threads = []
        # 创建线程并启动
        for i in range(6):
            t = threading.Thread(target=ThreadTask, args=("Thread-{}".format(i), i))
            threads.append(t)
            t.start()
        # 等待所有线程完成
        for t in threads:
            t.join()
        # 校验超过请求数5的命令对应的fd中包含request error信息
        assert IsRequestErrorInOutput()

