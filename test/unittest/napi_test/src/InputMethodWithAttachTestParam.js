/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

import inputMethod from '@ohos.inputMethod';
import commonEventManager from '@ohos.commonEventManager';
import inputMethodEngine from '@ohos.inputMethodEngine'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index';

describe('InputMethodWithAttachTestParam', function () {
  const WAIT_DEAL_OK = 500;
  const TEST_RESULT_CODE = 0;
  beforeAll(async function (done) {
    console.info('beforeAll called');
    let inputMethodProperty = {
      name: 'om.example.newTestIme',
      id: 'InputMethodExtAbility'
    };
    await inputMethod.switchInputMethod(inputMethodProperty);
    setTimeout(() => {
      done();
    }, WAIT_DEAL_OK);
  });

  afterAll(async function () {
  });

  beforeEach(async function () {
    console.info('beforeEach called');
  });

  afterEach(async function () {
    console.info('afterEach called');
  });

   /*
   * @tc.number  inputmethod_test_attach_palceholder_and_AbilityName_001
   * @tc.name    Test whether the current application can be bound with the default input method.
   * @tc.desc    Function test
   * @tc.level   2
   */
   it('inputmethod_test_attach_palceholder_and_AbilityName_001', 0, async function (done) {
    console.info('************* inputmethod_test_attach_palceholder_and_AbilityName_001 Test start*************');

    let inputMethodCtrl = inputMethod.getController();
    var palceholder = 'palceholder';
    var abilityName = 'ability name 👨';
    let cfg = {
      inputAttribute:
        {
          textInputType: inputMethod.TextInputType.TEXT,
          enterKeyType: inputMethod.EnterKeyType.NONE,
          palceholder: palceholder,
          abilityName: abilityName,
        }
    };
    let timeOutCb1 = async () => {
      commonEventManager.createSubscriber(subscribeInfo).then((data) => {
        let subscriber = data;
        commonEventManager.subscribe(subscriber, (err, eventData) => {
          console.info("inputmethod_test_attach_palceholder_and_AbilityName_001 subscribe");
          let recv = JSON.parse(eventData);
          if (palceholder === recv.palceholder && abilityName === recv.abilityName) {
            expect(true).assertTrue();
          } else {
            expect().assertFail();
          }
          commonEventManager.unsubscribe(subscriber);
          done();
        })
      });
      console.info(`inputmethod_test_attach_palceholder_and_AbilityName_001 begin attach`);
      await inputMethodCtrl.attach(false, cfg);
      console.info(`inputmethod_test_attach_palceholder_and_AbilityName_001 end attach`);
    }
    try {
      setTimeout(timeOutCb1, 500);
    } catch(error) {
      console.info(`inputmethod_test_attach_palceholder_and_AbilityName_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });
});