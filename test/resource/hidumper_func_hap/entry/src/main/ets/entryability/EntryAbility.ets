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
import { AbilityConstant, UIAbility, Want } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { window } from '@kit.ArkUI';
import jsLeakWatcher from '@ohos.hiviewdfx.jsLeakWatcher';

const DOMAIN = 0xFDF0;
const TAG = 'HidumperFuncTest';
/* GC probe garbage: 24576 chunks x ~1 KiB payload. Two-phase protocol used by the function UT:
 *   holdGarbage  - build the garbage and keep it referenced (it survives young collections and
 *                  gets promoted, so the heap-used reading grows and stays stable)
 *   dropGarbage  - release the reference; the promoted garbage can now only be reclaimed by a
 *                  full GC, which `hidumper --mem-jsheap <pid> --gc` triggers on demand.
 * The UT takes jsheap snapshots around the forced GC (S1 while holding, S2 after the drop) and
 * asserts the artifact size delta - the memory the full GC actually reclaimed. */
const GARBAGE_CHUNKS = 24576;
const GARBAGE_CHUNK_REPEAT = 50;
/* Leak probe for `hidumper --mem-jsheap <pid> --leakobj` (UT case JsHeap_Leakobj_043). The
 * command funnels jsLeakWatcher.check() into a faultloggerd "leaklist" artifact; check() only
 * reports objects registered through watch() that were never reclaimed. The probe below is
 * watched and held by a strong reference, so it is exactly that: a registered object that
 * survives every GC - a deterministic leak entry carrying the marker string the UT asserts
 * inside the artifact content. */
const LEAK_PROBE_MARKER = 'HidumperFuncLeakProbe';

interface GarbageItem {
  idx: number;
  tag: string;
}

class LeakProbe {
  marker: string = LEAK_PROBE_MARKER;
  payload: string = 'leak-probe-object-held-by-design';
}

export default class EntryAbility extends UIAbility {
  private static garbageHolder: GarbageItem[] | null = null;
  private static leakProbes: LeakProbe[] = [];

  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    hilog.info(DOMAIN, TAG, 'Ability onCreate');
    EntryAbility.setupLeakWatcher();
    this.handleTestAction(want);
  }

  onNewWant(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    hilog.info(DOMAIN, TAG, 'Ability onNewWant');
    this.handleTestAction(want);
  }

  /* Arms jsLeakWatcher once per process start: enable() first (watch() is a no-op while the
   * watcher is disabled), then register the probe. Kept in a static array so no GC can ever
   * finalize it - check() therefore always returns a non-empty list for --leakobj. */
  private static setupLeakWatcher(): void {
    if (EntryAbility.leakProbes.length > 0) {
      return;
    }
    try {
      jsLeakWatcher.enable(true);
      const probe = new LeakProbe();
      EntryAbility.leakProbes.push(probe);
      jsLeakWatcher.watch(probe, LEAK_PROBE_MARKER);
      hilog.info(DOMAIN, TAG, 'leak watcher armed, marker %{public}s', LEAK_PROBE_MARKER);
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'leak watcher setup failed: %{public}s', String(err));
    }
  }

  /* Driven by the function UT:
   *   aa start -b <bundle> -a EntryAbility --ps hdaction holdGarbage|dropGarbage|fillGarbage */
  private handleTestAction(want: Want): void {
    const params = want.parameters;
    if (params === undefined) {
      return;
    }
    const action = params['hdaction'] as string;
    if (action === 'holdGarbage') {
      EntryAbility.garbageHolder = EntryAbility.buildGarbage();
      hilog.info(DOMAIN, TAG, 'holdGarbage: holding %{public}d chunks',
        EntryAbility.garbageHolder.length);
    } else if (action === 'dropGarbage') {
      EntryAbility.garbageHolder = null;
      hilog.info(DOMAIN, TAG, 'dropGarbage: reference released');
    } else if (action === 'fillGarbage') {
      /* One-shot variant: build and drop immediately. */
      const garbage = EntryAbility.buildGarbage();
      hilog.info(DOMAIN, TAG, 'fillGarbage built %{public}d chunks, dropping reference',
        garbage.length);
    }
  }

  /* Every chunk is an object carrying a unique string so the forced full GC has both object
   * nodes and string bytes to reclaim, and the jsheap snapshot taken while holding them is
   * measurably larger than the one taken after dropGarbage + --gc (the UT compares artifact
   * sizes). A constant payload was verified to collapse via runtime string sharing. */
  private static buildGarbage(): GarbageItem[] {
    const garbage: GarbageItem[] = [];
    for (let i = 0; i < GARBAGE_CHUNKS; i++) {
      garbage.push({ idx: i, tag: (i.toString() + '-gcprobe-').repeat(GARBAGE_CHUNK_REPEAT) });
    }
    return garbage;
  }

  onDestroy(): void {
    hilog.info(DOMAIN, TAG, 'Ability onDestroy');
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    windowStage.loadContent('pages/Index', (err) => {
      if (err.code) {
        hilog.error(DOMAIN, TAG, 'loadContent failed: %{public}s', JSON.stringify(err));
        return;
      }
      hilog.info(DOMAIN, TAG, 'loadContent succeeded');
    });
  }

  onWindowStageDestroy(): void {
    hilog.info(DOMAIN, TAG, 'onWindowStageDestroy');
  }

  onForeground(): void {
    hilog.info(DOMAIN, TAG, 'onForeground');
  }

  onBackground(): void {
    hilog.info(DOMAIN, TAG, 'onBackground');
  }
}
