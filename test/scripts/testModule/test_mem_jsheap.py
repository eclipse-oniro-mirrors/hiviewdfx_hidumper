#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (C) 2025 Huawei Device Co., Ltd.
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

def WaitUntillOutputAppear(command, targetStr, second):
    time.sleep(1)
    command = None
    if IsRootVersion():
        command = "hdc shell \"ls -l /data/log/faultlog/temp |grep jsheap\""
    else:
        command = "hdc shell \"ls -l /data/log/reliability/resource_leak/memory_leak |grep jsheap\""
    output = subprocess.check_output(command, shell=True, text=True, encoding="utf-8").strip()
    return output != ""

def PreOperationHap():
    # 单击【Enable】按钮
    TouchButtonByText("Enable")
    time.sleep(3)
    # 单击【2_全局变量未使用造成js内存泄露】按钮
    TouchButtonByText("2_全局变量未使用造成js内存泄露")
    time.sleep(3)
    command = "rm -rf /data/log/reliability/resource_leak/memory_leak/hidumper-*"
    output = subprocess.check_output(f"hdc shell \"{command}\"", shell=True, text=True, encoding="utf-8")

def ParseJsLeakListOutput():
    jsleakDir = "/data/log/reliability/resource_leak/memory_leak"
    if IsOpenHarmonyVersion() or not IsRootVersion():
        jsleakDir = "/data/log/faultlog/temp"
    command = f"ls {jsleakDir} |grep -e leaklist"
    output = subprocess.check_output(f"hdc shell \"{command}\"", shell=True, text=True, encoding="utf-8")
    assert "leaklist" in output
    filename = output.strip('\n')
    print(f"leaklist filename:{filename} \n")
    command = f"cat {jsleakDir}/" + filename
    output = subprocess.check_output(f"hdc shell \"{command}\"", shell=True, text=True, encoding="utf-8")
    output = output.strip('\n')
    return output

def ParseJsHeapOutput(hash_val, name, msg):
    jsleakDir = "/data/log/reliability/resource_leak/memory_leak"
    if IsOpenHarmonyVersion() or not IsRootVersion():
        jsleakDir = "/data/log/faultlog/temp"
    command = f"ls {jsleakDir} |grep -e jsheap"
    output = subprocess.check_output(f"hdc shell \"{command}\"", shell=True, text=True, encoding="utf-8")
    assert "jsheap" in output
    filename = output.strip('\n')
    print(f"jsheap filename:{filename} \n")
    command = f"cat {jsleakDir}/" + filename + " |grep -e " + str(hash_val) + " -e " + name + " -e " + msg
    output = subprocess.check_output(f"hdc shell \"{command}\"", shell=True, text=True, encoding="utf-8")
    return output

