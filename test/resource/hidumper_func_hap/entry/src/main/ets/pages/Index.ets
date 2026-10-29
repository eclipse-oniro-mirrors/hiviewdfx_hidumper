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
import { webview } from '@kit.ArkWeb';
import { hilog } from '@kit.PerformanceAnalysisKit';
import hidumperTarget from 'libhidumper_target.so';

const DOMAIN = 0xFDF0;
const TAG = 'HidumperFuncTest';
const BALLAST_BATCH = 32;
const BALLAST_CAP = 8192;
const BALLAST_TRIM = 4096;
const BALLAST_INTERVAL_MS = 500;

interface HidumperTargetNative {
  start(): boolean;
  status(): string;
}

const target: HidumperTargetNative = hidumperTarget as HidumperTargetNative;

/**
 * Target page for hidumper function UT:
 *  - Web component       -> spawns the ArkWeb :render process (--mem-heap --arkweb-js target)
 *  - native start()      -> JSVM isolate + malloc churn + KMP listener (--jsvm/--native/--kotlin targets)
 *  - ArkTS heap ballast  -> keeps the ArkTS heap populated (--mem-jsheap / --raw / --leakobj target)
 */
@Entry
@Component
struct Index {
  private controller: webview.WebviewController = new webview.WebviewController();
  @State statusText: string = 'starting targets...';
  private arkHeapBallast: string[] = [];

  aboutToAppear(): void {
    target.start();
    this.statusText = target.status();
    hilog.info(DOMAIN, TAG, 'native target status: %{public}s', this.statusText);
    setInterval(() => {
      for (let i = 0; i < BALLAST_BATCH; i++) {
        this.arkHeapBallast.push('arkts-heap-ballast-' + this.arkHeapBallast.length.toString());
      }
      if (this.arkHeapBallast.length > BALLAST_CAP) {
        this.arkHeapBallast.splice(0, BALLAST_TRIM);
      }
      this.statusText = target.status();
    }, BALLAST_INTERVAL_MS);
  }

  build() {
    Column() {
      Text('HidumperFuncTestHap')
        .fontSize(20)
        .fontWeight(FontWeight.Bold)
        .margin(8)
      Text(this.statusText)
        .fontSize(12)
        .margin(4)
      Web({ src: $rawfile('web/index.html'), controller: this.controller })
        .width('100%')
        .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
  }
}
