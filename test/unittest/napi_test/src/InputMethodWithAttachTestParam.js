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
  const MAX_PLACEHOLDER_CHAR_SIZE = 255;
  const MAX_ABILITY_NAME_CHAR_SIZE = 127;
  const REPEAT_SIZE = 10;

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
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.detach();
  });

  function truncateByCodePoints(str, maxCodePoints) {
    let codePoints = 0;
    let position = 0;

    while (codePoints < maxCodePoints && position < str.length) {
      const codeUnit = str.charCodeAt(position);
      codePoints++;
      // Determine whether it is a High Surrogate
      if (codeUnit >= 0xD800 && codeUnit <= 0xDBFF) {
        position += 2;
      } else {
        position += 1;
      }
    }
    // Check if the High Surrogate is truncated at the end
    const lastCodeUnit = str.charCodeAt(position - 1);
    if (lastCodeUnit >= 0xD800 && lastCodeUnit <= 0xDBFF) {
      position -= 1; // Remove incomplete High Surrogate
    }
    return str.slice(0, position);
  }

  /*
   * @tc.number  inputmethod_test_attach_palceholder_and_AbilityName_001
   * @tc.name    test interface attach config parameters palceholder and ableName
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_attach_palceholder_and_ability_name_001', 0, async function (done) {
    let testName = 'inputmethod_test_attach_palceholder_and_ability_name_001';
    console.info(`************* ${testName} Test start*************`);
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
      console.info(`${testName} subscribe begin`);
      commonEventManager.createSubscriber(subscribeInfo).then((data) => {
          let subscriber = data;
          console.info(`${testName} subscribe end`);
          commonEventManager.subscribe(subscriber, (err, eventData) => {
            console.info(`${testName} subscribe recv`);
            if (err) {
              console.info(`${testName} fail:${JSON.stringify(err)}`);
              expect().assertFail();
              commonEventManager.unsubscribe(subscriber);
              console.info(`${testName} unsubscribe`);
              done();
              return;
            }
            console.info(`${testName}:${eventData},code:${eventData.code}`);
            if (eventData.code == 0) {
              return;
            }
            console.info(`${testName} eventData.data:${eventData.data}`);
            let recv = JSON.parse(`${eventData.data}`);
            console.info(`${testName} recv:${recv}, palceholder:${recv.placeholder}`);
            if (palceholder === recv.placeholder && abilityName === recv.abilityName) {
              expect(true).assertTrue();
              console.info(`${testName} result true`);
            } else {
              console.info(`${testName} result fail`);
              expect().assertFail();
            }
            commonEventManager.unsubscribe(subscriber);
            console.info(`${testName} unsubscribe`);
            done();
        })
      });
      console.info(`${testName} begin attach`);
      inputMethodCtrl.attach(false, cfg);
      console.info(`${testName} end attach`);
    }
    try {
      setTimeout(timeOutCb1, WAIT_DEAL_OK * 2);
    } catch(error) {
      console.info(`${testName} result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info(`${testName} end`);
  });

  /*
   * @tc.number  inputmethod_test_attach_palceholder_and_AbilityName_002
   * @tc.name    test interface attach config parameters palceholder and ableName
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_attach_palceholder_and_ability_name_002', 0, async function (done) {
    let testName = 'inputmethod_test_attach_palceholder_and_ability_name_002';
    console.info(`************* ${testName} Test start*************`);
    let inputMethodCtrl = inputMethod.getController();
    var palceholder = truncateByCodePoints("palceholder𪛊TestSizeNeedGT255".repeat(REPEAT_SIZE),
      MAX_PLACEHOLDER_CHAR_SIZE)
    var abilityName = truncateByCodePoints('abilityName\0𪛊TestSizeNeedGT127'.repeat(REPEAT_SIZE),
      MAX_ABILITY_NAME_CHAR_SIZE)
    let cfg = {
      inputAttribute:
        {
          textInputType: inputMethod.TextInputType.TEXT,
          enterKeyType: inputMethod.EnterKeyType.NONE,
          placeholder: palceholder + "a",
          abilityName: abilityName + "b",
        }
    };
    let timeOutCb1 = async () => {
      let subscribeInfo = {
        events: ['EditorAttributeChangedTest']
      };
      console.info(` ${testName} subscribe begin`);
      commonEventManager.createSubscriber(subscribeInfo).then((data) => {
          let subscriber = data;
          console.info(`${testName} subscribe end`);
          commonEventManager.subscribe(subscriber, (err, eventData) => {
            console.info(`${testName} subscribe recv`);
            if (err) {
              console.info(`${testName} fail:${JSON.stringify(err)}`);
              expect().assertFail();
              commonEventManager.unsubscribe(subscriber);
              console.info(`${testName} unsubscribe`);
              done();
              return;
            }
            console.info(`${testName}:${eventData},code:${eventData.code}`);
            if (eventData.code == 0) {
              return;
            }
            console.info(`${testName} eventData.data:${eventData.data}`);
            let recv = JSON.parse(`${eventData.data}`);
            console.info(`${testName} recv:${recv}, palceholder:${recv.placeholder}`);
            if (palceholder === recv.placeholder && abilityName === recv.abilityName) {
              expect(true).assertTrue();
              console.info(`${testName} result true`);
            } else {
              console.info(`${testName} result fail`);
              expect().assertFail();
            }
            commonEventManager.unsubscribe(subscriber);
            console.info(`${testName} unsubscribe`);
            done();
        })
      });
      console.info(`${testName} begin attach`);
      inputMethodCtrl.attach(false, cfg, (err) => {
        if (err) {
          console.error(`${testName} Failed to attach: ${JSON.stringify(err)}`);
          expect().assertFail();
          done();
          return;
        }
        console.log(`${testName} Succeeded in attaching the inputMethod.`);
      });
      console.info(`${testName} end attach`);
    }
    try {
      setTimeout(timeOutCb1, WAIT_DEAL_OK * 2);
    } catch(error) {
      console.info(`${testName} result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info(`${testName} end`);
  });

  /*
   * @tc.number  inputmethod_test_attach_palceholder_and_AbilityName_003
   * @tc.name    test interface attach config parameters palceholder and ableName
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_attach_palceholder_and_ability_name_003', 0, async function (done) {
    let testName = 'inputmethod_test_attach_palceholder_and_ability_name_003';
    console.info(`************* ${testName} Test start*************`);
    let inputMethodCtrl = inputMethod.getController();
    var palceholder = truncateByCodePoints("palceholder𪛊TestSize255aaaa".repeat(REPEAT_SIZE),
      MAX_PLACEHOLDER_CHAR_SIZE);
    var abilityName = truncateByCodePoints('ability𪛊Name\0Need127'.repeat(REPEAT_SIZE),
      MAX_ABILITY_NAME_CHAR_SIZE);
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
      console.info(`${testName} subscribe begin`);
      commonEventManager.createSubscriber(subscribeInfo).then((data) => {
          let subscriber = data;
          console.info(` ${testName} subscribe end`);
          commonEventManager.subscribe(subscriber, (err, eventData) => {
            console.info(`${testName} subscribe recv`);
            if (err) {
              console.info(`${testName} fail:${JSON.stringify(err)}`);
              expect().assertFail();
              commonEventManager.unsubscribe(subscriber);
              console.info(`${testName} unsubscribe`);
              done();
              return;
            }
            console.info(`${testName}:${eventData},code:${eventData.code}`);
            if (eventData.code == 0) {
              return;
            }
            console.info(`${testName} eventData.data:${eventData.data}`);
            let recv = JSON.parse(`${eventData.data}`);
            console.info(`${testName} recv:${recv}, palceholder:${recv.placeholder}`);
            if (palceholder === recv.placeholder && abilityName === recv.abilityName) {
              expect(true).assertTrue();
              console.info(`${testName} result true`);
            } else {
              console.info(`${testName} result fail`);
              expect().assertFail();
            }
            commonEventManager.unsubscribe(subscriber);
            console.info(`${testName} unsubscribe`);
            done();
        })
      });
      console.info(`${testName} begin attach`);
      inputMethodCtrl.attach(false, cfg);
      console.info(`${testName} end attach`);
    }
    try {
      setTimeout(timeOutCb1, WAIT_DEAL_OK * 2);
    } catch(error) {
      console.info(` ${testName} result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info(` ${testName} end`);
  });

  /*
   * @tc.number  inputmethod_test_attach_palceholder_and_AbilityName_004
   * @tc.name    test interface attach config parameters palceholder and ableName
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_attach_palceholder_and_ability_name_004', 0, async function (done) {
    let testName = 'inputmethod_test_attach_palceholder_and_ability_name_004' ;
    console.info(`************* ${testName} Test start*************`);
    let inputMethodCtrl = inputMethod.getController();
    let cfg = {
      inputAttribute:
        {
          textInputType: inputMethod.TextInputType.TEXT,
          enterKeyType: inputMethod.EnterKeyType.NONE,
          placeholder: null,
          abilityName: null,
        }
    };
    let timeOutCb1 = async () => {
      let subscribeInfo = {
        events: ['EditorAttributeChangedTest']
      };
      console.info(`${testName} subscribe begin`);
      commonEventManager.createSubscriber(subscribeInfo).then((data) => {
          let subscriber = data;
          console.info(` ${testName} subscribe end`);
          commonEventManager.subscribe(subscriber, (err, eventData) => {
            console.info(`${testName} subscribe recv`);
            if (err) {
              console.info(`${testName} fail:${JSON.stringify(err)}`);
              expect().assertFail();
              commonEventManager.unsubscribe(subscriber);
              console.info(` ${testName} unsubscribe`);
              done();
              return;
            }
            console.info(`${testName}:${eventData},code:${eventData.code}`);
            if (eventData.code == 0) {
              return;
            }
            console.info(`${testName} eventData.data:${eventData.data}`);
            let recv = JSON.parse(`${eventData.data}`);
            console.info(`${testName} recv:${recv}, palceholder:${recv.placeholder}`);
            if ( recv.placeholder === '' && recv.abilityName === '') {
              expect(true).assertTrue();
              console.info(` ${testName} true`);
            } else {
              console.info(` ${testName} fail`);
              expect().assertFail();
            }
            commonEventManager.unsubscribe(subscriber);
            console.info(` ${testName} unsubscribe`);
            done();
        })
      });
      console.info(`${testName} begin attach`);
      inputMethodCtrl.attach(false, cfg);
      console.info(`${testName} end attach`);
    }
    try {
      setTimeout(timeOutCb1, WAIT_DEAL_OK * 2);
    } catch(error) {
      console.info(`${testName} result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info(` ${testName} end`);
  });

  /*
   * @tc.number  inputmethod_test_attach_palceholder_and_AbilityName_005
   * @tc.name    test interface attach config parameters palceholder and ableName
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_attach_palceholder_and_ability_name_005', 0, async function (done) {
    let testName = 'inputmethod_test_attach_palceholder_and_ability_name_005' ;
    console.info(`************* ${testName} Test start*************`);
    let inputMethodCtrl = inputMethod.getController();
    let cfg = {
      inputAttribute:
        {
          textInputType: inputMethod.TextInputType.TEXT,
          enterKeyType: inputMethod.EnterKeyType.NONE,
          placeholder: undefined,
          abilityName: undefined,
        }
    };
    let timeOutCb1 = async () => {
      let subscribeInfo = {
        events: ['EditorAttributeChangedTest']
      };
      console.info(`${testName} subscribe begin`);
      commonEventManager.createSubscriber(subscribeInfo).then((data) => {
          let subscriber = data;
          console.info(` ${testName} subscribe end`);
          commonEventManager.subscribe(subscriber, (err, eventData) => {
            console.info(`${testName} subscribe recv`);
            if (err) {
              console.info(`${testName} fail:${JSON.stringify(err)}`);
              expect().assertFail();
              commonEventManager.unsubscribe(subscriber);
              console.info(` ${testName} unsubscribe`);
              done();
              return;
            }
            console.info(`${testName}:${eventData},code:${eventData.code}`);
            if (eventData.code == 0) {
              return;
            }
            console.info(`${testName} eventData.data:${eventData.data}`);
            let recv = JSON.parse(`${eventData.data}`);
            console.info(`${testName} recv:${recv}, palceholder:${recv.placeholder}`);
            if (recv.placeholder === '' && recv.abilityName === '') {
              expect(true).assertTrue();
              console.info(` ${testName} true`);
            } else {
              console.info(` ${testName} fail`);
              expect().assertFail();
            }
            commonEventManager.unsubscribe(subscriber);
            console.info(` ${testName} unsubscribe`);
            done();
        })
      });
      console.info(`${testName} begin attach`);
      inputMethodCtrl.attach(false, cfg);
      console.info(`${testName} end attach`);
    }
    try {
      setTimeout(timeOutCb1, WAIT_DEAL_OK * 2);
    } catch(error) {
      console.info(` ${testName} result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info(` ${testName} end`);
  });

  /*
   * @tc.number  inputmethod_test_attach_palceholder_and_AbilityName_006
   * @tc.name    test interface attach config parameters palceholder and ableName
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_attach_palceholder_and_ability_name_006', 0, async function (done) {
    let testName = 'inputmethod_test_attach_palceholder_and_ability_name_006' ;
    console.info(`************* ${testName} Test start*************`);
    let inputMethodCtrl = inputMethod.getController();
    let cfg = {
      inputAttribute:
        {
          textInputType: inputMethod.TextInputType.TEXT,
          enterKeyType: inputMethod.EnterKeyType.NONE,
        }
    };
    let timeOutCb1 = async () => {
      let subscribeInfo = {
        events: ['EditorAttributeChangedTest']
      };
      console.info(`${testName} subscribe begin`);
      commonEventManager.createSubscriber(subscribeInfo).then((data) => {
          let subscriber = data;
          console.info(` ${testName} subscribe end`);
          commonEventManager.subscribe(subscriber, (err, eventData) => {
            console.info(`${testName} subscribe recv`);
            if (err) {
              console.info(`${testName} fail:${JSON.stringify(err)}`);
              expect().assertFail();
              commonEventManager.unsubscribe(subscriber);
              console.info(` ${testName} unsubscribe`);
              done();
              return;
            }
            console.info(`${testName}:${eventData},code:${eventData.code}`);
            if (eventData.code == 0) {
              return;
            }
            console.info(`${testName} eventData.data:${eventData.data}`);
            let recv = JSON.parse(`${eventData.data}`);
            console.info(`${testName} recv:${recv}, palceholder:${recv.placeholder}`);
            if ( recv.placeholder === '' && recv.abilityName === '') {
              expect(true).assertTrue();
              console.info(` ${testName} true`);
            } else {
              console.info(` ${testName} fail`);
              expect().assertFail();
            }
            commonEventManager.unsubscribe(subscriber);
            console.info(` ${testName} unsubscribe`);
            done();
        })
      });
      console.info(`${testName} begin attach`);
      inputMethodCtrl.attach(false, cfg);
      console.info(`${testName} end attach`);
    }
    try {
      setTimeout(timeOutCb1, WAIT_DEAL_OK * 2);
    } catch(error) {
      console.info(` ${testName} result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info(` ${testName} end`);
  });
});

