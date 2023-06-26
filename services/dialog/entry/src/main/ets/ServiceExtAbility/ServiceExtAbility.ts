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
import ServiceExtensionAbility from '@ohos.app.ability.ServiceExtensionAbility';
import window from '@ohos.window';
import display from '@ohos.display';
import inputMethod from '@ohos.inputMethod';
import prompt from '@ohos.prompt';
import commonEvent from '@ohos.commonEvent';

let TAG = '[InputMethodChooseDialog]';
let commonEvent1 = 'usual.event.PACKAGE_ADDED';
let commonEvent2 = 'usual.event.PACKAGE_REMOVED';
let subscribeInfo = {
  events: [commonEvent1, commonEvent2]
};
const EXIT_TIME = 1000;
export default class ServiceExtAbility extends ServiceExtensionAbility {
  onCreate(want): void {
    console.log(TAG, 'onCreate');
    globalThis.windowNum = 0;
  }

  onRequest(want, startId): void {
    console.log(TAG, 'onRequest execute');
    globalThis.abilityWant = want;
    display.getDefaultDisplay().then(() => {
      let dialogRect = {
        left: 50,
        top: 900,
        width: 300,
        height: 300,
      };
      let windowConfig = {
        name:'inputmethod Dialog',
        windowType:window.WindowType.TYPE_FLOAT,
        ctx:this.context
      };
      this.getInputMethods().then(() => {
        this.createWindow(windowConfig, dialogRect);
      });
    }).catch((err) => {
      console.log(TAG + 'getDefaultDisplay err:' + JSON.stringify(err));
    });

    commonEvent.createSubscriber(subscribeInfo, (error, subcriber) => {
      commonEvent.subscribe(subcriber, (error, commonEventData) => {
        if (commonEventData.event === commonEvent1 || commonEventData.event === commonEvent2) {
          console.log(TAG + 'commonEvent:' + JSON.stringify(commonEvent1));
          this.updateImeList();
        }
      });
    });

    globalThis.chooseInputMethods = ((prop: inputMethod.InputMethodProperty): void => {
      inputMethod.switchInputMethod(prop).then((err) => {
        if (!err) {
          console.log(TAG + 'switchInputMethod failed,' + JSON.stringify(err));
          prompt.showToast({
            message: 'switch failed', duration: 200
          });
        } else {
          console.log(TAG + 'switchInputMethod success');
          prompt.showToast({
            message: 'switch success', duration: 200
          });
        }
        setTimeout(() => {
          this.releaseContext();
        }, EXIT_TIME);
      });
    });
  }

  onDestroy(): void {
    console.log(TAG + 'ServiceExtAbility destroyed');
    this.releaseContext();
  }

  private async createWindow(config: window.Configuration, rect): Promise<void> {
    console.log(TAG + 'createWindow execute');
    try {
      if (globalThis.windowNum > 0) {
        this.updateImeList();
        return;
      }
      try {
        await window.createWindow(config, async (err, data) => {
          if (err.code) {
            console.error('Failed to create the window. Cause: ' + JSON.stringify(err));
            return;
          }
          const win = data;
          globalThis.extensionWin = win;
          console.info('Succeeded in creating the window. Data: ' + JSON.stringify(data));
          win.on('windowEvent', async (data) => {
            console.log(TAG + 'windowEvent:' + JSON.stringify(data));
            if (data === window.WindowEventType.WINDOW_INACTIVE) {
              await this.releaseContext();
            }
          });
          await win.moveTo(rect.left, rect.top);
          await win.resetSize(rect.width, rect.height);
          await win.loadContent('pages/index');
          await win.show();
        });
      } catch (exception) {
        console.error('Failed to create the window. Cause: ' + JSON.stringify(exception));
      }
      globalThis.context = this.context;
      globalThis.windowNum++;
      console.log(TAG + 'window create successfully');
    } catch {
      console.info(TAG + 'window create failed');
    }
  }

  private async getInputMethods(): Promise<void> {
    globalThis.inputMethodList = [];
    try {
      let enableList = await inputMethod.getSetting().getInputMethods(true);
      let disableList = await inputMethod.getSetting().getInputMethods(false);
      globalThis.inputMethodList = [...enableList, ...disableList];
    } catch {
      console.log(TAG + 'getInputMethods failed');
    }
  }

  private async updateImeList(): Promise<void> {
    await this.getInputMethods().then(() => {
      if (!globalThis.extensionWin.isWindowShowing()) {
        globalThis.extensionWin.show();
      }
      globalThis.extensionWin.setUIContent('pages/index');
    });
  }

  private async releaseContext(): Promise<void> {
    if (globalThis.context !== null) {
      await globalThis.extensionWin.destroy();
      await globalThis.context.terminateSelf();
      globalThis.context = null;
    }
  }
};
