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
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index';
import { PanelInfo, PanelFlag, PanelType } from '@ohos.inputMethod.Panel';

describe('InputMethodWithAttachTest', function () {
  const WAIT_DEAL_OK = 500;
  const TEST_RESULT_CODE = 0;
  const TEST_FUNCTION = {
    INSERT_TEXT_SYNC: 0,
    MOVE_CURSOR_SYNC: 1,
    GET_ATTRIBUTE_SYNC: 2,
    SELECT_BY_RANGE_SYNC: 3,
    SELECT_BY_MOVEMENT_SYNC: 4,
    GET_INDEX_AT_CURSOR_SYNC: 5,
    DELETE_FORWARD_SYNC: 6,
    DELETE_BACKWARD_SYNC: 7,
    GET_FORWARD_SYNC: 8,
    GET_BACKWARD_SYNC: 9,
    CHANGE_FLAG_TO_FIXED: 10,
    CHANGE_FLAG_TO_FLOATING: 11,
    SETPRIVACYMODE_WITHOUT_PERMISSION: 12,
    SETPRIVACYMODE_ERROR_PARAM: 13,
    ADJUST_WITH_INVALID_FLAG: 14,
    ADJUST_WITH_NON_FULL_SCREEN_NO_PANEL_RECT: 15,
    ADJUST_WITH_FULL_SCREEN_NO_AVOID_Y: 16,
    ADJUST_WITH_INVALID_AVOID_Y:17,
    ADJUST_WITH_INVALID_TYPE:18,
    ADJUST_SUCCESS: 19,
    SET_PREVIEW_TEXT: 20,
    FINISH_TEXT_PREVIEW: 21
  }

  beforeAll(async function (done) {
    console.info('beforeAll called');
    let inputMethodProperty = {
      name: 'com.example.testIme',
      id: 'InputMethodExtAbility'
    };
    await inputMethod.switchInputMethod(inputMethodProperty);
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.on('finishTextPreview', () => {});
    inputMethodCtrl.on('setPreviewText', () => {});
    setTimeout(() => {
      done();
    }, WAIT_DEAL_OK);
  });

  afterAll(async function () {
    console.info('afterAll called');
    let inputMethodSetting = inputMethod.getInputMethodSetting();
    let props = await inputMethodSetting.listInputMethod();
    let bundleName = 'com.example.newTestIme';
    let bundleName1 = 'com.example.testIme';
    for (let i = 0; i < props.length; i++) {
      let prop = props[i];
      if (prop.name !== bundleName && prop.name !== bundleName1) {
        await inputMethod.switchInputMethod(prop);
      }
    }
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.off('finishTextPreview');
    inputMethodCtrl.off('setPreviewText');
  });

  beforeEach(async function () {
    console.info('beforeEach called');
    let inputMethodCtrl = inputMethod.getController();
    let cfg = {
      inputAttribute:
        {
          textInputType: inputMethod.TextInputType.TEXT,
          enterKeyType: inputMethod.EnterKeyType.NONE
        }
    };
    await inputMethodCtrl.attach(false, cfg);
  });

  afterEach(async function () {
    console.info('afterEach called');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.detach();
  });

  function publishCommonEvent(codeNumber) {
    console.info(`[publishCommonEvent] publish event, codeNumber = ${codeNumber}`);
    commonEventManager.publish('syncTestFunction', { code: codeNumber }, (err)=>{
      console.info(`inputMethod publish finish, err = ${JSON.stringify(err)}`);
    })
  }

  function subscribe(subscribeInfo, functionCode, done) {
    commonEventManager.createSubscriber(subscribeInfo).then((data) => {
      let subscriber = data;
      commonEventManager.subscribe(subscriber, (err, eventData) => {
        console.info("inputMethod subscribe");
        if (eventData.code === TEST_RESULT_CODE) {
          expect(true).assertTrue();
        } else {
          expect().assertFail();
        }
        commonEventManager.unsubscribe(subscriber);
        done();
      })
      publishCommonEvent(functionCode);
    })
  }
  /*
   * @tc.number  inputmethod_with_attach_test_showTextInput_001
   * @tc.name    Test whether the keyboard is displayed successfully.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_with_attach_test_showTextInput_001', 0, async function (done) {
    console.info('************* inputmethod_with_attach_test_showTextInput_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.showTextInput((err) => {
      if (err) {
        console.info(`inputmethod_with_attach_test_showTextInput_001 result: ${JSON.stringify(err)}`);
        expect().assertFail();
        done();
      }
      console.info('inputmethod_with_attach_test_showTextInput_001 callback success');
      expect(true).assertTrue();
      done();
    });
  });

  /*
  * @tc.number  inputmethod_with_attach_test_showTextInput_002
  * @tc.name    Test whether the keyboard is displayed successfully.
  * @tc.desc    Function test
  * @tc.level   2
  */
  it('inputmethod_with_attach_test_showTextInput_002', 0, async function (done) {
    console.info('************* inputmethod_with_attach_test_showTextInput_002 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.showTextInput().then(() => {
      console.info('inputmethod_with_attach_test_showTextInput_002 promise success');
      expect(true).assertTrue();
      done();
    }).catch((error) => {
      console.info(`inputmethod_with_attach_test_showTextInput_002 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    });
  });

  /*
  * @tc.number  inputmethod_with_attach_test_hideTextInput_001
  * @tc.name    Test whether the keyboard is hide successfully.
  * @tc.desc    Function test
  * @tc.level   2
  */
  it('inputmethod_with_attach_test_hideTextInput_001', 0, async function (done) {
    console.info('************* inputmethod_with_attach_test_hideTextInput_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.hideTextInput((err) => {
      if (err) {
        console.info(`inputmethod_with_attach_test_hideTextInput_001 result: ${JSON.stringify(err)}`);
        expect().assertFail();
        done();
        return;
      }
      console.info('inputmethod_with_attach_test_hideTextInput_001 callback success');
      expect(true).assertTrue();
      done();
    });
  });

  /*
  * @tc.number  inputmethod_with_attach_test_hideTextInput_002
  * @tc.name    Test whether the keyboard is hide successfully.
  * @tc.desc    Function test
  * @tc.level   2
  */
  it('inputmethod_with_attach_test_hideTextInput_002', 0, async function (done) {
    console.info('************* inputmethod_with_attach_test_hideTextInput_002 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.hideTextInput().then(() => {
      console.info('inputmethod_with_attach_test_hideTextInput_002 promise success');
      expect(true).assertTrue();
      done();
    }).catch((error) => {
      console.info(`inputmethod_with_attach_test_hideTextInput_002 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_with_attach_test_setCallingWindow_001
   * @tc.name    Test the window ID of the application that the notification system is currently bound to
   *     the input method. After setting correctly, whether the window where the client is located can avoid
   *     the input method window.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_with_attach_test_setCallingWindow_001', 0, async function (done) {
    console.info('************* inputmethod_with_attach_test_setCallingWindow_001 Test start*************');
    try {
      let inputMethodCtrl = inputMethod.getController();
      let windId = 100;
      inputMethodCtrl.setCallingWindow(windId, (err) => {
      if (err) {
        console.info(`inputmethod_with_attach_test_setCallingWindow_001 result: ${JSON.stringify(err)}`);
        expect().assertFail();
        done();
      }
      console.info('inputmethod_with_attach_test_setCallingWindow_001 callback success');
      expect(true).assertTrue();
      done();
      })
    } catch (error) {
      console.info(`inputmethod_with_attach_test_setCallingWindow_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_with_attach_test_setCallingWindow_002
   * @tc.name    Test the window ID of the application that the notification system is currently bound to
   *     the input method. After setting correctly, whether the window where the client is located can avoid
   *     the input method window.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_with_attach_test_setCallingWindow_002', 0, async function (done) {
    console.info('************* inputmethod_with_attach_test_setCallingWindow_002 Test start*************');
    try {
      let inputMethodCtrl = inputMethod.getController();
      let windId = 100;
      inputMethodCtrl.setCallingWindow(windId).then(() => {
        console.info('inputmethod_with_attach_test_setCallingWindow_002 promise success');
        expect(true).assertTrue();
        done();
      }).catch((error) => {
        console.info(`inputmethod_with_attach_test_setCallingWindow_002 result: ${JSON.stringify(error)}`);
        expect().assertFail();
        done();
      })
    } catch (error) {
      console.info(`inputmethod_with_attach_test_setCallingWindow_002 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info('************* inputmethod_with_attach_test_setCallingWindow_002 Test end*************');
  });

  /*
   * @tc.number  inputmethod_with_attach_test_updateCursor_001
   * @tc.name    Test whether the notification input method is valid when the current application cursor has changed.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_with_attach_test_updateCursor_001', 0, async function (done) {
    console.info('************* inputmethod_with_attach_test_updateCursor_001 Test start*************');
    try {
      let inputMethodCtrl = inputMethod.getController();
      let cursorInfo = { left: 100, top: 110, width: 600, height: 800 };
      inputMethodCtrl.updateCursor(cursorInfo, (err) => {
        if (err) {
          console.info(`inputmethod_with_attach_test_updateCursor_001 result: ${JSON.stringify(err)}`);
          expect().assertFail();
          done();
          return;
        }
        console.info('inputmethod_with_attach_test_updateCursor_001 callback success');
        expect(true).assertTrue();
        done();
      })
    } catch (error) {
      console.info(`inputmethod_with_attach_test_updateCursor_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info('************* inputmethod_with_attach_test_updateCursor_001 Test end*************');
  });

  /*
   * @tc.number  inputmethod_with_attach_test_updateCursor_002
   * @tc.name    Test whether the notification input method is valid when the current application cursor has changed.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_with_attach_test_updateCursor_002', 0, async function (done) {
    console.info('************* inputmethod_with_attach_test_updateCursor_002 Test start*************');
    try {
      let inputMethodCtrl = inputMethod.getController();
      let cursorInfo = { left: 100, top: 110, width: 600, height: 800 };
      inputMethodCtrl.updateCursor(cursorInfo).then(() => {
        console.info('inputmethod_with_attach_test_updateCursor_002 promise success');
        expect(true).assertTrue();
        done();
      }).catch((error) => {
        console.info(`inputmethod_with_attach_test_updateCursor_002 result: ${JSON.stringify(error)}`);
        expect().assertFail();
        done();
      })
    } catch (error) {
      console.info(`inputmethod_with_attach_test_updateCursor_002 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info('************* inputmethod_with_attach_test_updateCursor_002 Test end*************');
  });

  /*
   * @tc.number  inputmethod_with_attach_test_changeSelection_001
   * @tc.name    Test whether the selection range of the current application text of the notification input
   *     method has changed.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_with_attach_test_changeSelection_001', 0, async function (done) {
    console.info('************* inputmethod_with_attach_test_changeSelection_001 Test start*************');
    try {
      let inputMethodCtrl = inputMethod.getController();
      let text = 'text';
      let start = 0;
      let end = 5;
      inputMethodCtrl.changeSelection(text, start, end, (err) => {
        if (err) {
          console.info(`inputmethod_with_attach_test_changeSelection_001 result: ${JSON.stringify(err)}`);
          expect().assertFail();
          done();
          return;
        }
        console.info('inputmethod_with_attach_test_changeSelection_001 callback success');
        expect(true).assertTrue();
        done();
      })
    } catch (error) {
      console.info(`inputmethod_with_attach_test_changeSelection_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info('************* inputmethod_with_attach_test_changeSelection_001 Test end*************');
  });

  /*
   * @tc.number  inputmethod_with_attach_test_changeSelection_002
   * @tc.name    Test whether the selection range of the current application text of the notification input
   *     method has changed.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_with_attach_test_changeSelection_002', 0, async function (done) {
    console.info('************* inputmethod_with_attach_test_changeSelection_002 Test start*************');
    try {
      let inputMethodCtrl = inputMethod.getController();
      let text = 'text';
      let start = 0;
      let end = 5;
      inputMethodCtrl.changeSelection(text, start, end).then(() => {
        console.info('inputmethod_with_attach_test_changeSelection_002 promise success');
        expect(true).assertTrue();
        done();
      }).catch((error) => {
        console.info(`inputmethod_with_attach_test_changeSelection_002 result: ${JSON.stringify(error)}`);
        expect().assertFail();
        done();
      })
    } catch (error) {
      console.info(`inputmethod_with_attach_test_changeSelection_002 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info('************* inputmethod_with_attach_test_changeSelection_002 Test end*************');
  });

  /*
   * @tc.number  inputmethod_with_attach_test_updateAttribute_001
   * @tc.name    Test whether the InputAttribute information can be updated successfully.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_with_attach_test_updateAttribute_001', 0, async function (done) {
    console.info('************* inputmethod_with_attach_test_updateAttribute_001 Test start*************');
    try {
      let inputMethodCtrl = inputMethod.getController();
      let attribute = { textInputType: inputMethod.TextInputType.TEXT, enterKeyType: inputMethod.EnterKeyType.NONE };
      inputMethodCtrl.updateAttribute(attribute, (err) => {
        if (err) {
          console.info(`inputmethod_with_attach_test_updateAttribute_001 result: ${JSON.stringify(err)}`);
          expect().assertFail();
          done();
          return;
        }
        console.info('inputmethod_with_attach_test_updateAttribute_001 callback success');
        expect(true).assertTrue();
        done();
      })
    } catch (error) {
      console.info(`inputmethod_with_attach_test_updateAttribute_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info('************* inputmethod_with_attach_test_updateAttribute_001 Test end*************');
  });

  /*
   * @tc.number  inputmethod_with_attach_test_updateAttribute_001
   * @tc.name    Test whether the InputAttribute information can be updated successfully.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_with_attach_test_updateAttribute_001', 0, async function (done) {
    console.info('************* inputmethod_with_attach_test_updateAttribute_001 Test start*************');
    try {
      let inputMethodCtrl = inputMethod.getController();
      let attribute = { textInputType: 1, enterKeyType: 1 };
      inputMethodCtrl.updateAttribute(attribute).then(() => {
        console.info('inputmethod_with_attach_test_updateAttribute_001 promise success');
        expect(true).assertTrue();
        done();
      }).catch((error) => {
        console.info(`inputmethod_with_attach_test_updateAttribute_001 result: ${JSON.stringify(error)}`);
        expect().assertFail();
        done();
      })
    } catch (error) {
      console.info(`inputmethod_with_attach_test_updateAttribute_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
    console.info('************* inputmethod_with_attach_test_updateAttribute_001 Test end*************');
  });

  /*
   * @tc.number  inputmethod_test_showSoftKeyboard_001
   * @tc.name    Test Indicates the input method which will show softboard with callback.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_showSoftKeyboard_001', 0, async function (done) {
    console.info('************* inputmethod_test_showSoftKeyboard_001 Test start*************');
    let inputMethodCtrl = inputMethod.getInputMethodController();
    inputMethodCtrl.showSoftKeyboard((err)=>{
      if (err) {
        console.info(`inputmethod_test_showSoftKeyboard_001 err, ${JSON.stringify(err.message)}`);
        expect().assertFail();
        done();
      }
      console.info('************* inputmethod_test_showSoftKeyboard_001 Test end*************');
      expect(true).assertTrue();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_showSoftKeyboard_002
   * @tc.name    Test Indicates the input method which will show softboard with callback.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_showSoftKeyboard_002', 0, async function (done) {
    console.info('************* inputmethod_test_showSoftKeyboard_002 Test start*************');
    let inputMethodCtrl = inputMethod.getInputMethodController();
    inputMethodCtrl.showSoftKeyboard().then(()=>{
      console.info('inputmethod_test_showSoftKeyboard_002 success.');
      expect(true).assertTrue();
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_showSoftKeyboard_002 err, ${JSON.stringify(err.message)}`);
      expect().assertFail();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_hideSoftKeyboard_001
   * @tc.name    Test Indicates the input method which will hide softboard with callback.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_hideSoftKeyboard_001', 0, async function (done) {
    console.info('************* inputmethod_test_hideSoftKeyboard_001 Test start*************');
    let inputMethodCtrl = inputMethod.getInputMethodController();
    inputMethodCtrl.hideSoftKeyboard((err)=>{
      if(err){
        console.info(`inputmethod_test_hideSoftKeyboard_001 err, ${JSON.stringify(err.message)}`);
        expect().assertFail();
        done();
      }
      console.info('************* inputmethod_test_hideSoftKeyboard_001  Test end*************');
      expect(true).assertTrue();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_hideSoftKeyboard_001
   * @tc.name    Test Indicates the input method which will hide softboard with callback.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_hideSoftKeyboard_002', 0, async function (done) {
    console.info('************* inputmethod_test_hideSoftKeyboard_002 Test start*************');
    let inputMethodCtrl = inputMethod.getInputMethodController();
    inputMethodCtrl.hideSoftKeyboard().then(()=>{
      console.info('inputmethod_test_hideSoftKeyboard_002 success.');
      expect(true).assertTrue();
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_hideSoftKeyboard_002 err, ${JSON.stringify(err.message)}`);
      expect().assertFail();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_stopInputSessionWithAttach_001
   * @tc.name    Test Indicates the input method which will hides the keyboard.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_stopInputSessionWithAttach_001', 0, function (done) {
    console.info('************* inputmethod_test_stopInputSessionWithAttach_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.stopInputSession((err, ret) => {
      if (err) {
        console.info(`inputmethod_test_stopInputSessionWithAttach_001 err, ${JSON.stringify(err.message)}`);
        expect().assertFail();
        done();
        return;
      }
      expect(ret).assertTrue();
      console.info('************* inputmethod_test_stopInputSessionWithAttach_001 Test end*************');
      done();
    });
  });

  /*
 * @tc.number  inputmethod_test_stopInputSessionWithAttach_002
 * @tc.name    Test Indicates the input method which will hides the keyboard.
 * @tc.desc    Function test
 * @tc.level   2
 */
  it('inputmethod_test_stopInputSessionWithAttach_002', 0, function (done) {
    console.info('************* inputmethod_test_stopInputSessionWithAttach_002 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.hideSoftKeyboard().then((result)=>{
      if (result) {
        console.info('inputmethod_test_stopInputSessionWithAttach_002 failed.');
        expect().assertFail();
        done();
      }
      console.info('inputmethod_test_stopInputSessionWithAttach_002 success.');
      expect(true).assertTrue();
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_stopInputSessionWithAttach_002 err, ${JSON.stringify(err.message)}`);
      expect().assertFail();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_with_attach_test_on_000
   * @tc.name    Test whether the register the callback of the input method is valid.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_with_attach_test_on_000', 0, async function (done) {
    let inputMethodCtrl = inputMethod.getController();
    try {
      inputMethodCtrl.on('insertText', (text) => {
        console.info(`inputMethod insertText success, text: ${JSON.stringify(text)}`);
      });
      inputMethodCtrl.on('deleteLeft', (length) => {
        console.info(`inputMethod deleteLeft success, length: ${JSON.stringify(length)}`);
      });
      inputMethodCtrl.on('deleteRight', (length) => {
        console.info(`inputMethod deleteRight success, length: ${JSON.stringify(length)}`);
      });
      inputMethodCtrl.on('sendKeyboardStatus', (keyBoardStatus) => {
        console.info(`inputMethod sendKeyboardStatus success, keyBoardStatus: ${JSON.stringify(keyBoardStatus)}`);
      });
      inputMethodCtrl.on('sendFunctionKey', (functionKey) => {
        console.info(`inputMethod sendFunctionKey success, 
          functionKey.enterKeyType: ${JSON.stringify(functionKey.enterKeyType)}`);
      });
      inputMethodCtrl.on('moveCursor', (direction) => {
        console.info(`inputMethod moveCursor success, direction: ${JSON.stringify(direction)}`);
      });
      inputMethodCtrl.on('handleExtendAction', (action) => {
        console.info(`inputMethod handleExtendAction success, action: ${JSON.stringify(action)}`);
      });
      expect(true).assertTrue();
      done();
    } catch(error) {
      console.info(`inputmethod_with_attach_test_on_000 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_insertTextSync_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_insertTextSync_001', 0, async function (done) {
    console.info('************* inputmethod_test_insertTextSync_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    try {
      inputMethodCtrl.on('insertText', (text) => {
        console.info(`inputMethod insertText success, text: ${JSON.stringify(text)}`);
        expect(true).assertTrue();
        done();
      });
      publishCommonEvent(TEST_FUNCTION.INSERT_TEXT_SYNC);
    } catch(error) {
      console.info(`inputmethod_test_insertTextSync result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_moveCursorSync_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_moveCursorSync_001', 0, async function (done) {
    console.info('************* inputmethod_test_moveCursorSync_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    try {
      inputMethodCtrl.on('moveCursor', (direction) => {
        console.info(`inputMethod moveCursor success, direction: ${direction}`);
        expect(true).assertTrue();
        done();
      });
      publishCommonEvent(TEST_FUNCTION.MOVE_CURSOR_SYNC);
    } catch(error) {
      console.info(`inputmethod_text_moveCursorSync result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_getEditorAttributeSync_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_getEditorAttributeSync_001', 0, async function (done) {
    console.info('************* inputmethod_test_getEditorAttributeSync_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    try {
      let subscribeInfo = {
        events: ['getEditorAttributeSyncResult']
      };
      subscribe(subscribeInfo, TEST_FUNCTION.GET_ATTRIBUTE_SYNC, done);
    } catch(error) {
      console.info(`inputmethod_test_getEditorAttributeSync_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_SelectByRangeSync_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_selectByRangeSync_001', 0, async function (done) {
    console.info('************* inputmethod_test_selectByRangeSync_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    try {
      inputMethodCtrl.on('selectByRange', (range) => {
        console.info(`inputMethod selectByRangeSync success, direction: ${range}`);
        expect(true).assertTrue();
        done();
      });
      publishCommonEvent(TEST_FUNCTION.SELECT_BY_RANGE_SYNC);
    } catch(error) {
      console.info(`inputmethod_text_selectByRangeSync result: ${JSON.stringify(error)}`);
      expect().assertFail();
       done();
     }
  });

  /*
   * @tc.number  inputmethod_test_selectByMovementSync_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_selectByMovementSync_001', 0, async function (done) {
    console.info('************* inputmethod_test_selectByMovementSync_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    try {
      inputMethodCtrl.on('selectByMovement', (movement) => {
        console.info(`inputMethod selectByMovementSync success, movement: ${movement}`);
        expect(true).assertTrue();
        done();
      });
      publishCommonEvent(TEST_FUNCTION.SELECT_BY_MOVEMENT_SYNC);
    } catch(error) {
      console.info(`inputmethod_text_selectByMovementSync result: ${JSON.stringify(error)}`);
      expect().assertFail();
       done();
     }
  });

  /*
   * @tc.number  inputmethod_test_selectByMovementSync_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_getTextIndexAtCursorSync_001', 0, async function (done) {
    console.info('************* inputmethod_test_getTextIndexAtCursorSync_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    try {
      inputMethodCtrl.on('getTextIndexAtCursor', () => {
        console.info(`inputMethod getTextIndexAtCursor success`);
        return 2;
      });
      let subscribeInfo = {
        events: ['getTextIndexAtCursorSyncResult']
      };
      subscribe(subscribeInfo, TEST_FUNCTION.GET_INDEX_AT_CURSOR_SYNC, done);
    } catch(error) {
      console.info(`inputmethod_test_getTextIndexAtCursorSync_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_deleteForwardSync_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_deleteForwardSync_001', 0, async function (done) {
    console.info('************* inputmethod_test_deleteForwardSync_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    try {
      inputMethodCtrl.on('deleteLeft', (length) => {
        console.info(`inputMethod deleteForwardSync success, length: ${length}`);
        expect(true).assertTrue();
        done();
      });
      publishCommonEvent(TEST_FUNCTION.DELETE_FORWARD_SYNC);
    } catch(error) {
      console.info(`inputmethod_text_deleteForwardSync result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_deleteBackwardSync_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_deleteBackwardSync_001', 0, async function (done) {
    console.info('************* inputmethod_test_deleteBackwardSync_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    try {
      inputMethodCtrl.on('deleteRight', (length) => {
        console.info(`inputMethod deleteBackwardSync success, length: ${length}`);
        expect(true).assertTrue();
        done();
      });
      publishCommonEvent(TEST_FUNCTION.DELETE_BACKWARD_SYNC);
    } catch(error) {
      console.info(`inputmethod_text_deleteBackwardSync result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_getForwardSync_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_getForwardSync_001', 0, async function (done) {
    console.info('************* inputmethod_test_getForwardSync_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    try {
      inputMethodCtrl.on('getLeftTextOfCursor', (length) => {
        console.info(`inputMethod getForwardSync success, length: ${length}`);
        return 'getLeftTextOfCursor';
      });
      let subscribeInfo = {
        events: ['getForwardSyncResult']
      };
      subscribe(subscribeInfo, TEST_FUNCTION.GET_FORWARD_SYNC, done);
    } catch(error) {
      console.info(`inputmethod_text_getForwardSync result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });
  /*
   * @tc.number  inputmethod_test_getBackwardSync_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_getBackwardSync_001', 0, async function (done) {
    console.info('************* inputmethod_test_getBackwardSync_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    try {
      inputMethodCtrl.on('getRightTextOfCursor', (length) => {
        console.info(`inputMethod getBackwardSync success, length: ${length}`);
        return 'getRightTextOfCursor';
      });
      let subscribeInfo = {
        events: ['getBackwardSyncResult']
      };
      subscribe(subscribeInfo, TEST_FUNCTION.GET_BACKWARD_SYNC, done);
    } catch(error) {
      console.info(`inputmethod_text_getBackwardSync result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_isPanelShown_001
   * @tc.name    Test Indicates querying by isPanelShown.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_isPanelShown_001', 0, async function (done) {
    console.info('************* inputmethod_test_isPanelShown_001 Test start*************');
    try {
      let cfg = {
        inputAttribute:
            {
              textInputType: inputMethod.TextInputType.TEXT,
              enterKeyType: inputMethod.EnterKeyType.NONE
            }
      };
      await inputMethod.getController().attach(true, cfg);
      setTimeout(()=>{
        let result = inputMethod.getSetting().isPanelShown({type: PanelType.SOFT_KEYBOARD});
        if (result) {
          expect(true).assertTrue();
        } else {
          expect().assertFail();
        }
        done();
      }, WAIT_DEAL_OK);
    } catch (error) {
      console.info(`inputmethod_test_isPanelShown_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_isPanelShown_002
   * @tc.name    Test Indicates querying by isPanelShown.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_isPanelShown_002', 0, async function (done) {
    console.info('************* inputmethod_test_isPanelShown_002 Test start*************');
    try {
      let subscribeInfo = {
        events: ['changeFlag']
      };
      subscribe(subscribeInfo, TEST_FUNCTION.CHANGE_FLAG_TO_FLOATING, () => {
        let result = inputMethod.getSetting().isPanelShown({
          type: PanelType.SOFT_KEYBOARD,
          flag: PanelFlag.FLAG_FLOATING
        });
        if (result) {
          expect(true).assertTrue();
        } else {
          expect().assertFail();
        }
        subscribe(subscribeInfo, TEST_FUNCTION.CHANGE_FLAG_TO_FIXED, done);
      });
    } catch (error) {
      console.info(`inputmethod_test_isPanelShown_002 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_setPrivacyModeWithoutPermission_001
   * @tc.name    Test Indicates set panel privacy mode without permission.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_setPrivacyModeWithoutPermission_001', 0, async function (done) {
    console.info('************* inputmethod_test_setPrivacyModeWithoutPermission_001 Test start*************');
    try {
      let subscribeInfo = {
        events: ['setPrivacyModeWithoutPermissionResult']
      };
      subscribe(subscribeInfo, TEST_FUNCTION.SETPRIVACYMODE_WITHOUT_PERMISSION, done);
    } catch(error) {
      console.info(`inputmethod_test_setPrivacyModeWithoutPermission_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_setPrivacyModeErrorParam_001
   * @tc.name    Test Indicates set panel privacy mode with undefined param.
   * @tc.desc    Function test
   * @tc.level   2
   */
    it('inputmethod_test_setPrivacyModeErrorParam_001', 0, async function (done) {
      console.info('************* inputmethod_test_setPrivacyModeErrorParam_001 Test start*************');
      try {
        let subscribeInfo = {
          events: ['setPrivacyModeErrorParamResult']
        };
        subscribe(subscribeInfo, TEST_FUNCTION.SETPRIVACYMODE_ERROR_PARAM, done);
      } catch(error) {
        console.info(`inputmethod_test_setPrivacyModeErrorParam_001 result: ${JSON.stringify(error)}`);
        expect().assertFail();
        done();
      }
    });

  /*
   * @tc.number  inputmethod_test_adjustPanelRect_001
   * @tc.name    Test Indicates adjustPanelRect with invalid panel flag.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_adjustPanelRect_001', 0, async function (done) {
    console.info('************* inputmethod_test_adjustPanelRect_001 Test start*************');
    try {
      let subscribeInfo = {
        events: ['adjustWithInvalidFlagResult']
      };
      subscribe(subscribeInfo, TEST_FUNCTION.ADJUST_WITH_INVALID_FLAG, done);
    } catch(error) {
      console.info(`inputmethod_test_adjustPanelRect_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_adjustPanelRect_002
   * @tc.name    Test Indicates adjustPanelRect with non full screen mode but panel rect is not provided.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_adjustPanelRect_002', 0, async function (done) {
    console.info('************* inputmethod_test_adjustPanelRect_002 Test start*************');
    try {
      let subscribeInfo = {
        events: ['adjustWithNonFullScreenNoPanelRectResult']
      };
      subscribe(subscribeInfo, TEST_FUNCTION.ADJUST_WITH_NON_FULL_SCREEN_NO_PANEL_RECT, done);
    } catch(error) {
      console.info(`inputmethod_test_adjustPanelRect_002 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_adjustPanelRect_003
   * @tc.name    Test Indicates adjustPanelRect with full screen mode but no avoid Y.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_adjustPanelRect_003', 0, async function (done) {
    console.info('************* inputmethod_test_adjustPanelRect_003 Test start*************');
    try {
      let subscribeInfo = {
        events: ['adjustWithFullScreenNoAvoidYResult']
      };
      subscribe(subscribeInfo, TEST_FUNCTION.ADJUST_WITH_FULL_SCREEN_NO_AVOID_Y, done);
    } catch(error) {
      console.info(`inputmethod_test_adjustPanelRect_003 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_adjustPanelRect_004
   * @tc.name    Test Indicates adjustPanelRect with invalid avoid Y.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_adjustPanelRect_004', 0, async function (done) {
    console.info('************* inputmethod_test_adjustPanelRect_004 Test start*************');
    try {
      let subscribeInfo = {
        events: ['adjustWithInvalidAvoidYResult']
      };
      subscribe(subscribeInfo, TEST_FUNCTION.ADJUST_WITH_INVALID_AVOID_Y, done);
    } catch(error) {
      console.info(`inputmethod_test_adjustPanelRect_004 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_adjustPanelRect_005
   * @tc.name    Test Indicates adjustPanelRect with invalid panel type.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_adjustPanelRect_005', 0, async function (done) {
    console.info('************* inputmethod_test_adjustPanelRect_005 Test start*************');
    try {
      let subscribeInfo = {
        events: ['adjustWithInvalidTypeResult']
      };
      subscribe(subscribeInfo, TEST_FUNCTION.ADJUST_WITH_INVALID_TYPE, done);
    } catch(error) {
      console.info(`inputmethod_test_adjustPanelRect_005 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_adjustPanelRect_006
   * @tc.name    Test Indicates adjustPanelRect successfully.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_adjustPanelRect_006', 0, async function (done) {
    console.info('************* inputmethod_test_adjustPanelRect_006 Test start*************');
    try {
      let subscribeInfo = {
        events: ['adjustSuccessResult']
      };
      subscribe(subscribeInfo, TEST_FUNCTION.ADJUST_SUCCESS, done);
    } catch(error) {
      console.info(`inputmethod_test_adjustPanelRect_006 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_setPreviewText_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_setPreviewText_001', 0, async function (done) {
    console.info('************* inputmethod_test_setPreviewText_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    try {
      inputMethodCtrl.on('setPreviewText', (text, range) => {
        console.info(`inputMethod setPreviewText success, text: ${JSON.stringify(text)}, start: ${range.start}, end: ${range.end}`);
        expect(true).assertTrue();
        done();
      });
      publishCommonEvent(TEST_FUNCTION.SET_PREVIEW_TEXT);
    } catch(error) {
      console.info(`inputmethod_test_setPreviewText_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_setPreviewText_002
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_setPreviewText_002', 0, async function (done) {
    console.info('************* inputmethod_test_setPreviewText_002 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    try {
      inputMethodCtrl.on('setPreviewText', 'test');
    } catch(error) {
      console.info(`inputmethod_test_setPreviewText_002 result: ${JSON.stringify(error)}`);
      expect(error.code === 401).assertTrue();
      done();
    }
  });

   /*
   * @tc.number  inputmethod_test_setPreviewText_003
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
   it('inputmethod_test_setPreviewText_003', 0, async function (done) {
    console.info('************* inputmethod_test_setPreviewText_003 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    try {
      inputMethodCtrl.on('setPreviewText');
    } catch(error) {
      console.info(`inputmethod_test_setPreviewText_003 result: ${JSON.stringify(error)}`);
      expect(error.code === 401).assertTrue();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_setPreviewText_004
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_setPreviewText_004', 0, async function (done) {
    console.info('************* inputmethod_test_setPreviewText_004 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    let index = 0;
    let callback1 = (text, range) => {
      console.info(`inputMethod setPreviewText 1 success, text: ${JSON.stringify(text)}, start: ${range.start}, end: ${range.end}`);
      index += 1;
    };
    let callback2 = (text, range) => {
      console.info(`inputMethod setPreviewText 2 success, text: ${JSON.stringify(text)}, start: ${range.start}, end: ${range.end}`);
      index += 1;
    };
    try {
      inputMethodCtrl.on('setPreviewText', callback1);
      inputMethodCtrl.on('setPreviewText', callback2);
      publishCommonEvent(TEST_FUNCTION.SET_PREVIEW_TEXT);
      let timeOutCb = async () => {
        console.info(`inputMethod setPreviewText timeout`);
        clearTimeout(t);
        expect(index).assertEqual(2);
        done();
      };
      let t = setTimeout(timeOutCb, 500);
    } catch(error) {
      console.info(`inputmethod_test_setPreviewText_004 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_setPreviewText_005
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_setPreviewText_005', 0, async function (done) {
    console.info('************* inputmethod_test_setPreviewText_005 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    let index = 0;
    let callback1 = (text, range) => {
      console.info(`inputMethod setPreviewText 1 success, text: ${JSON.stringify(text)}, start: ${range.start}, end: ${range.end}`);
      index += 1;
    };
    let callback2 = (text, range) => {
      console.info(`inputMethod setPreviewText 2 success, text: ${JSON.stringify(text)}, start: ${range.start}, end: ${range.end}`);
      index += 1;
    };
    try {
      inputMethodCtrl.on('setPreviewText', callback1);
      inputMethodCtrl.on('setPreviewText', callback2);
      publishCommonEvent(TEST_FUNCTION.SET_PREVIEW_TEXT);
      let timeOutCb = async () => {
        console.info(`inputMethod setPreviewText timeOutCb`);
        clearTimeout(t);
        expect(index).assertEqual(2);
        inputMethodCtrl.off('setPreviewText', callback2);
        publishCommonEvent(TEST_FUNCTION.SET_PREVIEW_TEXT);
        let timeOutCb1 = async () => {
          console.info(`inputMethod setPreviewText timeOutCb1`);
          clearTimeout(t1);
          expect(index).assertEqual(3);
          inputMethodCtrl.off('setPreviewText');
          publishCommonEvent(TEST_FUNCTION.SET_PREVIEW_TEXT);
          let timeOutCb2 = async () => {
            console.info(`inputMethod setPreviewText timeOutCb2`);
            clearTimeout(t2);
            expect(index).assertEqual(3);
            inputMethodCtrl.on('setPreviewText', () => {});
            done();
          };
          let t2 = setTimeout(timeOutCb2, 500);
        };
        let t1 = setTimeout(timeOutCb1, 500);
      };
      let t = setTimeout(timeOutCb, 500);
    } catch(error) {
      console.info(`inputmethod_test_setPreviewText_005 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_finishTextPreview_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_finishTextPreview_001', 0, async function (done) {
    console.info('************* inputmethod_test_finishTextPreview_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    try {
      inputMethodCtrl.on('finishTextPreview', () => {
        console.info(`inputMethod finishTextPreview success`);
        expect(true).assertTrue();
        done();
      });
      publishCommonEvent(TEST_FUNCTION.FINISH_TEXT_PREVIEW);
    } catch(error) {
      console.info(`inputmethod_test_finishTextPreview_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_finishTextPreview_002
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_finishTextPreview_002', 0, async function (done) {
    console.info('************* inputmethod_test_finishTextPreview_002 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    try {
      inputMethodCtrl.on('finishTextPreview');
    } catch(error) {
      console.info(`inputmethod_test_finishTextPreview_002 result: ${JSON.stringify(error)}`);
      expect(error.code === 401).assertTrue();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_finishTextPreview_003
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_finishTextPreview_003', 0, async function (done) {
    console.info('************* inputmethod_test_finishTextPreview_002 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    try {
      inputMethodCtrl.on('finishTextPreview', 0);
    } catch(error) {
      console.info(`inputmethod_test_finishTextPreview_003 result: ${JSON.stringify(error)}`);
      expect(error.code === 401).assertTrue();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_finishTextPreview_004
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_finishTextPreview_004', 0, async function (done) {
    console.info('************* inputmethod_test_finishTextPreview_004 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    let index = 0;
    let callback1 = () => {
      console.info(`inputMethod finishTextPreview 1 success`);
      index += 1;
    };
    let callback2 = () => {
      console.info(`inputMethod finishTextPreview 2 success`);
      index += 1;
    };
    try {
      inputMethodCtrl.on('finishTextPreview', callback1);
      inputMethodCtrl.on('finishTextPreview', callback2);
      publishCommonEvent(TEST_FUNCTION.FINISH_TEXT_PREVIEW);
      let timeOutCb = async () => {
        console.info(`inputMethod finishTextPreview timeout`);
        clearTimeout(t);
        expect(index).assertEqual(2);
        done();
      };
      let t = setTimeout(timeOutCb, 500);
    } catch(error) {
      console.info(`inputmethod_test_finishTextPreview_004 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_finishTextPreview_005
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_finishTextPreview_005', 0, async function (done) {
    console.info('************* inputmethod_test_finishTextPreview_005 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    await inputMethodCtrl.showSoftKeyboard();
    let index = 0;
    let callback1 = () => {
      console.info(`inputMethod finishTextPreview 1 success`);
      index += 1;
    };
    let callback2 = () => {
      console.info(`inputMethod finishTextPreview 2 success`);
      index += 1;
    };
    try {
      inputMethodCtrl.on('finishTextPreview', callback1);
      inputMethodCtrl.on('finishTextPreview', callback2);
      publishCommonEvent(TEST_FUNCTION.FINISH_TEXT_PREVIEW);
      let timeOutCb = async () => {
        console.info(`inputMethod finishTextPreview timeOutCb`);
        clearTimeout(t);
        expect(index).assertEqual(2);

        inputMethodCtrl.off('finishTextPreview', callback2);
        publishCommonEvent(TEST_FUNCTION.FINISH_TEXT_PREVIEW);
        let timeOutCb1 = async () => {
          console.info(`inputMethod finishTextPreview timeOutCb1`);
          clearTimeout(t1);
          expect(index).assertEqual(3);
          inputMethodCtrl.off('finishTextPreview');
          publishCommonEvent(TEST_FUNCTION.FINISH_TEXT_PREVIEW);
          let timeOutCb2 = async () => {
            console.info(`inputMethod finishTextPreview timeOutCb2`);
            clearTimeout(t2);
            expect(index).assertEqual(3);
            done();
          };
          let t2 = setTimeout(timeOutCb2, 500);
        };
        let t1 = setTimeout(timeOutCb1, 500);
      };
      let t = setTimeout(timeOutCb, 500);
    } catch(error) {
      console.info(`inputmethod_test_finishTextPreview_005 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });
});