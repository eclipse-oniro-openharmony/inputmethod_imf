/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

import ImeExtension from '@ohos.inputmethodextensionability'
import Logger from '../model/Logger'
import InputMethodModel from '../model/InputMethodModel'
const ERROR_CODE = -1;
const SUCCESS_CODE = 1;

export default class InputMethodExtAbility extends ImeExtension {
    private inputMethodModel:InputMethodModel;
    onCreate(want) {
        Logger.log(`InputMethodExtAbility onCreate, want: ${want.abilityName}`);
        this.inputMethodModel = new InputMethodModel(this.context);
        // test startAbility
        this.inputMethodModel.startAbility(function (code) {
            if (code == SUCCESS_CODE) {
                console.log("InputMethodExtAbility startAbility success");
            } else {
                console.log("InputMethodExtAbility startAbility failed");
            }
        })
    }
    onDestroy() {
        Logger.log(`InputMethodExtAbility onDestroy`);
    }
}