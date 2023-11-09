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
  const TEST_FUNCTION =  {
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
    CHANGE_FLAG_TO_FLOATING: 11
  }

  beforeAll(async function (done) {
    console.info('beforeAll called');
    let inputMethodProperty = {
      name:'com.example.testIme',
      id:'InputMethodExtAbility'
    };
    await inputMethod.switchInputMethod(inputMethodProperty);
    setTimeout(()=>{
      done();
    }, WAIT_DEAL_OK);
  });

  afterAll(async function () {
    console.info('afterAll called');
    let inputMethodSetting = inputMethod.getInputMethodSetting();
    let props = await inputMethodSetting.listInputMethod();
    let bundleName = 'com.example.newTestIme';
    let bundleName1 = 'com.example.testIme';
    for(let i = 0;i< props.length; i++) {
      let prop = props[i];
      if(prop.name !== bundleName && prop.name !== bundleName1){
        await inputMethod.switchInputMethod(prop);
      }
    }
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
    commonEventManager.createSubscriber(subscribeInfo).then((data)=>{
      let subscriber = data;
      commonEventManager.subscribe(subscriber, (err, eventData)=>{
        console.info("inputMethod subscribe");
        if(eventData.code === TEST_RESULT_CODE) {
          expect(true).assertTrue();
        }else{
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
   * @tc.number  inputmethod_test_stopInputSession_001
   * @tc.name    Test Indicates the input method which will hides the keyboard.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_stopInputSession_001', 0, function (done) {
    console.info('************* inputmethod_test_stopInputSession_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.stopInputSession((err, ret) => {
      if (err) {
        console.info(`inputmethod_test_stopInputSession_001 err, ${JSON.stringify(err.message)}`);
        expect().assertFail();
        done();
        return;
      }
      expect(ret).assertTrue();
      console.info('************* inputmethod_test_stopInputSession_001 Test end*************');
      done();
    });
  });

  /*
 * @tc.number  inputmethod_test_stopInputSession_002
 * @tc.name    Test Indicates the input method which will hides the keyboard.
 * @tc.desc    Function test
 * @tc.level   2
 */
  it('inputmethod_test_stopInputSession_002', 0, function (done) {
    console.info('************* inputmethod_test_stopInputSession_002 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.hideSoftKeyboard().then((result)=>{
      if (result) {
        console.info('inputmethod_test_stopInputSession_002 failed.');
        expect().assertFail();
        done();
      }
      console.info('inputmethod_test_stopInputSession_002 success.');
      expect(true).assertTrue();
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_stopInputSession_002 err, ${JSON.stringify(err.message)}`);
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
      let result = inputMethod.getSetting().isPanelShown({type: PanelType.SOFT_KEYBOARD});
      if (result) {
        expect(true).assertTrue();
      } else {
        expect().assertFail();
      }
      done();
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
});