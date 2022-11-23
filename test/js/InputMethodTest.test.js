
/* * Copyright (c) 2021 Huawei Device Co., Ltd.
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
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'hypium/index';
import inputMethod from '@ohos.inputmethod';

describe('appInfoTest_input_2', function () {

    /*
    * @tc.number  inputmethod_test_listInputMethod_001
    * @tc.name    Test Indicates the input method which will replace the current one.
    * @tc.desc    Function test
    * @tc.level   2
    */
    it('inputmethod_test_listInputMethod_001', 0, async function (done) {
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      console.info("inputmethod_test_001 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.listInputMethod((err, data) => {
          console.info("inputmethod_test_001 listInputMethod result" + JSON.stringify(data));
          expect(err === undefined).assertTrue();
      });
       done();
    });

    /*
     * @tc.number  inputmethod_test_listInputMethod_002
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_listInputMethod_002', 0, async function (done) {
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      console.info("inputmethod_test_002 result:" + JSON.stringify(inputMethodSetting));
      let promise = await inputMethodSetting.listInputMethod();
      console.info("inputmethod_test_002 listInputMethod result" + JSON.stringify(promise));
      if (promise.length > 0){
        let obj = promise[0]
        console.info("inputmethod_test_002 listInputMethod obj" + JSON.stringify(obj));
        expect(obj.packageName != null).assertTrue();
        expect(obj.methodId != null).assertTrue();
      }else{
        console.info("inputmethod_test_002 listInputMethod is null");
        expect().assertFail();
      }
      done();
    });

    /*
     * @tc.number  inputmethod_test_listInputMethod_003
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_listInputMethod_003', 0, async function (done) {
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      console.info("inputmethod_test_listInputMethod_003 result:" + JSON.stringify(inputMethodSetting));
      await inputMethodSetting.listInputMethod().then((data)=>{
          console.info("inputmethod_test_listInputMethod_003 listInputMethod result" + JSON.stringify(data));
          expect(data.length > 0).assertTrue();
      }).catch((err) => {
          console.info('inputmethod_test_listInputMethod_003 listInputMethod err ' + err);
          expect(null).assertFail();
      });
      done();
    });

    /*
     * @tc.number  inputmethod_test_displayOptionalInputMethod_001
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_displayOptionalInputMethod_001', 0, async function (done) {
        let inputMethodSetting = inputMethod.getInputMethodSetting();
        console.info("inputmethod_test_displayOptionalInputMethod_001 result:" + JSON.stringify(inputMethodSetting));
        inputMethodSetting.displayOptionalInputMethod((err) => {
            console.info("inputmethod_test_displayOptionalInputMethod_001 err:" + err);
            expect(err === undefined).assertTrue();
        });
       done();
    });

    /*
     * @tc.number  inputmethod_test_displayOptionalInputMethod_002
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_displayOptionalInputMethod_002', 0, async function (done) {
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      console.info("inputmethod_test_displayOptionalInputMethod_002 result:" + JSON.stringify(inputMethodSetting));
      let promise = await inputMethodSetting.displayOptionalInputMethod();
      console.info("inputmethod_test_displayOptionalInputMethod_002 result" + JSON.stringify(promise));
      expect(promise).assertEqual(undefined)
      done();
    });


    /*
     * @tc.number  inputmethod_test_displayOptionalInputMethod_003
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
        it('inputmethod_test_displayOptionalInputMethod_003', 0, async function (done) {
          let inputMethodSetting = inputMethod.getInputMethodSetting();
          console.info("inputmethod_test_displayOptionalInputMethod_003 result:" + JSON.stringify(inputMethodSetting));
          await inputMethodSetting.displayOptionalInputMethod().then(()=>{
              console.info("inputmethod_test_displayOptionalInputMethod_003 displayOptionalInputMethod result");
              expect(true).assertTrue();
          }).catch((err) => {
              console.info('inputmethod_test_displayOptionalInputMethod_003 listInputMethod err ' + err);
              expect(null).assertFail();
          });
          done();
      });


    /*
     * @tc.number  inputmethod_test_stopInput_001
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_stopInput_001', 0, function (done) {
        let inputMethodCtrl = inputMethod.getInputMethodController();
        console.info("inputmethod_test_stopInput_001 result:" + JSON.stringify(inputMethodCtrl));
        inputMethodCtrl.stopInput((err,res) => {
            console.info("inputmethod_test_stopInput_001 stopInput result" + res);
            console.info("inputmethod_test_stopInput_001 stopInput result" + err);
            expect(res === true).assertTrue();
            expect(err === undefined).assertTrue();
        });
       done();
    });

    /*
     * @tc.number  inputmethod_test_stopInput_002
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_stopInput_002', 0, async function (done) {
      let inputMethodCtrl = inputMethod.getInputMethodController();
      console.info("inputmethod_test_stopInput_002 result:" + JSON.stringify(inputMethodCtrl));
      let promise = await inputMethodCtrl.stopInput();
      console.info("inputmethod_test_stopInput_002 inputMethodCtrl stopInput result---" + JSON.stringify(promise));
      expect(promise).assertEqual(true)
      done();
    });

    /*
     * @tc.number  inputmethod_test_stopInput_003
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
        it('inputmethod_test_stopInput_003', 0, async function (done) {
          let inputMethodCtrl = inputMethod.getInputMethodController();
          console.info("inputmethod_test_stopInput_003 result:" + JSON.stringify(inputMethodCtrl));
          await inputMethodCtrl.stopInput().then((res)=>{
              console.info('inputmethod_test_stopInput_003 stopInput result' + res);
              expect(res === true).assertTrue();
          }).catch((err) => {
              console.info('inputmethod_test_stopInput_003 stopInput err ' + err);
              expect(null).assertFail();
          });
          console.info("inputmethod_test_stopInput_003 inputMethodCtrl stopInput result");
          done();
      });

    /*
     * @tc.number: inputmethod_test_MAX_TYPE_NUM_001
     * @tc.name: inputMethod::MAX_TYPE_NUM
     * @tc.desc: Verify Max_ TYPE_ NUM
     */
    it('inputmethod_test_MAX_TYPE_NUM_001', 0, async function (done) {
      let inputMethodSetting = inputMethod.MAX_TYPE_NUM;
      console.info("inputmethod_test_001 result:" + inputMethodSetting);
      expect(inputMethodSetting != null).assertTrue();
      done();
    });

    /*
     * @tc.number  inputmethod_test_switchInputMethod_001
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchInputMethod_001', 0, async function (done) {
      let inputMethodProperty = {
        packageName:"com.example.kikakeyboard",
        methodId:"ServiceExtAbility",
        name:"com.example.kikakeyboard",
        id:"ServiceExtAbility"
      }
      inputMethod.switchInputMethod(inputMethodProperty).then((data) => {
        console.info("inputmethod_test_switchInputMethod_001 data:" + data)
        expect(data).assertEqual(true);
      }).catch((err) => {
        console.error('inputmethod_test_switchInputMethod_001 failed because ' + JSON.stringify(err));
      });
      done();
    });

    /*
     * @tc.number  inputmethod_test_switchInputMethod_002
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchInputMethod_002', 0, async function (done) {
      let inputMethodProperty = {
        packageName:"com.example.kikakeyboard",
        methodId:"ServiceExtAbility",
        name:"com.example.kikakeyboard",
        id:"ServiceExtAbility"
      }
      inputMethod.switchInputMethod(inputMethodProperty, (err, data)=>{
        if(err){
          console.info("inputmethod_test_switchInputMethod_002 error:" + err);
          expect().assertFail()
        }
        console.info("inputmethod_test_switchInputMethod_002 data:" + data)
        expect(data === true).assertTrue();
      });
      done();
    });

    /*
     * @tc.number  inputmethod_test_getCurrentInputMethodSubtype_001
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_getCurrentInputMethodSubtype_001', 0, async function (done) {
      let inputMethodSubtype = inputMethod.getCurrentInputMethodSubtype();
      console.info("inputmethod_test_getCurrentInputMethodSubtype_001 result:" + JSON.stringify(inputMethodSubtype));
      expect(inputMethodSubtype).assertTrue();
      done();
    });

    /*
     * @tc.number  inputmethod_test_getCurrentInputMethodSubtype_001
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchCurrentInputMethodSubtype_001', 0, async function (done) {
      let inputMethodSubProperty = {
        id: "com.example.kikainput",
        label: "ServiceExtAbility"
      }
      await inputMethod.switchCurrentInputMethodSubtype(inputMethodSubProperty).then((data) => {
        console.info("inputmethod_test_switchCurrentInputMethodSubtype_001 data:" + data)
        expect(data).assertEqual(true);
      }).catch((err) => {
        console.error('inputmethod_test_switchCurrentInputMethodSubtype_001 failed because ' + JSON.stringify(err));
      });
      done();
    });

    /*
     * @tc.number  inputmethod_test_switchCurrentInputMethodSubtype_002
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchCurrentInputMethodSubtype_002', 0, async function (done) {
      let inputMethodSubProperty = {
        id: "com.example.kikainput",
        label: "ServiceExtAbility"
      }
      inputMethod.switchCurrentInputMethodSubtype(inputMethodSubProperty, (err, data)=>{
        if(err){
          console.info("inputmethod_test_switchCurrentInputMethodSubtype_002 error:" + err);
          expect().assertFail()
        }
        console.info("inputmethod_test_switchCurrentInputMethodSubtype_002 data:" + data)
        expect(data === true).assertTrue();
      });
    });

    /*
     * @tc.number  inputmethod_test_getCurrentInputMethodSubtype_001
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchCurrentInputMethodAndSubtype_001', 0, async function (done) {
      let inputMethodSubProperty = {
        id: "com.example.kikainput",
        label: "ServiceExtAbility"
      }
      await inputMethod.switchCurrentInputMethodAndSubtype(inputMethodProperty, inputMethodSubProperty).then((data) => {
        console.info("inputmethod_test_switchCurrentInputMethodAndSubtype_001 data:" + data)
        expect(data).assertEqual(true);
      }).catch((err) => {
        console.error('inputmethod_test_switchCurrentInputMethodAndSubtype_001 failed because ' + JSON.stringify(err));
      });
      done();
    });

    /*
     * @tc.number  inputmethod_test_switchCurrentInputMethodAndSubtype_002
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_switchCurrentInputMethodAndSubtype_002', 0, async function (done) {
      let inputMethodProperty = {
        packageName:"com.example.kikakeyboard",
        methodId:"ServiceExtAbility"
      }
      let inputMethodSubProperty = {
        id: "com.example.kikainput",
        label: "ServiceExtAbility"
      }
      inputMethod.switchCurrentInputMethodAndSubtype(inputMethodProperty, inputMethodSubProperty, (err, data)=>{
        if(err){
          console.info("inputmethod_test_switchCurrentInputMethodAndSubtype_002 error:" + err);
          expect().assertFail()
        }
        console.info("inputmethod_test_switchCurrentInputMethodAndSubtype_002 data:" + data)
        expect(data === true).assertTrue();
      });
    });

    /*
    * @tc.number  inputmethod_test_ListInputMethodSubtype_001
    * @tc.name    Test Indicates the input method which will replace the current one.
    * @tc.desc    Function test
    * @tc.level   2
    */
    it('inputmethod_test_ListInputMethodSubtype_001', 0, async function (done) {
      let inputMethodProperty = {
        packageName:"com.example.kikakeyboard",
        methodId:"ServiceExtAbility",
        name:"com.example.kikakeyboard",
        id:"ServiceExtAbility",
      }
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      console.info("inputmethod_test_ListInputMethodSubtype_001 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.listInputMethodSubtype(inputMethodProperty, (err, data) => {
          console.info("inputmethod_test_ListInputMethodSubtype_001 result" + JSON.stringify(data));
          expect(err === undefined).assertTrue();
      });
       done();
    });

    /*
     * @tc.number  inputmethod_test_ListInputMethodSubtype_002
     * @tc.name    Test Indicates the input method which will replace the current one.
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('inputmethod_test_ListInputMethodSubtype_002', 0, async function (done) {
      let inputMethodProperty = {
        packageName:"com.example.kikakeyboard",
        methodId:"ServiceExtAbility",
        name:"com.example.kikakeyboard",
        id:"ServiceExtAbility",
      }
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      console.info("inputmethod_test_ListInputMethodSubtype_002 result:" + JSON.stringify(inputMethodSetting));
      await inputMethodSetting.listInputMethodSubtype(inputMethodProperty).then((data)=>{
          console.info("inputmethod_test_ListInputMethodSubtype_002 result" + JSON.stringify(data));
          expect(data.length > 0).assertTrue();
      }).catch((err) => {
          console.info('inputmethod_test_ListInputMethodSubtype_002 listInputMethod err ' + err);
          expect(null).assertFail();
      });
      done();
    });

    /*
    * @tc.number  inputmethod_test_ListCurrentInputMethodSubtype_001
    * @tc.name    Test Indicates the input method which will replace the current one.
    * @tc.desc    Function test
    * @tc.level   2
    */
    it('inputmethod_test_ListCurrentInputMethodSubtype_001', 0, async function (done) {
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      console.info("inputmethod_test_ListCurrentInputMethodSubtype_001 result:" + JSON.stringify(inputMethodSetting));
      inputMethodSetting.listCurrentInputMethodSubtype((err, data) => {
          console.info("inputmethod_test_ListCurrentInputMethodSubtype_001 result" + JSON.stringify(data));
          expect(err === undefined).assertTrue();
      });
       done();
    });

    /*
    * @tc.number  inputmethod_test_ListCurrentInputMethodSubtype_002
    * @tc.name    Test Indicates the input method which will replace the current one.
    * @tc.desc    Function test
    * @tc.level   2
    */
    it('inputmethod_test_ListCurrentInputMethodSubtype_002', 0, async function (done) {
      let inputMethodSetting = inputMethod.getInputMethodSetting();
      console.info("inputmethod_test_ListCurrentInputMethodSubtype_002 result:" + JSON.stringify(inputMethodSetting));
      await inputMethodSetting.listCurrentInputMethodSubtype().then((data)=>{
          console.info("inputmethod_test_ListCurrentInputMethodSubtype_002 result" + JSON.stringify(data));
          expect(data.length > 0).assertTrue();
      }).catch((err) => {
          console.info('inputmethod_test_ListCurrentInputMethodSubtype_002 err ' + err);
          expect(null).assertFail();
      });
      done();
    });

})
