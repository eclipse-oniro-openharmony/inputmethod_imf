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

import inputMethod from '@ohos.inputMethod'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('InputMethodTest', function () {
  beforeAll(function () {
    console.info('beforeAll called');
    inputMethod.getSetting().on('imeChange', imeChange);
    propertyBeforeSwitch = inputMethod.getCurrentInputMethod();
  });

  afterAll(function () {
    inputMethod.getSetting().off('imeChange');
    console.info('afterAll called');
  }); 

  beforeEach(function () {
    console.info('beforeEach called');
  });

  afterEach(function () {
    console.info('afterEach called');
  });
  let propertyBeforeSwitch = undefined;
  let bundleName = 'com.example.newTestIme';
  let extName = 'InputMethodExtAbility';
  let subName = ['lowerInput', 'upperInput', 'chineseInput'];
  let locale = ['en-US', 'en-US', 'zh-CN'];
  let language = ['english', 'english', 'chinese'];
  let bundleName1 = 'com.example.testIme';
  let extName1 = ['InputMethodExtAbility', 'InputMethodExtAbility2'];
  let locale1 = ['zh-CN', 'en-US'];
  let language1 = ['chinese', 'english'];
  const LEAST_ALL_IME_NUM = 2;
  const ENABLE_IME_NUM = 3;
  const LEAST_DISABLE_IME_NUM = 1;
  const NEW_IME_SUBTYPE_NUM = 3;
  const OLD_IME_SUBTYPE_NUM = 2;
  const WAIT_DEAL_OK = 500;
  const NO_DISABLED_IME = 0;
  const CURRENT_IME_COUNT = 1;

  let isImeChange = false;
  let imeChangeProp = undefined;
  let imeChangeSubProp = undefined;
  function imeChange(prop, subProp) {
    isImeChange = true;
    imeChangeProp = prop;
    imeChangeSubProp = subProp;
  }

  function checkNewImeCurrentProp(property)
  {
    expect(property.name).assertEqual(bundleName);
    expect(property.id).assertEqual(extName);
    expect(property.packageName).assertEqual(bundleName);
    expect(property.methodId).assertEqual(extName);
  }

  function checkNewImeCurrentSubProp(subProp, index)
  {
    expect(subProp.name).assertEqual(bundleName);
    expect(subProp.id).assertEqual(subName[index]);
    expect(subProp.locale).assertEqual(locale[index]);
    expect(subProp.language).assertEqual(language[index]);
  }

  function checkNewImeSubProps(subProps)
  {
    expect(subProps.length).assertEqual(NEW_IME_SUBTYPE_NUM);
    for (let i = 0; i < subProps.length; i++) {
      expect(subProps[i].name).assertEqual(bundleName);
      expect(subProps[i].id).assertEqual(subName[i]);
      expect(subProps[i].locale).assertEqual(locale[i]);
      expect(subProps[i].language).assertEqual(language[i]);
    }
  }

  function checkImeCurrentProp(property, index)
  {
    expect(property.name).assertEqual(bundleName1);
    expect(property.id).assertEqual(extName1[index]);
    expect(property.packageName).assertEqual(bundleName1);
    expect(property.methodId).assertEqual(extName1[index]);
  }

  function checkImeCurrentSubProp(subProp, index)
  {
    expect(subProp.name).assertEqual(bundleName1);
    expect(subProp.id).assertEqual(extName1[index]);
    expect(subProp.locale).assertEqual(locale1[index]);
    expect(subProp.language).assertEqual(language1[index]);
  }

  function checkImeSubProps(subProps)
  {
    expect(subProps.length).assertEqual(OLD_IME_SUBTYPE_NUM);
    for (let i = 0; i < subProps.length; i++) {
      expect(subProps[i].name).assertEqual(bundleName1);
      expect(subProps[i].id).assertEqual(extName1[i]);
      expect(subProps[i].locale).assertEqual(locale1[i]);
      expect(subProps[i].language).assertEqual(language1[i]);
    }
  }

  function wait(delay) {
    let start = new Date().getTime();
    while (new Date().getTime() - start < delay){
    }
  }

  /*
   * @tc.number  inputmethod_test_MAX_TYPE_NUM_001
   * @tc.name    Test MAX_TYPE_NUM.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_MAX_TYPE_NUM_001', 0, async function (done) {
    console.info('************* inputmethod_test_MAX_TYPE_NUM_001 Test start*************');
    let MAX_NUM = 128;
    let num = inputMethod.MAX_TYPE_NUM;
    console.info(`inputmethod_test_001 result: ${ num }`);
    expect(num).assertEqual(MAX_NUM);
    done();
  });

  /*
   * @tc.number  inputmethod_test_getInputMethodController_001
   * @tc.name    Test to get an InputMethodController instance.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_getInputMethodController_001', 0, async function (done) {
    console.info('************* inputmethod_test_getInputMethodController_001 Test start*************');
    let controller = inputMethod.getInputMethodController();
    expect(controller !== undefined).assertTrue();
    console.info('************* inputmethod_test_getInputMethodController_001 Test end*************');
    done();
  });

  /*
   * @tc.number  inputmethod_test_getController_001
   * @tc.name    Test to get an InputMethodController instance.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_getController_001', 0, async function (done) {
    console.info('************* inputmethod_test_getController_001 Test start*************');
    let controller = inputMethod.getController();
    expect(controller !== undefined).assertTrue();
    console.info('************* inputmethod_test_getController_001 Test end*************');
    done();
  });

  /*
   * @tc.number  inputmethod_test_getInputMethodSetting_001
   * @tc.name    Test to get an InputMethodSetting instance.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_getInputMethodSetting_001', 0, async function (done) {
    console.info('************* inputmethod_test_getInputMethodSetting_001 Test start*************');
    let setting = inputMethod.getInputMethodSetting();
    expect(setting !== undefined).assertTrue();
    console.info('************* inputmethod_test_getInputMethodSetting_001 Test end*************');
    done();
  });

  /*
   * @tc.number  inputmethod_test_getInputMethodSetting_001
   * @tc.name    Test to get an InputMethodSetting instance.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_getSetting_001', 0, async function (done) {
    console.info('************* inputmethod_test_getSetting_001 Test start*************');
    let setting = inputMethod.getSetting();
    expect(setting !== undefined).assertTrue();
    console.info('************* inputmethod_test_getSetting_001 Test end*************');
    done();
  });

  /*
   * @tc.number  inputmethod_test_switchInputMethod_001
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_switchInputMethod_001', 0, async function (done) {
    console.info('************* inputmethod_test_switchInputMethod_001 Test start*************');
    let inputMethodProperty = {
      name:bundleName,
      id:extName
    };
    inputMethod.switchInputMethod(inputMethodProperty).then(ret => {
      expect(ret).assertTrue();
      let property = inputMethod.getCurrentInputMethod();
      checkNewImeCurrentProp(property);
      console.info('************* inputmethod_test_switchInputMethod_001 Test end*************');
      wait(WAIT_DEAL_OK);
      expect(true).assertTrue();
      done();
    }).catch( err=> {
      console.info(`inputmethod_test_switchInputMethod_001 err: ${JSON.stringify(err.message)}`);
      expect().assertFail();
    })
  });

  /*
  * @tc.number  inputmethod_test_listCurrentInputMethodSubtype_001
  * @tc.name    Test list current input method subtypes.
  * @tc.desc    Function test
  * @tc.level   2
  */
  it('inputmethod_test_listCurrentInputMethodSubtype_001', 0, async function (done) {
    console.info('************* inputmethod_test_listCurrentInputMethodSubtype_001 Test start*************');
    let inputMethodSetting = inputMethod.getSetting();
    inputMethodSetting.listCurrentInputMethodSubtype((err, subProps) => {
      if (err) {
        console.error(`inputmethod_test_listCurrentInputMethodSubtype_001 err: ${ err }`);
        expect().assertFail();
        done();
        return;
      }
      checkNewImeSubProps(subProps);
      console.info('************* inputmethod_test_listCurrentInputMethodSubtype_001 Test end*************');
      expect(true).assertTrue();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_listCurrentInputMethodSubtype_002
   * @tc.name    Test list current input method subtypes.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_listCurrentInputMethodSubtype_002', 0, async function (done) {
    console.info('************* inputmethod_test_listCurrentInputMethodSubtype_002 Test start*************');
    let inputMethodSetting = inputMethod.getSetting();
    inputMethodSetting.listCurrentInputMethodSubtype().then((subProps)=>{
      checkNewImeSubProps(subProps);
      console.info('************* inputmethod_test_listCurrentInputMethodSubtype_002 Test end*************');
      expect(true).assertTrue();
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_listCurrentInputMethodSubtype_002 err: ${JSON.stringify(err.message)}`);
      expect().assertFail();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_listInputMethod_001
   * @tc.name    Test list input methods.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_listInputMethod_001', 0, async function (done) {
    console.info('************* inputmethod_test_listInputMethod_001 Test start*************');
    let inputMethodSetting = inputMethod.getInputMethodSetting();
    inputMethodSetting.listInputMethod((err, props) => {
      if (err) {
        console.error(`inputmethod_test_listInputMethod_001 err: ${ JSON.stringify(err.message) }`);
        expect().assertFail();
        done();
        return;
      }
      expect(props.length >= LEAST_ALL_IME_NUM).assertTrue();
      let imeProp = props.filter(function (prop) {return prop.name === bundleName && prop.id === extName;});
      expect(imeProp.length).assertEqual(LEAST_DISABLE_IME_NUM);
      let imeProp1 = props.filter(function (prop) {return prop.name === bundleName1;});
      expect(imeProp1.length).assertEqual(LEAST_DISABLE_IME_NUM);
      console.info('************* inputmethod_test_listInputMethod_001 Test end*************');
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_listInputMethod_002
   * @tc.name    Test list input methods.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_listInputMethod_002', 0, async function (done) {
    console.info('************* inputmethod_test_listInputMethod_002 Test start*************');
    let inputMethodSetting = inputMethod.getInputMethodSetting();
    await inputMethodSetting.listInputMethod().then((props) => {
      expect(props.length >= LEAST_ALL_IME_NUM).assertTrue();
      let imeProp = props.filter(function (prop) {return prop.name === bundleName && prop.id === extName;});
      expect(imeProp.length).assertEqual(LEAST_DISABLE_IME_NUM);
      let imeProp1 = props.filter(function (prop) {return prop.name === bundleName1;});
      expect(imeProp1.length).assertEqual(LEAST_DISABLE_IME_NUM);
      console.info('************* inputmethod_test_listInputMethod_002 Test end*************');
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_listInputMethod_002 err, ${JSON.stringify(err.message)} `);
      expect().assertFail();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_getInputMethods_001
   * @tc.name    Test get enable input methods.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_getInputMethods_001', 0, async function (done) {
    console.info('************* inputmethod_test_getInputMethods_001 Test start*************');
    let inputMethodSetting = inputMethod.getInputMethodSetting();
    await inputMethodSetting.getInputMethods(true).then((props)=>{
      expect(props.length).assertEqual(ENABLE_IME_NUM);
      let imeProp = props.filter(function (prop) {return prop.name === bundleName;});
      expect(imeProp.length).assertEqual(CURRENT_IME_COUNT);
      console.info('************* inputmethod_test_getInputMethods_001 Test end*************');
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_getInputMethods_001 err, ${JSON.stringify(err.message)} `);
      expect().assertFail();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_getInputMethods_002
   * @tc.name    Test get enable input methods.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_getInputMethods_002', 0, async function (done) {
    console.info('************* inputmethod_test_getInputMethods_002 Test start*************');
    let inputMethodSetting = inputMethod.getInputMethodSetting();
    inputMethodSetting.getInputMethods(true, (err, props) => {
      if (err) {
        console.error(`inputmethod_test_getInputMethods_002 err: ${JSON.stringify(err.message)}`);
        expect().assertFail();
        done();
        return;
      }
      let imeProp = props.filter(function (prop) {return prop.name === bundleName;});
      expect(imeProp.length).assertEqual(CURRENT_IME_COUNT);
      console.info('************* inputmethod_test_getInputMethods_002 Test end*************');
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_getInputMethods_003
   * @tc.name    Test get disable input methods.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_getInputMethods_003', 0, async function (done) {
    console.info('************* inputmethod_test_getInputMethods_003 Test start*************');
    let inputMethodSetting = inputMethod.getInputMethodSetting();
    await inputMethodSetting.getInputMethods(false).then((props)=>{
      expect(props.length >= NO_DISABLED_IME).assertTrue();
      let imeProp = props.filter(function (prop) {return prop.name === bundleName1;});
      expect(imeProp.length).assertEqual(NO_DISABLED_IME);
      console.info('************* inputmethod_test_getInputMethods_003 Test end*************');
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_getInputMethods_003 err, ${JSON.stringify(err.message)}`);
      expect().assertFail();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_getInputMethods_004
   * @tc.name    Test get disable input methods.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_getInputMethods_004', 0, async function (done) {
    console.info('************* inputmethod_test_getInputMethods_004 Test start*************');
    let inputMethodSetting = inputMethod.getInputMethodSetting();
    inputMethodSetting.getInputMethods(false, (err, props) => {
      if (err) {
        console.error(`inputmethod_test_getInputMethods_004 err, ${JSON.stringify(err.message)}`);
        expect().assertFail();
        done();
        return;
      }
      expect(props.length >= NO_DISABLED_IME).assertTrue();
      let imeProp = props.filter(function (prop) {return prop.name === bundleName1;});
      expect(imeProp.length).assertEqual(NO_DISABLED_IME);
      console.info('************* inputmethod_test_getInputMethods_004 Test end*************');
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_switchCurrentInputMethodSubtype_001
   * @tc.name    Test Indicates the input method subtype which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_switchCurrentInputMethodSubtype_001', 0, async function (done) {
    console.info('************* inputmethod_test_switchCurrentInputMethodSubtype_001 Test start*************');
    let InputMethodSubtype = {
      name: bundleName,
      id: subName[1],
      locale:'en_US.ISO-8859-1',
      language:'en',
      extra:{},
    };
    inputMethod.switchCurrentInputMethodSubtype(InputMethodSubtype).then(ret => {
      expect(ret).assertTrue();
      let subProp = inputMethod.getCurrentInputMethodSubtype();
      checkNewImeCurrentSubProp(subProp, 1);
      console.info('************* inputmethod_test_switchCurrentInputMethodSubtype_001 Test end*************');
      wait(WAIT_DEAL_OK);
      done();
    }).catch( err=> {
      console.info(`inputmethod_test_switchCurrentInputMethodSubtype_001 err, ${JSON.stringify(err.message)}`);
      expect().assertFail();
      done();
    })
  });

  /*
   * @tc.number  inputmethod_test_switchCurrentInputMethodSubtype_002
   * @tc.name    Test Indicates the input method which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_switchCurrentInputMethodSubtype_002', 0, async function (done) {
    console.info('************* inputmethod_test_switchCurrentInputMethodSubtype_002 Test start*************');
    let InputMethodSubtype = {
      name:bundleName,
      id:subName[0],
      locale:'en_US.ISO-8859-1',
      language:'en',
      extra:{},
    };
    inputMethod.switchCurrentInputMethodSubtype(InputMethodSubtype, (err, ret)=>{
      if(err){
        console.info(`inputmethod_test_switchCurrentInputMethodSubtype_002 err, ${JSON.stringify(err.message)}`);
        expect().assertFail();
        done();
        return;
      }
      expect(ret).assertTrue();
      let subProp = inputMethod.getCurrentInputMethodSubtype();
      checkNewImeCurrentSubProp(subProp, 0);
      console.info('************* inputmethod_test_switchCurrentInputMethodSubtype_002 Test end*************');
      wait(WAIT_DEAL_OK);
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_imeChange_001
   * @tc.name    Test to subscribe 'imeChange'.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_imeChange_001', 0, async function (done) {
    console.info('************* inputmethod_test_imeChange_001 Test start*************');
    expect(isImeChange).assertTrue();
    let subProp = inputMethod.getCurrentInputMethodSubtype();
    let prop = inputMethod.getCurrentInputMethod();
    expect(imeChangeSubProp.name).assertEqual(subProp.name);
    expect(imeChangeSubProp.id).assertEqual(subProp.id);
    expect(imeChangeProp.name).assertEqual(prop.name);
    expect(imeChangeProp.id).assertEqual(prop.id);
    console.info('************* inputmethod_test_imeChange_001 Test end*************');
    done();
  });

  /*
   * @tc.number  inputmethod_test_switchCurrentInputMethodAndSubtype_001
   * @tc.name    Test Indicates the input method subtype which will replace the current one.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_switchCurrentInputMethodAndSubtype_001', 0, async function (done) {
    console.info('************* inputmethod_test_switchCurrentInputMethodAndSubtype_001 Test start*************');
    let InputMethodSubtype = {
      name:bundleName1,
      id:extName1[0],
      locale:locale1[0],
      language:language1[0],
      extra:{},
    };
    let inputMethodProperty = {
      name:bundleName1,
      id:extName1[0],
    };
    inputMethod.switchCurrentInputMethodAndSubtype(inputMethodProperty, InputMethodSubtype).then(ret => {
      expect(ret).assertTrue();
      let subProp = inputMethod.getCurrentInputMethodSubtype();
      checkImeCurrentSubProp(subProp, 0);
      let property = inputMethod.getCurrentInputMethod();
      checkImeCurrentProp(property, 0);
      console.info('************* inputmethod_test_switchCurrentInputMethodAndSubtype_001 Test end*************');
      wait(WAIT_DEAL_OK);
      done();
    }).catch( err=> {
      console.info(`inputmethod_test_switchCurrentInputMethodAndSubtype_001 err, ${JSON.stringify(err.message)}`);
      expect().assertFail();
      done();
    })
  });

  /*
  * @tc.number  inputmethod_test_listInputMethodSubtype_001
  * @tc.name    Test list input method subtypes.
  * @tc.desc    Function test
  * @tc.level   2
  */
  it('inputmethod_test_listInputMethodSubtype_001', 0, async function (done) {
    console.info('************* inputmethod_test_listInputMethodSubtype_001 Test start*************');
    let inputMethodProperty = {
      name:bundleName,
      id:extName
    };
    let inputMethodSetting = inputMethod.getSetting();
    inputMethodSetting.listInputMethodSubtype(inputMethodProperty, (err, subProps) => {
      if (err) {
        console.error(`inputmethod_test_listInputMethodSubtype_001 err, ${JSON.stringify(err.message)}`);
        expect().assertFail();
        done();
        return;
      }
      checkNewImeSubProps(subProps);
      console.info('************* inputmethod_test_listInputMethodSubtype_001 Test end*************');
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_listInputMethodSubtype_002
   * @tc.name    Test list input method subtypes.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_listInputMethodSubtype_002', 0, async function (done) {
    console.info('************* inputmethod_test_listInputMethodSubtype_002 Test start*************');
    let inputMethodProperty = {
      name:bundleName1,
      id:extName1[0]
    };
    let inputMethodSetting = inputMethod.getSetting();
    inputMethodSetting.listInputMethodSubtype(inputMethodProperty).then((subProps)=>{
      checkImeSubProps(subProps);
      console.info('************* inputmethod_test_listInputMethodSubtype_002 Test end*************');
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_listInputMethodSubtype_002 err, ${JSON.stringify(err.message)}`);
      expect().assertFail();
      done();
    });
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
    inputMethodCtrl.showSoftKeyboard((err) => {
      if (err) {
        console.info(`inputmethod_test_showSoftKeyboard_001 err, ${JSON.stringify(err.message)}`);
        expect(err.code === 12800003).assertTrue();
        done();
        return;
      }
      console.info('************* inputmethod_test_showSoftKeyboard_001 Test end*************');
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
        console.info(`inputmethod_test_hideSoftKeyboard_001 err, ${JSON.stringify(err)}`);
        expect(err.code === 12800003).assertTrue();
        done();
        return;
      }
      console.info('************* inputmethod_test_hideSoftKeyboard_001  Test end*************');
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
        expect(err.code === 12800003).assertTrue();
        done();
        return;
      }
      expect().assertFalse();
      console.info('************* inputmethod_test_stopInputSession_001 Test end*************');
      done();
    });
  });


  /*
   * @tc.number  inputmethod_test_showSoftKeyboard_002
   * @tc.name    Test Indicates the input method which will show softboard with Promise.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_showSoftKeyboard_002', 0, async function (done) {
    console.info('************* inputmethod_test_showSoftKeyboard_002 Test start*************');
    let inputMethodCtrl = inputMethod.getInputMethodController();
    inputMethodCtrl.showSoftKeyboard().then(() =>{
      console.info('************* inputmethod_test_showSoftKeyboard_002 Test end*************' );
      expect().assertFail();
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_showSoftKeyboard_002 err, ${JSON.stringify(err.message)}`);
      expect(err.code === 12800003).assertTrue();
      done();
    })
  });

  /*
   * @tc.number  inputmethod_test_hideSoftKeyboard_002
   * @tc.name    Test Indicates the input method which will hide softboard with Promise.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_hideSoftKeyboard_002', 0, async function (done) {
    console.info('************* inputmethod_test_hideSoftKeyboard_002 Test start*************');
    let inputMethodCtrl = inputMethod.getInputMethodController();
    inputMethodCtrl.hideSoftKeyboard().then(() =>{
      console.info('************* inputmethod_test_hideSoftKeyboard_002 Test end*************' );
      expect().assertFail();
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_hideSoftKeyboard_002 err, ${JSON.stringify(err.message)}`);
      expect(err.code === 12800003).assertTrue();
      done();
    })
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
    inputMethodCtrl.stopInputSession().then((ret) => {
      expect().assertFail();
      console.info('************* inputmethod_test_stopInputSession_002 Test end*************' );
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_stopInputSession_002 err, ${JSON.stringify(err.message)}`);
      expect(err.code === 12800003).assertTrue();
      done();
    })
  });

  /*
   * @tc.number  inputmethod_test_switchInputMethod_002
   * @tc.name    Switch to ime before testcases run.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_switchInputMethod_002', 0, async function (done) {
    console.info('************* inputmethod_test_switchInputMethod_002 Test start*************');
    inputMethod.switchInputMethod(propertyBeforeSwitch).then(ret => {
      expect(ret).assertTrue();
      let property = inputMethod.getCurrentInputMethod();
      expect(property.name).assertEqual(propertyBeforeSwitch.name);
      expect(property.id).assertEqual(propertyBeforeSwitch.id);
      console.info('************* inputmethod_test_switchInputMethod_001 Test end*************');
      done();
    }).catch( err=> {
      console.info(`inputmethod_test_switchInputMethod_001 err: ${JSON.stringify(err.message)}`);
      expect().assertFail();
    })
  });

  /*
   * @tc.number  inputmethod_test_attach_001
   * @tc.name    Test whether the current application can be bound with the default input method.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_attach_001', 0, async function (done) {
    console.info('************* inputmethod_test_attach_001 Test start*************');
    let inputMethodCtrl = inputMethod.getInputMethodController();
    let attribute = {
      textInputType: inputMethod.TextInputType.TEXT,
      enterKeyType: inputMethod.EnterKeyType.NONE
    };
    let textConfig = {
      inputAttribute: attribute
    };
    try {
      inputMethodCtrl.attach(false, textConfig, (err)=>{
        if (err) {
          console.info(`inputmethod_test_attach_001 result: ${JSON.stringify(err)}`);
          expect().assertFail();
          done();
        }
        console.info('************* inputmethod_test_attach_001 Test end*************');
        expect(true).assertTrue();
        done();
      });
    } catch (error) {
      console.info(`inputmethod_test_attach_001 error, result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_attach_002
   * @tc.name    Test whether the current application can be bound with the default input method.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_attach_002', 0, async function (done) {
    console.info('************* inputmethod_test_attach_002 Test start*************');
    let inputMethodCtrl = inputMethod.getInputMethodController();
    let attribute = {
      textInputType: inputMethod.TextInputType.TEXT,
      enterKeyType: inputMethod.EnterKeyType.NONE
    };
    let textConfig = {
      inputAttribute: attribute
    };
    try {
      inputMethodCtrl.attach(false, textConfig).then(()=>{
        console.info('************* inputmethod_test_attach_002 Test end*************');
        expect(true).assertTrue();
        done();
      }).catch((err) => {
        console.info(`inputmethod_test_attach_002 err result: ${JSON.stringify(err)}`);
        expect().assertFail();
        done();
      })
    } catch (error) {
      console.info(`inputmethod_test_attach_002 error result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_detach_001
   * @tc.name    Test whether it can successfully unbind with input method.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_detach_001', 0, async function (done) {
    console.info('************* inputmethod_test_detach_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.detach((err) => {
      if (err) {
        console.info(`inputmethod_test_detach_001 result: ${JSON.stringify(err)}`);
        expect().assertFail();
        done();
      }
      console.info('inputmethod_test_detach_001 callback success');
      expect(true).assertTrue();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_detach_002
   * @tc.name    Test whether the keyboard is hide successfully.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_detach_002', 0, async function (done) {
    console.info('************* inputmethod_test_detach_002 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    console.info(`inputmethod_test_detach_002 result: ${JSON.stringify(inputMethodCtrl)}`);
    inputMethodCtrl.detach().then(() => {
      console.info('inputmethod_test_detach_002 promise success.');
      expect(true).assertTrue();
      done();
    }).catch((err) => {
      console.info(`inputmethod_test_detach_002 result: ${JSON.stringify(err)}`);
      expect().assertFail();
      done();
    });
  });

  /*
   * @tc.number  inputmethod_test_showTextInput_001
   * @tc.name    Test whether the keyboard is displayed successfully.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_showTextInput_001', 0, async function (done) {
    console.info('************* inputmethod_test_showTextInput_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
      inputMethodCtrl.showTextInput((err) => {
        console.info(`inputmethod_test_detach_002 result: ${JSON.stringify(err)}`);
        if (err.code === 12800009) {
          console.info(`inputmethod_test_detach_002 err.code === 12800009.`);
          expect(true).assertTrue();
        } else {
          expect().assertFail();
        }
        done();
      })
  });

  /*
   * @tc.number  inputmethod_test_hideTextInput_001
   * @tc.name    Test whether the keyboard is hidden successfully.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_hideTextInput_001', 0, async function (done) {
    console.info('************* inputmethod_test_hideTextInput_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    inputMethodCtrl.hideTextInput((err) => {
      console.info(`inputmethod_test_setCallingWindow_001 err: ${JSON.stringify(err)}`);
      if (err.code === 12800009) {
        expect(true).assertTrue();
      } else {
        expect().assertFail();
      }
      done();
    })
  });

  /*
   * @tc.number  inputmethod_test_setCallingWindow_001
   * @tc.name    Test whether set calling window successfully.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_setCallingWindow_001', 0, async function (done) {
    console.info('************* inputmethod_test_setCallingWindow_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    let windowId = 100;
    try {
      inputMethodCtrl.setCallingWindow(windowId, (err) => {
        console.info(`inputmethod_test_setCallingWindow_001 err: ${JSON.stringify(err)}`);
        if (err.code === 12800009) {
          expect(true).assertTrue();
        } else {
          expect().assertFail();
        }
        done();
      })
    } catch (error) {
      console.info(`inputmethod_test_setCallingWindow_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_setCallingWindow_002
   * @tc.name    Test whether set calling window successfully when type of param is wrong.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_setCallingWindow_002', 0, async function (done) {
    console.info('************* inputmethod_test_setCallingWindow_002 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    let windowId = '100';
    try {
      inputMethodCtrl.setCallingWindow(windowId, (err) => {
        expect().assertFail();
        done();
      })
    } catch (error) {
      console.info(`inputmethod_test_setCallingWindow_002 result: ${JSON.stringify(error)}`);
      expect(true).assertTrue();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_updateCursor_001
   * @tc.name    Test whether update cursor successfully.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_updateCursor_001', 0, async function (done) {
    console.info('************* inputmethod_test_updateCursor_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    let cursorInfo = { left: 100, top: 110, width: 600, height: 800 };
    try {
      inputMethodCtrl.updateCursor(cursorInfo, (err) => {
        if (err.code === 12800009) {
          expect(true).assertTrue();
        } else {
          expect().assertFail();
        }
        done();
      })
    } catch (error) {
      console.info(`inputmethod_test_updateCursor_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_changeSelection_001
   * @tc.name    Test whether change selection successfully.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_changeSelection_001', 0, async function (done) {
    console.info('************* inputmethod_test_changeSelection_001 Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    let text = 'test';
    let start = 0;
    let end = 5;
    try {
      inputMethodCtrl.changeSelection(text, start, end, (err) => {
        if (err.code === 12800009) {
          expect(true).assertTrue();
        } else {
          expect().assertFail();
        }
        done();
      });
    } catch (error) {
      console.info(`inputmethod_test_changeSelection_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_updateAttribute
   * @tc.name    Test whether update attribute successfully.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_updateAttribute', 0, async function (done) {
    console.info('************* inputmethod_test_updateAttribute Test start*************');
    let inputMethodCtrl = inputMethod.getController();
    let attribute = {textInputType: inputMethod.TextInputType.TEXT, enterKeyType: inputMethod.EnterKeyType.NONE};
    try {
      inputMethodCtrl.updateAttribute(attribute, (err) => {
        if (err.code === 12800009) {
          expect(true).assertTrue();
        } else {
          expect().assertFail();
        }
        done();
      });
    } catch (error) {
      console.info(`inputmethod_test_updateAttribute result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_on_insertText_001
   * @tc.name    Test whether the register the callback of the input method is valid.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_on_insertText_001', 0, async function (done) {
    let inputMethodCtrl = inputMethod.getController();
    try {
      inputMethodCtrl.on('insertText', (text) => {
      });
      expect().assertFail();
      done();
    } catch (error) {
      console.info(`inputmethod_test_on_insertText_001 result: ${JSON.stringify(error)}`);
      expect(error.code === 12800009).assertTrue();
      done();
    }
  });

  /*
  * @tc.number  inputmethod_test_on_deleteLeft_001
  * @tc.name    Test whether the register the callback of the input method is valid.
  * @tc.desc    Function test
  * @tc.level   2
  */
  it('inputmethod_test_on_deleteLeft_001', 0, async function (done) {
    let inputMethodCtrl = inputMethod.getController();
    try {
      inputMethodCtrl.on('deleteLeft', (length) => {});
      expect().assertFail();
      done();
    } catch (error) {
      console.info(`inputmethod_test_on_deleteLeft_001 result: ${JSON.stringify(error)}`);
      expect(error.code === 12800009).assertTrue();
      done();
    }
  });

  /*
  * @tc.number  inputmethod_test_on_deleteRight_001
  * @tc.name    Test whether the register the callback of the input method is valid.
  * @tc.desc    Function test
  * @tc.level   2
  */
  it('inputmethod_test_on_deleteRight_001', 0, async function (done) {
      let inputMethodCtrl = inputMethod.getController();
      try {
          inputMethodCtrl.on('deleteRight', (length) => {});
          expect().assertFail();
          done();
      } catch (error) {
          console.info(`inputmethod_test_on_deleteRight_001 result: ${JSON.stringify(error)}`);
          expect(error.code === 12800009).assertTrue();
          done();
      }
  });

  /*
  * @tc.number  inputmethod_test_on_sendKeyboardStatus_001
  * @tc.name    Test whether the register the callback of the input method is valid.
  * @tc.desc    Function test
  * @tc.level   2
  */
  it('inputmethod_test_on_sendKeyboardStatus_001', 0, async function (done) {
    let inputMethodCtrl = inputMethod.getController();
    try {
      inputMethodCtrl.on('sendKeyboardStatus', (status) => {});
      expect().assertFail();
      done();
    } catch (error) {
      console.info(`inputmethod_test_on_sendKeyboardStatus_001 result: ${JSON.stringify(error)}`);
      expect(error.code === 12800009).assertTrue();
      done();
    }
  });

  /*
  * @tc.number  inputmethod_test_on_sendKeyboardStatus_001
  * @tc.name    Test whether the register the callback of the input method is valid.
  * @tc.desc    Function test
  * @tc.level   2
  */
  it('inputmethod_test_on_sendFunctionKey_001', 0, async function (done) {
    let inputMethodCtrl = inputMethod.getController();
    try {
      inputMethodCtrl.on('sendFunctionKey', (functionKey) => {});
      expect().assertFail();
      done();
    } catch (error) {
      console.info(`inputmethod_test_on_sendFunctionKey_001 result: ${JSON.stringify(error)}`);
      expect(error.code === 12800009).assertTrue();
      done();
    }
  });

  /*
  * @tc.number  inputmethod_test_on_moveCursor_001
  * @tc.name    Test whether the register the callback of the input method is valid.
  * @tc.desc    Function test
  * @tc.level   2
  */
  it('inputmethod_test_on_moveCursor_001', 0, async function (done) {
    let inputMethodCtrl = inputMethod.getController();
    try {
      inputMethodCtrl.on('moveCursor', (direction) => {});
      expect().assertFail();
      done();
    } catch (error) {
      console.info(`inputmethod_test_on_moveCursor_001 result: ${JSON.stringify(error)}`);
      expect(error.code === 12800009).assertTrue();
      done();
    }
  });

  /*
  * @tc.number  inputmethod_test_on_handleExtendAction_001
  * @tc.name    Test whether the register the callback of the input method is valid.
  * @tc.desc    Function test
  * @tc.level   2
  */
  it('inputmethod_test_on_handleExtendAction_001', 0, async function (done) {
    let inputMethodCtrl = inputMethod.getController();
    try {
      inputMethodCtrl.on('handleExtendAction', (action) => {});
      expect().assertFail();
      done();
    } catch (error) {
      console.info(`inputmethod_test_on_handleExtendAction_001 result: ${JSON.stringify(error)}`);
      expect(error.code === 12800009).assertTrue();
      done();
    }
  });

  /*
   * @tc.number  inputmethod_test_off_001
   * @tc.name    Test whether the unregister the callback of the input method is valid.
   * @tc.desc    Function test
   * @tc.level   2
   */
  it('inputmethod_test_off_001', 0, async function (done) {
    let inputMethodCtrl = inputMethod.getController();
    try {
      inputMethodCtrl.off('insertText');
      inputMethodCtrl.off('deleteLeft');
      inputMethodCtrl.off('deleteRight');
      inputMethodCtrl.off('sendKeyboardStatus');
      inputMethodCtrl.off('sendFunctionKey');
      inputMethodCtrl.off('moveCursor');
      inputMethodCtrl.off('handleExtendAction');
      expect(true).assertTrue();
      done();
    } catch(error) {
      console.info(`inputmethod_test_off_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

  /*
  * @tc.number  inputmethod_settings_test_on_imeShow_001
  * @tc.name    Test whether the register the callback of the input method is valid.
  * @tc.desc    Function test
  * @tc.level   2
  */
  it('inputmethod_setting_test_on_imeShow_001', 0, async function (done) {
    let inputMethodSetting = inputMethod.getSetting();
    try {
      inputMethodSetting.on('imeShow', (info) => {});
      expect(true).assertTrue();
      done();
    } catch (error) {
      console.info(`inputmethod_setting_test_on_imeShow_001 result: ${JSON.stringify(error)}`);
      expect().assertFail();
      done();
    }
  });

    /*
  * @tc.number  inputmethod_settings_test_on_imeHide_001
  * @tc.name    Test whether the register the callback of the input method is valid.
  * @tc.desc    Function test
  * @tc.level   2
  */
    it('inputmethod_setting_test_on_imeHide_001', 0, async function (done) {
      let inputMethodSetting = inputMethod.getSetting();
      try {
        inputMethodSetting.on('imeHide', (info) => {});
        expect(true).assertTrue();
        done();
      } catch (error) {
        console.info(`inputmethod_setting_test_on_imeHide_001 result: ${JSON.stringify(error)}`);
        expect().assertFail();
        done();
      }
    });
});