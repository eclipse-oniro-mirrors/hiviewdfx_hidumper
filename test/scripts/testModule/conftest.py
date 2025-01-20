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
import subprocess
import time

def IsLogVersion():
    output = subprocess.check_output("hdc shell param get const.product.software.version", shell=True, text=True, encoding="utf-8").strip()
    return "log" in output

def IsRootVersion():
    output = subprocess.check_output("hdc shell param get const.debuggable", shell=True, text=True, encoding="utf-8").strip()
    return output == "1"

def IsOpenHarmonyVersion():
    output = subprocess.check_output("hdc shell param get const.product.software.version", shell=True, text=True, encoding="utf-8").strip()
    return "OpenHarmony" in output

def pytest_sessionstart(session):
    print("start install jsleakwatcher.hap")
    subprocess.check_call("hdc install testModule/resource/jsleakwatcher.hap", shell=True)

def pytest_sessionfinish(session, exitstatus):
    print("start uinstall jsleakwatcher.hap")
    subprocess.check_call("hdc uninstall com.example.jsleakwatcher", shell=True)