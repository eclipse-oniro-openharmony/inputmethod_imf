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
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index';

describe('InputMethodWithAttachTest', function () {
  beforeAll(function () {
      console.info('beforeAll called');
  });

  afterAll(function () {
      console.info('afterAll called');
  });

  beforeEach(function () {
    console.info('beforeEach called');
    let inputMethodCtrl = inputMethod.getController();
    let cfg = {
      inputAttribute:
        {
          textInputType: inputMethod.TextInputType.TEXT,
          enterKeyType: inputMethod.EnterKeyType.NONE
        }
    };
    inputMethodCtrl.attach(false, cfg, (err) => {
      if (err) {
        console.info(`beforeEach called attach failed: ${JSON.stringify(err)}`);
        return;
      }
      console.info('beforeEach called attach success');
    })
  });

  afterEach(function () {
    console.info('afterEach called');
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.detach((err) => {
      if (err) {
        console.info(`afterEach called detach failed: ${JSON.stringify(err)}`);
        return;
      }
      console.info('afterEach called detach success');
    })
  });

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
});
