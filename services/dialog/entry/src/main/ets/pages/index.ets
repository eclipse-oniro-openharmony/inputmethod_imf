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
import inputMethod from '@ohos.inputmethod';
import prompt from '@ohos.prompt';

@Entry
@Component
struct Dialog {
  private arr: string[] = []
  private properityMap: Map<string, inputMethod.InputMethodProperty> = new Map();
  @State private win: any = undefined
  @State private context: any = undefined
  @State display: string = '所有输入法'
  @State btnColor: string = '#bbccaa';
  private inputMethods: Array<inputMethod.InputMethodProperty> = null
  private TAG = "[InputMethodChooseDialog]"

  aboutToAppear() {
    console.log(this.TAG, "dialog page appears")
    this.context = globalThis.context
    this.win = globalThis.extensionWin
    this.inputMethods = globalThis.inputMethodList

    for (let inputmethod of this.inputMethods) {
      let name = inputmethod.packageName
      this.arr.push(name)
      this.properityMap.set(name, inputmethod)
    }
  }

  onPrint() {
    console.log(this.TAG + "print file or text")
  }

  onCopy() {
    console.log(this.TAG + "copy file and html")
  }

  build() {
    Column() {
      List({ space: 1, initialIndex: 0 }) {
        ListItem() {
          Text(this.display)
            .width('100%')
            .height(40)
            .fontSize(14)
            .textAlign(TextAlign.Center)
            .backgroundColor(Color.Pink)
            .onClick(() => {
              this.btnColor = '#aaaaaa'
            })
        }.sticky(Sticky.Normal)

        ForEach(this.arr, (item, index) => {
          ListItem() {
            Text(item.split('.')[2])
              .width('100%')
              .height(60)
              .fontSize(16)
              .textAlign(TextAlign.Center)
              .borderRadius(10)
              .backgroundColor(this.btnColor)
              .onClick(async () => {
                if (this.properityMap.has(item)) {
                  let prop = this.properityMap.get(item)
                  await inputMethod.switchInputMethod(prop).then((err) => {
                    if (!err) {
                      console.log(this.TAG + "switchInputMethod failed, " + JSON.stringify(err))
                      prompt.showToast({ message: 'switch failed', duration: 200 })
                    } else {
                      console.log(this.TAG + "switchInputMethod success ")
                      prompt.showToast({ message: 'switch success', duration: 200 })
                    }
                  })
                }
                this.btnColor = '#aaaaaa'
                setTimeout(() => {
                  this.win.destroy()
                  this.context.terminateSelf()
                }, 1000)
              })
          }
          .sticky(0 == index ? Sticky.Opacity : Sticky.None)
        }, item => item)
      }
    }.width('100%').height('100%').backgroundColor(0xDCDCDC).padding({ top: 5 })
  }
}