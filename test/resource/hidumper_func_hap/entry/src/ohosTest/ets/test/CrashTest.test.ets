/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { describe, it } from '@ohos/hypium';
import targetNapi from 'libhidumper_target.so';

/**
 * Crash-trigger cases driven from the native hidumper function UT through:
 *   aa test -b ohos.samples.hidumperfunc -m entry_test \
 *     -s unittest /ets/testrunner/OpenHarmonyTestRunner -s class CrashTest#<case>
 * Each case terminates the test process with a real fault so that faultloggerd
 * provisions a cppcrash record, which hidumper -e --list / -e --print then consume.
 * Pattern follows faultlogger_hap (hiviewdfx_hiview PR !4552).
 */
export default function crashTest() {
  describe('CrashTest', () => {
    it('SegvCase', 0, () => {
      targetNapi.TriggerSegv();
    });
  });
}
