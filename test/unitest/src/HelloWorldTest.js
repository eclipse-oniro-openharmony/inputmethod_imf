/*
 * Copyright (C) 2021 XXXX Device Co., Ltd.
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
import inputMethod from '@ohos.inputmethod'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe("HelloWorldTest", function () {
    beforeAll(function() {
        // input testsuit setup step，setup invoked before all testcases
        console.info('beforeAll caled')
    })

    afterAll(function() {
        // input testsuit teardown step，teardown invoked after all testcases
        console.info('afterAll caled')
    })

    beforeEach(function() {
        // input testcase setup step，setup invoked before each testcases
        console.info('beforeEach caled')
    })

    afterEach(function() {
        // input testcase teardown step，teardown invoked after each testcases
        console.info('afterEach caled')
    })

    /*
     * @tc.name:appInfoTest001
     * @tc.desc:verify app info is not null
     * @tc.type: FUNC
     * @tc.require: issueNumber
     */
    it("HelloWorldTest", 0, function () {
        //step 1:调用函数获取结果
        let a=-1
        inputMethod.getInputMethodSetting().listInputMethod().then((successMessage)=>{
         a=successMessage.length()
        })

        //Step 2:使用断言比较预期与实际结果
        expect(a>0).assertEqual(true)
    })
})