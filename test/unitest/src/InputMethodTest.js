/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

import app from '@system.app'
import inputMethod from '@ohos.inputMethod'
import inputMethodEngine from '@ohos.inputMethodEngine'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe("InputMethodTest", function () {
    beforeAll(function () {
        console.info('beforeAll called')
    })

    afterAll(function () {
        console.info('afterAll called')
    })

    beforeEach(function () {
        console.info('beforeEach called')
    })

    afterEach(function () {
        console.info('afterEach called')
    })
    let mKeyboardDelegate = null;
    let inputMethodEngineObject = inputMethodEngine.getInputMethodAbility();
    let textInputClient = null;
    let kbController = null;
    let KeyboardDelegate = null;

    /*
     * @tc.number  inputmethod_test_MAX_TYPE_NUM_001
     * @tc.name    Test MAX_TYPE_NUM.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_MAX_TYPE_NUM_001', 0, async function (done) {
      console.info("************* inputmethod_test_MAX_TYPE_NUM_001 Test start*************");
      let MAX_NUM = 128;
      let inputMethodSetting = inputMethod.MAX_TYPE_NUM;
      console.info("inputmethod_test_001 result:" + inputMethodSetting);
      expect(inputMethodSetting == MAX_NUM).assertTrue();
      console.info("************* inputmethod_test_MAX_TYPE_NUM_001 Test end*************");
      done();
    });
    
    /*
     * @tc.number  inputmethod_test_getInputMethodController_001
     * @tc.name    Test to get an InputMethodController instance.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getInputMethodController_001', 0, async function (done) {
      console.info("************* inputmethod_test_getInputMethodController_001 Test start*************");
      let controller = inputMethod.getInputMethodController();
      expect(controller != undefined).assertTrue();
      console.info("************* inputmethod_test_getInputMethodController_001 Test end*************");
      done();
    });

      /*
     * @tc.number  inputmethod_test_getController_001
     * @tc.name    Test to get an InputMethodController instance.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getController_001', 0, async function (done) {
      console.info("************* inputmethod_test_getController_001 Test start*************");
      let controller = inputMethod.getController();
      expect(controller != undefined).assertTrue();
      console.info("************* inputmethod_test_getController_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_getInputMethodSetting_001
     * @tc.name    Test to get an InputMethodSetting instance.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getInputMethodSetting_001', 0, async function (done) {
      console.info("************* inputmethod_test_getInputMethodSetting_001 Test start*************");
      let setting = inputMethod.getInputMethodSetting();
      expect(setting != undefined).assertTrue();
      console.info("************* inputmethod_test_getInputMethodSetting_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_getInputMethodSetting_001
     * @tc.name    Test to get an InputMethodSetting instance.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getSetting_001', 0, async function (done) {
      console.info("************* inputmethod_test_getSetting_001 Test start*************");
      let setting = inputMethod.getSetting();
      expect(setting != undefined).assertTrue();
      console.info("************* inputmethod_test_getSetting_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_on_001
     * @tc.name    Test to subscribe 'imeChange'.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_on_001', 0, async function (done) {
      let setting = inputMethod.getSetting();
      setting.on('imeChange', (err, property, subtype) => {
        if (err) {
          console.info("inputmethod_test_on_001 imeChange err:" + err);
          return;
        }
        console.info("inputmethod_test_on_001 imeChange property:" + JSON.stringify(property));
        console.info("inputmethod_test_on_001 imeChange subtype:" + JSON.stringify(subtype));
      });
      done();
    });

    /*
     * @tc.number  inputmethod_test_switchInputMethod_001
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchInputMethod_001', 0, async function (done) {
      console.info("************* inputmethod_test_switchInputMethod_001 Test start*************");
      let inputMethodProperty = {
        packageName:"com.example.kikakeyboard",
        methodId:"ServiceExtAbility"
      }
      inputMethod.switchInputMethod(inputMethodProperty).then(data => {
        console.info("inputmethod_test_switchInputMethod_001 data:" + data)
        expect(data == true).assertTrue();
      }).catch( err=> {
        console.info("inputmethod_test_switchInputMethod_001 err:" + JSON.stringify(err.message))
      })
      console.info("************* inputmethod_test_switchInputMethod_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_getCurrentInputMethod_001
     * @tc.name    Test get current input method.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getCurrentInputMethod_001', 0, async function (done) {
      console.info("************* inputmethod_test_getCurrentInputMethod_001 Test start*************");
      let property = inputMethod.getCurrentInputMethod();
      console.info("getCurrentInputMethod_test_001 result property:" + property.packageName);
      console.info("getCurrentInputMethod_test_001 result property:" + property.methodId);
      expect(property != undefined).assertTrue();
      console.info("************* inputmethod_test_getCurrentInputMethod_001 Test end*************");
      done();
    });

    /*
    * @tc.number  inputmethod_test_listInputMethod_001
    * @tc.name    Test list input methods.
    * @tc.desc    Function test
    * @tc.level   2
    */
    it('inputmethod_test_listInputMethod_001', 0, async function (done) {
      console.info("************* inputmethod_test_listInputMethod_001 Test start*************");
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      console.info("listInputMethod_001 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.listInputMethod((err, data) => {
        if (err) {
          console.error("listInputMethod callback result---err: " + JSON.stringify(err.message));
          expect().assertFail();
        }
          console.info("listInputMethod_001 listInputMethod result" + JSON.stringify(data));
          expect(err == undefined).assertTrue();
      });
      console.info("************* inputmethod_test_listInputMethod_001 Test end*************");
       done();
    });
    
    /*
     * @tc.number  inputmethod_test_listInputMethod_002
     * @tc.name    Test list input methods.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_listInputMethod_002', 0, async function (done) {
      console.info("************* inputmethod_test_listInputMethod_002 Test start*************");
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      console.info("inputmethod_test_listInputMethod_002 result:" + JSON.stringify(inputMethodSetting));
      await inputMethodSetting.listInputMethod().then((data)=>{
        console.info("inputmethod_test_listInputMethod_002 listInputMethod result" + JSON.stringify(data));
        expect(data.length > 0).assertTrue();
      }).catch((err) => {
        console.info('inputmethod_test_listInputMethod_002 listInputMethod err ' + JSON.stringify(err.message));
        expect(null).assertFail();
      });
      console.info("************* inputmethod_test_listInputMethod_002 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_listInputMethod_003
     * @tc.name    Test list input methods.
     * @tc.desc    Function test
     * @tc.level   2
     */
        it('inputmethod_test_listInputMethod_003', 0, async function (done) {
          console.info("************* inputmethod_test_listInputMethod_003 Test start*************");
          let inputMethodSetting = inputMethod.getInputMethodSetting();
          console.info("inputmethod_test_listInputMethod_003 result:" + JSON.stringify(inputMethodSetting));
          await inputMethodSetting.listInputMethod(true).then((data)=>{
            console.info("inputmethod_test_listInputMethod_003 listInputMethod result" + JSON.stringify(data));
            expect(data.length > 0).assertTrue();
          }).catch((err) => {
            console.info('inputmethod_test_listInputMethod_003 listInputMethod err ' + JSON.stringify(err.message));
            expect(null).assertFail();
          });
          done();
          console.info("************* inputmethod_test_listInputMethod_003 Test end*************");
        });
  
      /*
      * @tc.number  inputmethod_test_listInputMethod_004
      * @tc.name    Test list input methods.
      * @tc.desc    Function test
      * @tc.level   2
      */
      it('inputmethod_test_listInputMethod_004', 0, async function (done) {
        console.info("************* inputmethod_test_listInputMethod_004 Test start*************");
        let inputMethodSetting = inputMethod.getInputMethodSetting();
        console.info("inputmethoh_test_004 result:" + JSON.stringify(inputMethodSetting));
        inputMethodSetting.listInputMethod(true, (err, data) => {
          if (err) {
            console.error("listInputMethod callback result---err: " + JSON.stringify(err.message));
            expect().assertFail();
          }
            console.info("inputmethoh_test_004 listInputMethod result" + JSON.stringify(data));
            expect(err == undefined).assertTrue();
        });
        console.info("************* inputmethod_test_listInputMethod_004 Test end*************");
         done();
      });
    /*
     * @tc.number  inputmethod_test_switchCurrentInputMethodSubtype_001
     * @tc.name    Test Indicates the input method subtype which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchCurrentInputMethodSubtype_001', 0, async function (done) {
      console.info("************* inputmethod_test_switchCurrentInputMethodSubtype_001 Test start*************");
      let InputMethodSubtype = {
        name:"com.example.kikakeyboard",
        id:"ServiceExtAbility",
        locale:"en_US.ISO-8859-1",
        language:"en",
        extra:{},
      }
      inputMethod.switchCurrentInputMethodSubtype(InputMethodSubtype).then(data => {
        console.info("inputmethod_test_switchCurrentInputMethodSubtype_001 data:" + data)
        expect(data == true).assertTrue();
      }).catch( err=> {
        console.info("inputmethod_test_switchCurrentInputMethodSubtype_001 err:" + JSON.stringify(err.message))
      })
      console.info("************* inputmethod_test_switchCurrentInputMethodSubtype_001 Test end*************");
      done();
    });
    
    /*
     * @tc.number  inputmethod_test_switchCurrentInputMethodSubtype_002
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchCurrentInputMethodSubtype_002', 0, async function (done) {
      console.info("************* inputmethod_test_switchCurrentInputMethodSubtype Test start*************");
      let InputMethodSubtype = {
        name:"com.example.kikakeyboard",
        id:"ServiceExtAbility",
        locale:"en_US.ISO-8859-1",
        language:"en",
        extra:{},
      }
      inputMethod.switchInputMethod(InputMethodSubtype, (err, data)=>{
        if(err){
          console.info("inputmethod_test_switchCurrentInputMethodSubtype error:" + JSON.stringify(err.message));
          expect().assertFail();
        }
        console.info("inputmethod_test_switchCurrentInputMethodSubtype data:" + data)
        expect(data == true).assertTrue();
      });
      console.info("************* inputmethod_test_switchCurrentInputMethodSubtype Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_switchCurrentInputMethodAndSubtype_001
     * @tc.name    Test Indicates the input method subtype which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchCurrentInputMethodAndSubtype_001', 0, async function (done) {
      console.info("************* inputmethod_test_switchCurrentInputMethodAndSubtype_001 Test start*************");
      let InputMethodSubtype = {
        name:"com.example.kikakeyboard",
        id:"ServiceExtAbility",
        locale:"en_US.ISO-8859-1",
        language:"en",
        extra:{},
      }
      let inputMethodProperty = {
        packageName:"com.example.kikakeyboard",
        methodId:"ServiceExtAbility"
      }
      inputMethod.switchCurrentInputMethodAndSubtype(inputMethodProperty, InputMethodSubtype).then(data => {
        console.info("inputmethod_test_switchCurrentInputMethodAndSubtype_001 data:" + data)
        expect(data == true).assertTrue();
      }).catch( err=> {
        console.info("inputmethod_test_switchCurrentInputMethodAndSubtype_001 err:" + JSON.stringify(err.message))
      })
      console.info("************* inputmethod_test_switchCurrentInputMethodAndSubtype_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_switchCurrentInputMethodAndSubtype_002
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchCurrentInputMethodAndSubtype_002', 0, async function (done) {
      console.info("************* inputmethod_test_switchCurrentInputMethodAndSubtype Test start*************");
      let InputMethodSubtype = {
        name:"com.example.kikakeyboard",
        id:"ServiceExtAbility",
        locale:"en_US.ISO-8859-1",
        language:"en",
        extra:{},
      }
      let inputMethodProperty = {
        packageName:"com.example.kikakeyboard",
        methodId:"ServiceExtAbility"
      }
      inputMethod.switchCurrentInputMethodAndSubtype(inputMethodProperty, InputMethodSubtype, (err, data)=>{
        if(err){
          console.info("inputmethod_test_switchCurrentInputMethodAndSubtype error:" + JSON.stringify(err.message));
          expect().assertFail();
        }
        console.info("inputmethod_test_switchCurrentInputMethodAndSubtype data:" + data)
        expect(data == true).assertTrue();
      });
      console.info("************* inputmethod_test_switchCurrentInputMethodAndSubtype Test end*************");
      done();
    });

    /*
    * @tc.number  inputmethod_test_listInputMethodSubtype_001
    * @tc.name    Test list input method subtypes.
    * @tc.desc    Function test
    * @tc.level   2
    */
    it('inputmethod_test_listInputMethodSubtype_001', 0, async function (done) {
      console.info("************* inputmethod_test_listInputMethodSubtype_001 Test start*************");
      let inputMethodProperty = {
        name:"com.example.kikakeyboard",
        id:"ServiceExtAbility"
      }
      let inputMethodSetting = inputMethod.getSetting();
      console.info("listInputMethodSubtype_001 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.listInputMethodSubtype(inputMethodProperty, (err, data) => {
        if (err) {
          console.error("listInputMethodSubtype callback result---err: " + JSON.stringify(err.message));
          expect().assertFail();
          return;
        }
          console.info("listInputMethodSubtype_001 listInputMethodSubtype result" + JSON.stringify(data));
          expect().assertTrue();
      });
      console.info("************* inputmethod_test_listInputMethodSubtype_001 Test end*************");
       done();
    });

    /*
     * @tc.number  inputmethod_test_listInputMethodSubtype_002
     * @tc.name    Test list input method subtypes.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_listInputMethodSubtype_002', 0, async function (done) {
      console.info("************* inputmethod_test_listInputMethodSubtype_002 Test start*************");
      let inputMethodProperty = {
        name:"com.example.kikakeyboard",
        id:"ServiceExtAbility"
      }
      let inputMethodSetting = inputMethod.getSetting();
      console.info("inputmethod_test_listInputMethodSubtype_002 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.listInputMethodSubtype(inputMethodProperty).then((data)=>{
        console.info("inputmethod_test_listInputMethodSubtype_002 listInputMethodSubtype result" + JSON.stringify(data));
        expect(data.length > 0).assertTrue();
      }).catch((err) => {
        console.info('inputmethod_test_listInputMethodSubtype_002 listInputMethodSubtype err ' + JSON.stringify(err.message));
        expect(null).assertFail();
      });
      console.info("************* inputmethod_test_listInputMethodSubtype_002 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_getCurrentInputMethodSubtype_001
     * @tc.name    Test get current input method.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getCurrentInputMethodSubtype_001', 0, async function (done) {
      console.info("************* inputmethod_test_getCurrentInputMethodSubtype_001 Test start*************");
      let property = inputMethod.getCurrentInputMethodSubtype();
      expect(property != undefined).assertTrue();
      console.info("************* inputmethod_test_getCurrentInputMethodSubtype_001 Test end*************");
      done();
    });

    /*
    * @tc.number  inputmethod_test_listCurrentInputMethodSubtype_001
    * @tc.name    Test list current input method subtypes.
    * @tc.desc    Function test
    * @tc.level   2
    */
    it('inputmethod_test_listCurrentInputMethodSubtype_001', 0, async function (done) {
      console.info("************* inputmethod_test_listCurrentInputMethodSubtype_001 Test start*************");
      let inputMethodSetting = inputMethod.getSetting();
      console.info("listCurrentInputMethodSubtype_001 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.listCurrentInputMethodSubtype((err, data) => {
        if (err) {
          console.error("listCurrentInputMethodSubtype callback result---err: " + JSON.stringify(err.message));
          expect().assertFail();
        }
          console.info("listCurrentInputMethodSubtype_001 listCurrentInputMethodSubtype result" + JSON.stringify(data));
          expect(err == undefined).assertTrue();
      });
      console.info("************* inputmethod_test_listCurrentInputMethodSubtype_001 Test end*************");
       done();
    });
    
    /*
     * @tc.number  inputmethod_test_listCurrentInputMethodSubtype_002
     * @tc.name    Test list current input method subtypes.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_listCurrentInputMethodSubtype_002', 0, async function (done) {
      console.info("************* inputmethod_test_listCurrentInputMethodSubtype_002 Test start*************");
      let inputMethodSetting = inputMethod.getSetting();
      console.info("inputmethod_test_listCurrentInputMethodSubtype_002 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.listCurrentInputMethodSubtype().then((data)=>{
        console.info("inputmethod_test_listCurrentInputMethodSubtype_002 listInputMethod result" + JSON.stringify(data));
        expect(data.length > 0).assertTrue();
      }).catch((err) => {
        console.info('inputmethod_test_listCurrentInputMethodSubtype_002 listInputMethod err ' + JSON.stringify(err.message));
        expect(null).assertFail();
      });
      console.info("************* inputmethod_test_listCurrentInputMethodSubtype_002 Test end*************");
      done();
    });
    
       /*
     * @tc.number  inputmethod_test_getInputMethods_001
     * @tc.name    Test list input methods.
     * @tc.desc    Function test
     * @tc.level   2
     */
       it('inputmethod_test_getInputMethods_001', 0, async function (done) {
        console.info("************* inputmethod_test_getInputMethods_001 Test start*************");
        let inputMethodSetting = inputMethod.getSetting();
        console.info("inputmethod_test_getInputMethods_001 result:" + JSON.stringify(inputMethodSetting));
        inputMethodSetting.getInputMethods(true).then((data)=>{
          console.info("inputmethod_test_getInputMethods_001 getInputMethods result" + JSON.stringify(data));
          expect(data.length > 0).assertTrue();
        }).catch((err) => {
          console.info('inputmethod_test_getInputMethods_001 getInputMethods err ' + JSON.stringify(err.message));
          expect(null).assertFail();
        });
        done();
        console.info("************* inputmethod_test_getInputMethods_001 Test end*************");
      });

    /*
    * @tc.number  inputmethod_test_getInputMethods_002
    * @tc.name    Test list input methods.
    * @tc.desc    Function test
    * @tc.level   2
    */
    it('inputmethod_test_getInputMethods_002', 0, async function (done) {
      console.info("************* inputmethod_test_getInputMethods_002 Test start*************");
      let inputMethodSetting = inputMethod.getSetting();
      console.info("inputmethod_test_getInputMethods_002 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.getInputMethods(true, (err, data) => {
        if (err) {
          console.error("getInputMethods callback result---err: " + JSON.stringify(err.message));
          expect().assertFail();
        }
          console.info("inputmethod_test_getInputMethods_002 getInputMethods result" + JSON.stringify(data));
          expect(err == undefined).assertTrue();
      });
      console.info("************* inputmethod_test_getInputMethods_002 Test end*************");
       done();
    });

    /*
     * @tc.number  inputmethod_test_displayOptionalInputMethod_001
     * @tc.name    Test displays a dialog box for selecting an input method.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_displayOptionalInputMethod_001', 0, async function (done) {
      console.info("************* inputmethod_test_displayOptionalInputMethod_001 Test start*************");
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      console.info("inputmethod_test_displayOptionalInputMethod_001 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.displayOptionalInputMethod((err) => {
        console.info("inputmethod_test_displayOptionalInputMethod_001 err:" + JSON.stringify(err.message));
        expect(err == undefined).assertTrue();
      });
      console.info("************* inputmethod_test_displayOptionalInputMethod_001 Test end*************");
     done();
    });

    /*
     * @tc.number  inputmethod_test_displayOptionalInputMethod_002
     * @tc.name    Test displays a dialog box for selecting an input method.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_displayOptionalInputMethod_002', 0, async function (done) {
      console.info("************* inputmethod_test_displayOptionalInputMethod_002 Test start*************");
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      console.info("inputmethod_test_displayOptionalInputMethod_002 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.displayOptionalInputMethod().then(()=>{
        console.info("inputmethod_test_displayOptionalInputMethod_002 displayOptionalInputMethod result");
        expect(true).assertTrue();
      }).catch((err) => {
        console.info('inputmethod_test_displayOptionalInputMethod_002 listInputMethod err ' + JSON.stringify(err.message));
        expect(null).assertFail();
      });
      console.info("************* inputmethod_test_displayOptionalInputMethod_002 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_showOptionalInputMethods_001
     * @tc.name    Test displays a dialog box for selecting an input method.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_showOptionalInputMethods_001', 0, async function (done) {
      console.info("************* inputmethod_test_showOptionalInputMethods_001 Test start*************");
      let inputMethodSetting = inputMethod.getSetting();
      console.info("inputmethod_test_showOptionalInputMethods_001 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.showOptionalInputMethods((err) => {
        console.info("inputmethod_test_showOptionalInputMethods_001 err:" + JSON.stringify(err.message));
        expect(err == undefined).assertTrue();
      });
      console.info("************* inputmethod_test_showOptionalInputMethods_001 Test end*************");
     done();
    });

    /*
     * @tc.number  inputmethod_test_showOptionalInputMethods_002
     * @tc.name    Test displays a dialog box for selecting an input method.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_showOptionalInputMethods_002', 0, async function (done) {
      console.info("************* inputmethod_test_showOptionalInputMethods_002 Test start*************");
      let inputMethodSetting = inputMethod.getSetting();
      console.info("inputmethod_test_showOptionalInputMethods_002 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.showOptionalInputMethods().then(()=>{
        console.info("inputmethod_test_showOptionalInputMethods_002 showOptionalInputMethods result");
        expect(true).assertTrue();
      }).catch((err) => {
        console.info('inputmethod_test_showOptionalInputMethods_002 showOptionalInputMethods err ' + JSON.stringify(err.message));
        expect(null).assertFail();
      });
      console.info("************* inputmethod_test_showOptionalInputMethods_002 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_showSoftKeyboard_001
     * @tc.name    Test Indicates the input method which will show softboard with calback.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_showSoftKeyboard_001', 0, async function (done) {
      console.info("************* inputmethod_test_showSoftKeyboard_001 Test start*************");
      let inputMethodCtrl = inputMethod.getInputMethodController()
      inputMethodCtrl.showSoftKeyboard((err)=>{
        if (err == undefined) {
          console.info("showSoftKeyboard callbace success" );
        } else {
          console.info('showSoftKeyboard callbace failed : ' + JSON.stringify(err.message));
          expect().assertFail();
        }
      });
      console.info("************* inputmethod_test_showSoftKeyboard_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_showSoftKeyboard_002
     * @tc.name    Test Indicates the input method which will show softboard with Promise.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_showSoftKeyboard_002', 0, async function (done) {
      console.info("************* inputmethod_test_showSoftKeyboard_002 Test start*************");
      let inputMethodCtrl = inputMethod.getInputMethodController()
      inputMethodCtrl.showSoftKeyboard().then((data) =>{
        console.info("showSoftKeyboard promise success" );
      }).catch((err) => {
        console.info('showSoftKeyboard promise failed : ' + JSON.stringify(err.message));
        expect().assertFail();
      })
      console.info("************* inputmethod_test_showSoftKeyboard_002 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_hideSoftKeyboard_001
     * @tc.name    Test Indicates the input method which will hide softboard with calback.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_hideSoftKeyboard_001', 0, async function (done) {
      console.info("************* inputmethod_test_hideSoftKeyboard_001 Test start*************");
      let inputMethodCtrl = inputMethod.getInputMethodController()
      inputMethodCtrl.hideSoftKeyboard((data)=>{
        if(data == undefined){
          console.info("hideSoftKeyboard callbace success" );
        }else{
          console.info('hideSoftKeyboard callbace failed : ' + JSON.stringify(err.message))
          expect().assertFail();
        }
      });
      console.info("************* inputmethod_test_hideSoftKeyboard_001 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_hideSoftKeyboard_002
     * @tc.name    Test Indicates the input method which will hide softboard with Promise.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_hideSoftKeyboard_002', 0, async function (done) {
      console.info("************* inputmethod_test_hideSoftKeyboard_002 Test start*************");
      let inputMethodCtrl = inputMethod.getInputMethodController()
      inputMethodCtrl.hideSoftKeyboard().then((data) =>{
        console.info("hideSoftKeyboard promise success" );
      }).catch((err) => {
        console.info('hideSoftKeyboard promise failed : ' + JSON.stringify(err.message));
        expect().assertFail();
      })
      console.info("************* inputmethod_test_hideSoftKeyboard_002 Test end*************");
      done();
    });

    /*
     * @tc.number  inputmethod_test_stopInputSession_001
     * @tc.name    Test Indicates the input method which will hides the keyboard.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_stopInputSession_001', 0, function (done) {
      console.info("************* inputmethod_test_stopInputSession_001 Test start*************");
      let inputMethodCtrl = inputMethod.getController();
      console.info("inputmethod_test_stopInputSession_001 result:" + JSON.stringify(inputMethodCtrl));
      inputMethodCtrl.stopInputSession((err, res) => {
        if (err) {
          console.info("inputmethod_test_stopInputSession_001 stopInput result" + JSON.stringify(err.message));
          expect().assertFail();
          return;
        } 
        console.info("inputmethod_test_stopInputSession_001 callback success" );
        expect(res == true).assertTrue();
      });
      console.info("************* inputmethod_test_stopInputSession_001 Test end*************");
      done();
    });

    it('inputMethodEngine_test_Off_000', 0 , async function (done) {
      inputMethodEngineObject.off('inputStart', (kbController, textInputClient) => {
        console.info("inputMethodEngine beforeEach inputStart:" + JSON.stringify(kbController));
        console.info("inputMethodEngine beforeEach inputStart:" + JSON.stringify(textInputClient));
      });
      inputMethodEngineObject.off('setSubtype', (err) => {
        console.info("inputMethodEngine beforeEach setSubtype:" + err);
      });
      inputMethodEngineObject.off('keyboardShow', (err) => {
        console.info("inputMethodEngine beforeEach keyboardShow:" + err);
      });
      inputMethodEngineObject.off('keyboardHide', (err) => {
        console.info("inputMethodEngine beforeEach keyboardHide:" + err);
      });
      KeyboardDelegate = inputMethodEngine.createKeyboardDelegate();
      KeyboardDelegate.off('keyDown', (keyEvent) => {
        console.info("inputMethodEngine beforeEach keyDown:" + keyEvent.keyCode);
        expect(keyEvent.keyCode).assertEqual('1');
  
        console.info("inputMethodEngine beforeEach keyDown:" + keyEvent.keyAction);
        expect(keyEvent.keyAction).assertEqual('1');
  
      });
      KeyboardDelegate.off('keyUp', (keyEvent) => {
        console.info("inputMethodEngine beforeEach keyUp:" + keyEvent.keyCode);
        expect(keyEvent.keyCode).assertEqual('1');
        console.info("inputMethodEngine beforeEach keyDown:" + keyEvent.keyAction);
        expect(keyEvent.keyAction).assertEqual('0');
  
      });
      KeyboardDelegate.off('cursorContextChange', (x, y, height) => {
        console.info("inputMethodEngine beforeEach cursorContextChange x:" + x);
        console.info("inputMethodEngine beforeEach cursorContextChange y:" + y);
        console.info("inputMethodEngine beforeEach cursorContextChange height:" + height);
      });
      KeyboardDelegate.off('selectionChange', (oldBegin, oldEnd, newBegin, newEnd) => {
        console.info("inputMethodEngine beforeEach selectionChange oldBegin:" + oldBegin);
        console.info("inputMethodEngine beforeEach selectionChange oldEnd:" + oldEnd);
        console.info("inputMethodEngine beforeEach selectionChange newBegin:" + newBegin);
        console.info("inputMethodEngine beforeEach selectionChange newEnd:" + newEnd);
      });
      KeyboardDelegate.off('textChange', (text) => {
        console.info("inputMethodEngine beforeEach textChange:" + text);
      });
      done();
    });
    
    it('inputMethodEngine_test_000', 0, async function (done) {
      inputMethodEngineObject.on('inputStart', (controller, client) => {
        console.info("inputMethodEngine beforeEach inputStart:" + JSON.stringify(controller));
        console.info("inputMethodEngine beforeEach inputStart:" + JSON.stringify(client));
        textInputClient = client;
        kbController = controller;
      });
      inputMethodEngineObject.on('setSubtype', (err) => {
        console.info("inputMethodEngine beforeEach setSubtype:" + err);
      });
      inputMethodEngineObject.on('keyboardShow', (err) => {
        console.info("inputMethodEngine beforeEach keyboardShow:" + err);
      });
      inputMethodEngineObject.on('keyboardHide', (err) => {
        console.info("inputMethodEngine beforeEach keyboardHide:" + err);
      });
      mKeyboardDelegate = inputMethodEngine.createKeyboardDelegate();
      mKeyboardDelegate.on('keyDown', (keyEvent) => {
        console.info("inputMethodEngine beforeEach keyDown:" + keyEvent.keyCode);
        expect(keyEvent.keyCode).assertEqual('1');
  
        console.info("inputMethodEngine beforeEach keyDown:" + keyEvent.keyAction);
        expect(keyEvent.keyAction).assertEqual('1');
  
  
      });
      mKeyboardDelegate.on('keyUp', (keyEvent) => {
        console.info("inputMethodEngine beforeEach keyUp:" + keyEvent.keyCode);
        expect(keyEvent.keyCode).assertEqual('1');
        console.info("inputMethodEngine beforeEach keyDown:" + keyEvent.keyAction);
        expect(keyEvent.keyAction).assertEqual('0');
  
      });
      mKeyboardDelegate.on('cursorContextChange', (x, y, height) => {
        console.info("inputMethodEngine beforeEach cursorContextChange x:" + x);
        console.info("inputMethodEngine beforeEach cursorContextChange y:" + y);
        console.info("inputMethodEngine beforeEach cursorContextChange height:" + height);
      });
      mKeyboardDelegate.on('selectionChange', (oldBegin, oldEnd, newBegin, newEnd) => {
        console.info("inputMethodEngine beforeEach selectionChange oldBegin:" + oldBegin);
        console.info("inputMethodEngine beforeEach selectionChange oldEnd:" + oldEnd);
        console.info("inputMethodEngine beforeEach selectionChange newBegin:" + newBegin);
        console.info("inputMethodEngine beforeEach selectionChange newEnd:" + newEnd);
      });
      mKeyboardDelegate.on('textChange', (text) => {
        console.info("inputMethodEngine beforeEach textChange:" + text);
      });
      done();
    });
  
    it('inputMethodEngine_test_001', 0, async function (done) {
      let keyType = inputMethodEngine.ENTER_KEY_TYPE_UNSPECIFIED;
      console.info("inputMethodEngine_test_001 result:" + keyType);
      expect(keyType).assertEqual(0);
      done();
    });
  
    it('inputMethodEngine_test_002', 0, async function (done) {
      let keyType = inputMethodEngine.ENTER_KEY_TYPE_GO;
      console.info("inputMethodEngine_test_002 result:" + keyType);
      expect(keyType).assertEqual(2);
      done();
    });
  
    it('inputMethodEngine_test_003', 0, async function (done) {
      let keyType = inputMethodEngine.ENTER_KEY_TYPE_SEARCH;
      console.info("inputMethodEngine_test_003 result:" + keyType);
      expect(keyType).assertEqual(3);
      done();
    });
  
    it('inputMethodEngine_test_004', 0, async function (done) {
      let keyType = inputMethodEngine.ENTER_KEY_TYPE_SEND;
      console.info("inputMethodEngine_test_004 result:" + keyType);
      expect(keyType).assertEqual(4);
      done();
    });
  
    it('inputMethodEngine_test_005', 0, async function (done) {
      let keyType = inputMethodEngine.ENTER_KEY_TYPE_NEXT;
      console.info("inputMethodEngine_test_005 result:" + keyType);
      expect(keyType).assertEqual(5);
      done();
    });
  
    it('inputMethodEngine_test_006', 0, async function (done) {
      let keyType = inputMethodEngine.ENTER_KEY_TYPE_DONE;
      console.info("inputMethodEngine_test_006 result:" + keyType);
      expect(keyType).assertEqual(6);
      done();
    });
  
    it('inputMethodEngine_test_007', 0, async function (done) {
      let keyType = inputMethodEngine.ENTER_KEY_TYPE_PREVIOUS;
      console.info("inputMethodEngine_test_007 result:" + keyType);
      expect(keyType).assertEqual(7);
      done();
    });
  
    it('inputMethodEngine_test_008', 0, async function (done) {
      let keyType = inputMethodEngine.PATTERN_NULL;
      console.info("inputMethodEngine_test_008 result:" + keyType);
      expect(keyType).assertEqual(-1);
      done();
    });
  
    it('inputMethodEngine_test_009', 0, async function (done) {
      let keyType = inputMethodEngine.PATTERN_TEXT;
      console.info("inputMethodEngine_test_009 result:" + keyType);
      expect(keyType).assertEqual(0);
      done();
    });
  
    it('inputMethodEngine_test_010', 0, async function (done) {
      let keyType = inputMethodEngine.PATTERN_NUMBER;
      console.info("inputMethodEngine_test_010 result:" + keyType);
      expect(keyType).assertEqual(2);
      done();
    });
  
    it('inputMethodEngine_test_011', 0, async function (done) {
      let keyType = inputMethodEngine.PATTERN_PHONE;
      console.info("inputMethodEngine_test_011 result:" + keyType);
      expect(keyType).assertEqual(3);
      done();
    });
  
    it('inputMethodEngine_test_012', 0, async function (done) {
      let keyType = inputMethodEngine.PATTERN_DATETIME;
      console.info("inputMethodEngine_test_012 result:" + keyType);
      expect(keyType).assertEqual(4);
      done();
    });
  
    it('inputMethodEngine_test_013', 0, async function (done) {
      let keyType = inputMethodEngine.PATTERN_EMAIL;
      console.info("inputMethodEngine_test_013 result:" + keyType);
      expect(keyType).assertEqual(5);
      done();
    });
  
    it('inputMethodEngine_test_014', 0, async function (done) {
      let keyType = inputMethodEngine.PATTERN_URI;
      console.info("inputMethodEngine_test_014 result:" + keyType);
      expect(keyType).assertEqual(6);
      done();
    });
  
    it('inputMethodEngine_test_015', 0, async function (done) {
      let keyType = inputMethodEngine.PATTERN_PASSWORD;
      console.info("inputMethodEngine_test_015 result:" + keyType);
      expect(keyType).assertEqual(7);
      done();
    });
  
    it('inputMethodEngine_test_016', 0, async function (done) {
      let keyType = inputMethodEngine.FLAG_SELECTING;
      console.info("inputMethodEngine_test_016 result:" + keyType);
      expect(keyType).assertEqual(2);
      done();
    });
  
    it('inputMethodEngine_test_017', 0, async function (done) {
      let keyType = inputMethodEngine.FLAG_SINGLE_LINE;
      console.info("inputMethodEngine_test_017 result:" + keyType);
      expect(keyType).assertEqual(1);
      done();
    });
  
    it('inputMethodEngine_test_018', 0, async function (done) {
      let keyType = inputMethodEngine.DISPLAY_MODE_PART;
      console.info("inputMethodEngine_test_018 result:" + keyType);
      expect(keyType).assertEqual(0);
      done();
    });
  
    it('inputMethodEngine_test_019', 0, async function (done) {
      let keyType = inputMethodEngine.DISPLAY_MODE_FULL;
      console.info("inputMethodEngine_test_019 result:" + keyType);
      expect(keyType).assertEqual(1);
      done();
    });
  
    it('inputMethodEngine_test_020', 0, async function (done) {
      let keyType = inputMethodEngine.OPTION_ASCII;
      console.info("inputMethodEngine_test_020 result:" + keyType);
      expect(keyType).assertEqual(20);
      done();
    });
  
    it('inputMethodEngine_test_021', 0, async function (done) {
      let keyType = inputMethodEngine.OPTION_NONE;
      console.info("inputMethodEngine_test_021 result:" + keyType);
      expect(keyType).assertEqual(0);
      done();
    });
  
    it('inputMethodEngine_test_022', 0, async function (done) {
      let keyType = inputMethodEngine.OPTION_AUTO_CAP_CHARACTERS;
      console.info("inputMethodEngine_test_022 result:" + keyType);
      expect(keyType).assertEqual(2);
      done();
    });
  
    it('inputMethodEngine_test_023', 0, async function (done) {
      let keyType = inputMethodEngine.OPTION_AUTO_CAP_SENTENCES;
      console.info("inputMethodEngine_test_023 result:" + keyType);
      expect(keyType).assertEqual(8);
      done();
    });
  
    it('inputMethodEngine_test_024', 0, async function (done) {
      let keyType = inputMethodEngine.OPTION_AUTO_WORDS;
      console.info("inputMethodEngine_test_024 result:" + keyType);
      expect(keyType).assertEqual(4);
      done();
    });
  
    it('inputMethodEngine_test_025', 0, async function (done) {
      let keyType = inputMethodEngine.OPTION_MULTI_LINE;
      console.info("inputMethodEngine_test_025 result:" + keyType);
      expect(keyType).assertEqual(1);
      done();
    });
  
    it('inputMethodEngine_test_026', 0, async function (done) {
      let keyType = inputMethodEngine.OPTION_NO_FULLSCREEN;
      console.info("inputMethodEngine_test_026 result:" + keyType);
      expect(keyType).assertEqual(10);
      done();
    });
  
    it('inputMethodEngine_test_027', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        textInputClient.sendKeyFunction(0, (value) => {
          console.info("inputMethodEngine_test_027 textInputClient sendKeyFunction:" + value);
          expect(value).assertEqual(true);
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_028', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        let promise = textInputClient.sendKeyFunction(0);
        promise.then(res => {
          console.info("inputMethodEngine_test_028 listInputMethod promise result-----" + JSON.stringify(res));
          expect(res).assertEqual(true);
        }).catch(err => {
          console.info("inputMethodEngine_test_028 listInputMethod promise error----" + JSON.stringify(err));
          expect().assertFail();
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_029', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        textInputClient.deleteForward(1, (value) => {
          console.info("inputMethodEngine_test_029 deleteForward:" + value);
          expect(value).assertEqual(true);
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_030', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        let promise = textInputClient.deleteForward(1);
        promise.then(res => {
          console.info("inputMethodEngine_test_030 deleteForward promise result-----" + JSON.stringify(res));
          expect(res).assertEqual(true);
        }).catch(err => {
          console.info("inputMethodEngine_test_030 deleteForward promise error----" + JSON.stringify(err));
          expect().assertFail();
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_031', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        textInputClient.deleteBackward(1, (value) => {
          console.info("inputMethodEngine_test_031 deleteBackward:" + value);
          expect(value).assertEqual(true);
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_032', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        let promise = textInputClient.deleteBackward(1);
        promise.then(res => {
          console.info("inputMethodEngine_test_032 deleteBackward promise result-----" + JSON.stringify(res));
          expect(res).assertEqual(true);
        }).catch(err => {
          console.info("inputMethodEngine_test_032 deleteBackward promise error----" + JSON.stringify(err));
          expect().assertFail();
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_033', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        textInputClient.InsertText('test', (value) => {
          console.info("inputMethodEngine_test_033 InsertText:" + value);
          expect(value).assertEqual(true);
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_034', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        let promise = textInputClient.InsertText('test');
        promise.then(res => {
          console.info("inputMethodEngine_test_034 InsertText promise result-----" + JSON.stringify(res));
          expect(res).assertEqual(true);
        }).catch(err => {
          console.info("inputMethodEngine_test_034 InsertText promise error----" + JSON.stringify(err));
          expect().assertFail();
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_035', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        textInputClient.getForward(1, (value) => {
          console.info("inputMethodEngine_test_035 getForward:" + value);
          expect(value).assertEqual(true);
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_036', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        let promise = textInputClient.getForward(1);
        promise.then(res => {
          console.info("inputMethodEngine_test_036 getForward promise result-----" + JSON.stringify(res));
          expect(res).assertEqual(true);
        }).catch(err => {
          console.info("inputMethodEngine_test_036 getForward promise error----" + JSON.stringify(err));
          expect().assertFail();
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_037', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        textInputClient.getEditorAttribute(1, (editorAttribute) => {
          console.info("inputMethodEngine_test_037 getEditorAttribute:" + value);
          expect(editorAttribute.inputPattern).assertEqual(1);
          expect(editorAttribute.enterKeyType).assertEqual(1);
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_038', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        let promise = textInputClient.getEditorAttribute();
        promise.then(res => {
          console.info("inputMethodEngine_test_038 getEditorAttribute promise result-----" + JSON.stringify(res));
          expect(res.inputPattern).assertEqual(1);
          expect(res.enterKeyType).assertEqual(1);
        }).catch(err => {
          console.info("inputMethodEngine_test_038 getEditorAttribute promise error----" + JSON.stringify(err));
          expect().assertFail();
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_039', 0, async function (done) {
      if (kbController == null) {
        expect(kbController == null).assertEqual(true);
      } else {
        kbController.hideKeyboard((err) => {
          if (err == undefined) {
            console.info("inputMethodEngine_test_039 hideKeyboard success.");
          } else {
            console.info('inputMethodEngine_test_039 hideKeyboard callbace failed : ' + JSON.stringify(err.message));
            expect().assertFail();
          }
        });
      }
      done();
    });

    it('inputMethodEngine_test_040', 0, async function (done) {
      if (kbController == null) {
        expect(kbController == null).assertEqual(true);
      } else {
        let promise = kbController.hideKeyboard();
        promise.then(res => {
          console.info("inputMethodEngine_test_040 hideKeyboard promise success.");
        }).catch(err => {
          console.info("inputMethodEngine_test_040 hideKeyboard promise error----" + JSON.stringify(err));
          expect().assertFail();
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_041', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        textInputClient.getBackward(1, (value) => {
          console.info("inputMethodEngine_test_041 getBackward:" + value);
          expect(value).assertEqual(true);
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_042', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        let promise = textInputClient.getBackward(1);
        promise.then(res => {
          console.info("inputMethodEngine_test_042 getBackward promise result-----" + JSON.stringify(res));
          expect(res).assertEqual(true);
        }).catch(err => {
          console.info("inputMethodEngine_test_042 getBackward promise error----" + JSON.stringify(err));
          expect().assertFail();
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_043', 0, async function (done) {
      let keyType = inputMethodEngine.WINDOW_TYPE_INPUT_METHOD_FLOAT;
      console.error("inputMethodEngine_test_043 result:" + keyType);
      expect(keyType == 2105).assertTrue();
      done();
    });
  
    it('inputMethodEngine_test_044', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        textInputClient.moveCursor(inputMethodEngine.CURSOR_UP, (value) => {
          console.info("inputMethodEngine_test_044 moveCursor:" + value);
          expect(value == null).assertEqual(true);
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_045', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        textInputClient.moveCursor(inputMethodEngine.CURSOR_DOWN, (value) => {
          console.info("inputMethodEngine_test_045 moveCursor:" + value);
          expect(value == null).assertEqual(true);
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_046', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        textInputClient.moveCursor(inputMethodEngine.CURSOR_LEFT).then(res => {
          console.info("inputMethodEngine_test_046 moveCursor promise result-----" + JSON.stringify(res));
          expect(res == null).assertEqual(true);
        }).catch(err => {
          console.info("inputMethodEngine_test_046 moveCursor promise error----" + JSON.stringify(err));
          expect().assertFail();
        });
      }
      done();
    });
  
    it('inputMethodEngine_test_047', 0, async function (done) {
      if (textInputClient == null) {
        expect(textInputClient == null).assertEqual(true);
      } else {
        textInputClient.moveCursor(inputMethodEngine.CURSOR_RIGHT).then(res => {
          console.info("inputMethodEngine_test_047 moveCursor promise result-----" + JSON.stringify(res));
          expect(res == null).assertEqual(true);
        }).catch(err => {
          console.info("inputMethodEngine_test_047 moveCursor promise error----" + JSON.stringify(err));
          expect().assertFail();
        });
      }
      done();
    });

    it('inputMethodEngine_test_048', 0, async function (done) {
      if (kbController == null) {
        expect(kbController == null).assertEqual(true);
      } else {
        kbController.hide((err) => {
          if (err == undefined) {
            console.info("inputMethodEngine_test_048 hide success.");
          } else {
            console.info('inputMethodEngine_test_048 hide callbace failed : ' + JSON.stringify(err.message));
            expect().assertFail();
          }
        });
      }
      done();
    });

    it('inputMethodEngine_test_049', 0, async function (done) {
      if (kbController == null) {
        expect(kbController == null).assertEqual(true);
      } else {
        let promise = kbController.hide();
        promise.then(res => {
          console.info("inputMethodEngine_test_049 hide promise success.");
        }).catch(err => {
          console.info("inputMethodEngine_test_049 hide promise error----" + JSON.stringify(err));
          expect().assertFail();
        });
      }
      done();
    });
})
