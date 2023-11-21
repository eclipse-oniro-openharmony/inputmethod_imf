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
import common from '@ohos.app.ability.common';
import inputMethod from '@ohos.inputMethod';
import prompt from '@ohos.promptAction';
import { BusinessError } from '@ohos.base';

const EXIT_TIME = 1000;

@Entry
@Component
struct Index {
  private arr: string[] = [];
  private propertyMap: Map<string, inputMethod.InputMethodProperty> = new Map();
  @StorageLink('inputMethodList') inputMethods: Array<inputMethod.InputMethodProperty> | undefined = [];
  private TAG = "[InputMethodChooseDialog]";

  aboutToAppear() {
    console.log(this.TAG, "dialog page appears")
    this.inputMethods = AppStorage.get('inputMethodList');
    if (this.inputMethods) {
      for (let inputmethod of this.inputMethods) {
        let name = inputmethod.packageName;
        this.arr.push(name);
        this.propertyMap.set(name, inputmethod);
      }
    }
  }

  onPrint(): void {
    console.log(this.TAG + "print file or text");
  }

  onCopy(): void {
    console.log(this.TAG + "copy file and html");
  }

  build() {
    Column() {
      List({ space: 1, initialIndex: 0 }) {
        ListItem() {
          Text($r("app.string.dialogTitle"))
            .width('100%')
            .height(40)
            .fontSize(14)
            .textAlign(TextAlign.Center)
            .backgroundColor(Color.Pink)
        }.sticky(Sticky.Normal)

        ForEach(this.arr, (item: string, index: number) => {
          ListItem() {
            Text(item.split('.').length > 2 ? item.split('.')[2] : item.split('.')[-1])
              .width('100%')
              .height(60)
              .fontSize(16)
              .textAlign(TextAlign.Center)
              .borderRadius(10)
              .backgroundColor($r("app.color.btn_default"))
              .onClick(async () => {
                if (this.propertyMap.has(item)) {
                  let prop = this.propertyMap.get(item);
                  this.chooseInputMethods(prop);
                }
              })
          }
          .sticky(0 == index ? Sticky.Opacity : Sticky.None)
        }, (item: string) => item)
      }
      .width('100%')
      .height('100%')
    }.width('100%').height('100%').backgroundColor(0xDCDCDC).padding({ top: 5 })
  }

  chooseInputMethods(prop: inputMethod.InputMethodProperty): void {
    let context = getContext(this) as common.ServiceExtensionContext;
    inputMethod.switchInputMethod(prop).then(() => {
      console.log(this.TAG + 'switchInputMethod success');
      prompt.showToast({
        message: 'switch success', duration: 200
      });
      setTimeout(() => {
        context.terminateSelf();
      }, EXIT_TIME);
    }).catch((err: BusinessError) => {
      if (!err) {
        console.log(this.TAG + 'switchInputMethod failed,' + JSON.stringify(err));
        prompt.showToast({
          message: 'switch failed', duration: 200
        });
      }
    });
  }
}