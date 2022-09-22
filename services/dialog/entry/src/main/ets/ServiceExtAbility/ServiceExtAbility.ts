/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
import extension from '@ohos.application.ServiceExtensionAbility'
import window from '@ohos.window';
import display from '@ohos.display';
import inputMethod from '@ohos.inputmethod';

let TAG = "[InputMethodChooseDialog]"

export default class ServiceExtAbility extends extension {
    onCreate(want) {
        console.log(TAG, "InputMethod ServiceExtensionAbility onCreate")
        globalThis.context = this.context
        globalThis.windowNum = 0
    }

    onRequest(want, startId) {
        console.log(TAG, "onRequest execute")
        globalThis.abilityWant = want
        display.getDefaultDisplay().then(() => {
            let dialogRect = {
                left:50,
                top:900,
                width:300,
                height:300,
            }
            this.getInputMethods().then(() => {
                this.createWindow("inputmethod Dialog:" + startId, window.WindowType.TYPE_DIALOG, dialogRect)
            })
        }).catch((err) => {
            console.log(TAG + "getDefaultDisplay err: " + JSON.stringify(err));
        });
    }

    onDestroy() {
        console.log(TAG + "ServiceExtAbility destroyed")
        globalThis.extensionWin.destroy()
        globalThis.context.terminateSelf()
    }

    private async createWindow(name: string, windowType: number, rect) {
        console.log(TAG + "createWindow execute")
        try {
            const win = await window.create(this.context, name, windowType)
            globalThis.extensionWin = win
            await win.moveTo(rect.left, rect.top)
            await win.resetSize(rect.width, rect.height)
            await win.loadContent('pages/index')
            await win.show()
            globalThis.windowNum ++
            console.log(TAG + "window create successfully")
        } catch {
            console.info(TAG + "window create failed")
        }
    }

    private async getInputMethods() {
        globalThis.inputMethodList = []
        try {
            let enableList = await inputMethod.getInputMethodSetting().listInputMethod(true)
            let disableList = await inputMethod.getInputMethodSetting().listInputMethod(false)
            globalThis.inputMethodList = [...enableList, ...disableList]
        } catch {
            console.log(TAG + "getInputMethods failed ");
        }
    }
};