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

import hilog from '@ohos.hilog';
import TestRunner from '@ohos.application.testRunner';
import AbilityDelegatorRegistry from '@ohos.app.ability.abilityDelegatorRegistry';
import { BusinessError } from '@ohos.base';

const domain = 0x0000;
const tag = 'testTag';

let abilityDelegator: AbilityDelegatorRegistry.AbilityDelegator;
let abilityDelegatorArguments: AbilityDelegatorRegistry.AbilityDelegatorArgs;

async function OnAbilityCreateCallback(): Promise<void> {
  hilog.info(domain, tag, '%{public}s', 'onAbilityCreateCallback');
}

async function AddAbilityMonitorCallback(err: BusinessError): Promise<void> {
  hilog.info(domain, tag, 'addAbilityMonitorCallback : %{public}s', JSON.stringify(err) ?? '');
}

export default class OpenHarmonyTestRunner implements TestRunner {
  constructor() {
  }

  onPrepare(): void {
    hilog.info(domain, tag, '%{public}s', 'OpenHarmonyTestRunner OnPrepare');
  }

  async onRun(): Promise<void> {
    hilog.info(domain, tag, '%{public}s', 'OpenHarmonyTestRunner onRun run');
    abilityDelegatorArguments = AbilityDelegatorRegistry.getArguments();
    abilityDelegator = AbilityDelegatorRegistry.getAbilityDelegator();
    const testAbilityName = abilityDelegatorArguments.bundleName + '.TestAbility';
    const abilityMonitor: AbilityDelegatorRegistry.AbilityMonitor = {
      abilityName: testAbilityName,
      onAbilityCreate: OnAbilityCreateCallback,
    };
    abilityDelegator.addAbilityMonitor(abilityMonitor, AddAbilityMonitorCallback);
    const cmd = 'aa start -d 0 -a TestAbility -b ' + abilityDelegatorArguments.bundleName;
    hilog.info(domain, tag, 'cmd : %{public}s', cmd);
    abilityDelegator.executeShellCommand(cmd,
      (err: BusinessError, data: AbilityDelegatorRegistry.ShellCmdResult): void => {
        hilog.info(domain, tag, 'executeShellCommand : err : %{public}s', JSON.stringify(err) ?? '');
        hilog.info(domain, tag, 'executeShellCommand : data : %{public}s', data.stdResult ?? '');
      });
    hilog.info(domain, tag, '%{public}s', 'OpenHarmonyTestRunner onRun end');
  }
}
