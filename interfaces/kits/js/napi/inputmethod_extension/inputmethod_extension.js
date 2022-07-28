/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

var WindowManager = requireNapi("window")
var WindowName = "inputmethod"
var windowType = 2000
var windowsCreated = false;

class InputMethodExtension {
    createInputMethodWin(){
        console.log(WindowName + " createInputMethodWin");
        console.log(this.context);
        console.log(WindowName + JSON.stringify(this.context));
        console.log(windowType);
        console.log(WindowName + JSON.stringify(windowType));

        WindowManager.create(this.context, WindowName, windowType).then((win) => {
            console.log(WindowName + "inputmethodWindow");
            this.inputmethodWindow = win;
            console.log(WindowName + JSON.stringify(this.inputmethodWindow));
            console.log(this.inputmethodWindow);
            console.log(WindowName + "moveTo");
            this.inputmethodWindow.moveTo(0, 0).then(() => {
                console.log(WindowName + "resetSize");
                this.inputmethodWindow.resetSize(480, 960).then(() => {
                    console.log(WindowName + " resetSize"+JSON.stringify(this.inputmethodURL));
                    this.loadUiContent(this.inputmethodURL);
                    console.log(WindowName + " window created");
                    windowsCreated = true;
                })
            })
        }, (error) => {
            console.log(WindowName + " window createFailed, error.code = " + error.code);
        })
    }

    onCreated(want){
        console.log(WindowName + "onInputMethodExtensionCreated");
    }

    setUiContent(url) {
        console.log(WindowName + " setUiContent");
        if (windowsCreated) {
            console.log(WindowName + " loadUiContent");
            loadUiContent(url);
        } else {
            console.log(WindowName + " save inputmethodURL");
            this.inputmethodURL = url;
        }
    }

    loadUiContent(url){
        console.log(WindowName + "initUiContent"+url);
        console.log(WindowName + JSON.stringify(this.inputmethodWindow));
        console.log(WindowName + JSON.stringify(this.inputmethodWindow.loadContent));
        console.log(WindowName + JSON.stringify(this.inputmethodWindow.loadContent(url)));
        this.inputmethodWindow.loadContent(url).then(() => {
            console.log(WindowName + " loadContent");
            this.inputmethodWindow.show().then(() => {
                console.log(WindowName + " window is show");
            })
        })
    }

    onInputMethodChanged(inputmethodType){
        console.log(WindowName + "onInputMethodChanged"+JSON.stringify(inputmethodType));
    }

    onDestroy(){
        console.log(WindowName + "onInputMethodExtensionDestroy");
    }
}

export default InputMethodExtension