class TestHidumperMemoryJsheap:
    @classmethod
    def setup_class(cls):
        subprocess.check_call("hdc shell aa start -a EntryAbility -b com.example.jsleakwatcher", shell=True)

    @classmethod
    def teardown_class(cls):
        subprocess.check_call("hdc shell aa force-stop -b com.example.jsleakwatcher", shell=True)

    def teardown_method(self):
        if not IsOpenHarmonyVersion():
            subprocess.check_call("hdc shell \"rm -rf /data/log/reliability/resource_leak/memory_leak/*\"", shell=True)
        else:
            subprocess.check_call("hdc shell \"rm -rf /data/log/faultlog/temp/*\"", shell=True)

    @pytest.mark.L0
    def test_mem_jsheap(self):
        pid = None
        if IsOpenHarmonyVersion():
            pid = GetPidByProcessName("com.ohos.launcher")
        elif IsRootVersion():
            pid = GetPidByProcessName("com.ohos.sceneboard")
        else:
            pid = GetPidByProcessName("com.example.myapplication")
            if pid == "":
                pytest.skip("test application not found")
        command = f"hdc shell \"hidumper --mem-jsheap {pid}\""
        # 校验命令行输出
        subprocess.check_call(command, shell=True)
        ret = WaitUntillOutputAppear("hdc shell \"ls -l /data/log/faultlog/temp |grep jsheap\"", "jsheap", 10)
        if not ret:
            ret = WaitUntillOutputAppear("hdc shell \"ls -l /data/log/reliability/resource_leak/memory_leak |grep jsheap\"", "jsheap", 10)
        assert ret

    @pytest.mark.L0
    def test_mem_jsheap_T(self):
        pid = None
        if IsOpenHarmonyVersion():
            pid = GetPidByProcessName("com.ohos.launcher")
        elif IsRootVersion():
            pid = GetPidByProcessName("com.ohos.sceneboard")
        else:
            pid = GetPidByProcessName("com.example.myapplication")
            if pid == "":
                pytest.skip("test application not found")
        command = f"hdc shell \"hidumper --mem-jsheap {pid} -T {pid}\""
        # 校验命令行输出
        subprocess.check_call(command, shell=True)
        ret = WaitUntillOutputAppear("hdc shell \"ls -l /data/log/faultlog/temp |grep jsheap\"", "jsheap", 10)
        if not ret:
            ret = WaitUntillOutputAppear("hdc shell \"ls -l /data/log/reliability/resource_leak/memory_leak |grep jsheap\"", "jsheap", 10)
        assert ret

    @pytest.mark.L0
    def test_mem_jsheap_gc(self):
        pid = None
        if IsOpenHarmonyVersion():
            pid = GetPidByProcessName("com.ohos.launcher")
        elif IsRootVersion():
            pid = GetPidByProcessName("com.ohos.sceneboard")
        else:
            pid = GetPidByProcessName("com.example.myapplication")
            if pid == "":
                pytest.skip("test application not found")
        process_hilog = subprocess.Popen(['hdc', 'shell', 'hilog | grep ArkCompiler > /data/local/tmp/test_mem_jsheap_gc.txt'])
        command = f"hdc shell \"hidumper --mem-jsheap {pid} --gc\""
        # 校验命令行输出
        subprocess.check_call(command, shell=True)
        time.sleep(3)
        process_hilog.terminate()
        output = subprocess.check_output(f"hdc shell cat /data/local/tmp/test_mem_jsheap_gc.txt", text=True, encoding="utf-8")
        log = "TriggerGC tid 0 curTid " + str(pid)
        assert log in output

    @pytest.mark.L0
    def test_mem_jsheap_T_gc(self):
        pid = None
        if IsOpenHarmonyVersion():
            pid = GetPidByProcessName("com.ohos.launcher")
        elif IsRootVersion():
            pid = GetPidByProcessName("com.ohos.sceneboard")
        else:
            pid = GetPidByProcessName("com.example.myapplication")
            if pid == "":
                pytest.skip("test application not found")
        process_hilog = subprocess.Popen(['hdc', 'shell', 'hilog | grep ArkCompiler > /data/local/tmp/test_mem_jsheap_T_gc.txt'])
        command = f"hdc shell \"hidumper --mem-jsheap {pid} -T {pid} --gc\""
        # 校验命令行输出
        subprocess.check_call(command, shell=True)
        time.sleep(3)
        process_hilog.terminate()
        output = subprocess.check_output(f"hdc shell cat /data/local/tmp/test_mem_jsheap_T_gc.txt", text=True, encoding="utf-8")
        log = "TriggerGC tid " + str(pid) + " curTid " + str(pid)
        assert log in output

    @pytest.mark.L0
    def test_mem_jsheap_leakobj(self):
        PreOperationHap()
        pid = GetPidByProcessName("com.example.jsleakwatcher")
        if pid == "":
            pytest.skip("com.example.jsleakwatcher not found")
        command = f"hdc shell \"hidumper --mem-jsheap {pid} --leakobj\""
        subprocess.check_call(command, shell=True)
        time.sleep(3)
        # 解析leaklist文件
        output = ParseJsLeakListOutput()
        json_data = json.loads(output)
        hash_val = json_data[0]['hash']
        name = json_data[0]['name']
        msg = json_data[0]['msg']
        # 解析heapsnapshot文件，判断heapsnapshot中是否包含leaklist对象
        output = ParseJsHeapOutput(hash_val, name, msg)
        print(f"output:{output}\n")
        assert str(hash_val) in output
        assert "Int:" + str(hash_val) in output
        assert name in output
        assert msg in output

    @pytest.mark.L3
    def test_mem_jsheap_error_pid(self):
        command = f"hidumper --mem-jsheap 2147483647;hidumper --mem-jsheap -2147483647"
        hidumperTmpCmd = "OPT:mem-jsheap SUB_OPT:"
        # 校验命令行输出
        CheckCmd(command, lambda output : "hidumper: No such process: 2147483647\nhidumper: invalid arg: -2147483647" in output, hidumperTmpCmd)
        command = f"hidumper --mem-jsheap 2147483648;hidumper --mem-jsheap -2147483648"
        CheckCmd(command, lambda output : "hidumper: invalid arg: -2147483648" in output, hidumperTmpCmd)

