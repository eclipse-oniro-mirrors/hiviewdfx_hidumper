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

import UIAbility from '@ohos.app.ability.UIAbility';
import AbilityConstant from '@ohos.app.ability.AbilityConstant';
import Want from '@ohos.app.ability.Want';
import AbilityDelegatorRegistry from '@ohos.app.ability.abilityDelegatorRegistry';
import hilog from '@ohos.hilog';
import window from '@ohos.window';
import { Hypium } from '@ohos/hypium';
import testsuite from '../test/List.test';

const domain = 0x0000;
const tag = 'testTag';

export default class TestAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    hilog.info(domain, tag, '%{public}s', 'TestAbility onCreate');
    const abilityDelegator = AbilityDelegatorRegistry.getAbilityDelegator();
    const abilityDelegatorArguments = AbilityDelegatorRegistry.getArguments();
    hilog.info(domain, tag, '%{public}s', 'start run testcase!!!');
    Hypium.hypiumTest(abilityDelegator, abilityDelegatorArguments, testsuite);
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    hilog.info(domain, tag, '%{public}s', 'TestAbility onWindowStageCreate');
    windowStage.loadContent('TestAbility/pages/Index', (err) => {
      if (err.code) {
        hilog.error(domain, tag, 'Failed to load content: %{public}s', JSON.stringify(err));
      }
    });
  }
}
