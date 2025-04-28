/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index';

describe('InputMethodWithAttachTestParam', function () {
  const WAIT_DEAL_OK = 500;

  function wait(delay) {
    let start = new Date().getTime();
    while (new Date().getTime() - start < delay) {
    }
  }

  beforeAll(async function (done) {
    console.info('InputMethodWithAttachTestParam beforeAll called');
    let inputMethodProperty = {
      name: 'com.example.testIme',
      id: 'InputMethodExtAbility'
    };
    try {
      inputMethod.switchInputMethod(inputMethodProperty).then(() => {
        console.log('InputMethodWithAttachTestParam Succeeded in switching inputmethod.');
      }).catch((err) => {
        console.error(`InputMethodWithAttachTestParam Failed to switchInputMethod: ${JSON.stringify(err)}`);
      })
    } catch (err) {
      console.error(`InputMethodWithAttachTestParam Failed to switchInputMethod: ${JSON.stringify(err)}`);
    }
    wait(WAIT_DEAL_OK);
    done();
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
   it('inputmethod_test_attach_palceholder_and_ability_name_001', 0, async function (done) {
    console.info('************* inputmethod_test_attach_palceholder_and_ability_name_001 Test start*************');

    let inputMethodCtrl = inputMethod.getController();
    var palceholder = 'palceholder';
    var abilityName = 'ability name 👨';
    let cfg = {
      inputAttribute:
        {
          textInputType: inputMethod.TextInputType.TEXT,
          enterKeyType: inputMethod.EnterKeyType.NONE,
          placeholder: palceholder,
          abilityName: abilityName,
        }
    };
    let timeOutCb1 = async () => {
      let subscribeInfo = {
        events: ['EditorAttributeChangedTest']
      };
      console.info(`EditorAttributeChangedTest subscribe begin`);
      commonEventManager.createSubscriber(subscribeInfo).then((data) => {
          let subscriber = data;
          console.info(`EditorAttributeChangedTest subscribe end`);
          commonEventManager.subscribe(subscriber, (err, eventData) => {
            console.info(`EditorAttributeChangedTest subscribe recv`);
            if (err) {
              console.info(`EditorAttributeChangedTest fail:${JSON.stringify(err)}`);
              expect().assertFail();
              done();
              return;
            }
            console.info(`EditorAttributeChangedTest:${eventData},code:${eventData.code}`);
            if (eventData.code == 0) {
              return;
            }
            console.info(`EditorAttributeChangedTest eventData.data:${eventData.data}`);
            let recv = JSON.parse(`${eventData.data}`);
            console.info(`EditorAttributeChangedTest recv:${recv}, palceholder:${recv.placeholder}`);
            if (palceholder === recv.placeholder && abilityName === recv.abilityName) {
              expect(true).assertTrue();
              console.info(`inputmethod_test_attach_palceholder_and_ability_name_001 true`);
            } else {
              console.info(`inputmethod_test_attach_palceholder_and_ability_name_001 fail`);
              expect().assertFail();
            }
            commonEventManager.unsubscribe(subscriber);
            console.info(`inputmethod_test_attach_palceholder_and_ability_name_001 unsubscribe`);
            done();
        })
      });
      console.info(`inputmethod_test_attach_palceholder_and_ability_name_001 begin attach`);
      inputMethodCtrl.attach(false, cfg);
      console.info(`inputmethod_test_attach_palceholder_and_ability_name_001 end attach`);
    }
    try {
      setTimeout(timeOutCb1, 1000);
    } catch(error) {
      console.info(`inputmethod_test_attach_palceholder_and_ability_name_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info(`inputmethod_test_attach_palceholder_and_ability_name_001 end`);
  });
});