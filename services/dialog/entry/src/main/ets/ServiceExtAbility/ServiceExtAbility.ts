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
import common from '@ohos.app.ability.common';
import window from '@ohos.window';
import inputMethod from '@ohos.inputMethod';
import commonEvent from '@ohos.commonEventManager';
import Want from '@ohos.app.ability.Want';
import display from '@ohos.display';
import { BusinessError } from '@ohos.base';

let TAG: string = '[InputMethodChooseDialog]';
let PACKAGE_ADDED: string = 'usual.event.PACKAGE_ADDED';
let PACKAGE_REMOVED: string = 'usual.event.PACKAGE_REMOVED';
let subscribeInfo: commonEvent.CommonEventSubscribeInfo = {
  events: [PACKAGE_ADDED, PACKAGE_REMOVED]
};

interface DialogRect {
  left: number;
  top: number;
  width: number;
  height: number;
}

const DISPLAY_SCALE: number = 0.35;
const MIN_SIZE: number = 350;
const MAX_SZIE: number = 550;
const DIALOG_POSITION_X: number = 50;
const DIALOG_POSITION_Y_SCALE: number = 0.5;

export default class ServiceExtAbility extends ServiceExtensionAbility {
  private extensionWin: window.Window | undefined = undefined;
  private mContext: common.ServiceExtensionContext | undefined = undefined;
  private windowNum: number = 0;

  onCreate(want: Want): void {
    console.log(TAG, 'onCreate');
    this.windowNum = 0;
    this.mContext = this.context;
  }

  onRequest(want: Want, startId: number): void {
    console.log(TAG, 'onRequest execute');
    let defaultDisplay = display.getDefaultDisplaySync();
    let size = defaultDisplay.width * DISPLAY_SCALE > MIN_SIZE ? defaultDisplay.width * DISPLAY_SCALE : MIN_SIZE;
    size = size < MAX_SZIE ? size : MAX_SZIE;
    let dialogTop = defaultDisplay.height * DIALOG_POSITION_Y_SCALE;
    let dialogRect: DialogRect = {
      left: DIALOG_POSITION_X,
      top: dialogTop,
      width: size,
      height: size
    };
    let windowConfig: window.Configuration = {
      name: 'inputmethod Dialog',
      windowType: window.WindowType.TYPE_FLOAT,
      ctx: this.mContext
    };
    this.getInputMethods().then(() => {
      this.createWindow(windowConfig, dialogRect);
    });

    commonEvent.createSubscriber(subscribeInfo, (error: BusinessError, subcriber: commonEvent.CommonEventSubscriber) => {
      commonEvent.subscribe(subcriber, (error: BusinessError, commonEventData: commonEvent.CommonEventData) => {
        console.log(TAG + 'commonEvent:' + JSON.stringify(commonEventData.event));
        if (commonEventData.event === PACKAGE_ADDED || commonEventData.event === PACKAGE_REMOVED) {
          this.updateImeList();
        }
      });
    });
  }

  onDestroy(): void {
    console.log(TAG + 'ServiceExtAbility destroyed');
    this.releaseContext();
  }

  private async createWindow(config: window.Configuration, rect: DialogRect): Promise<void> {
    console.log(TAG + 'createWindow execute');
    try {
      if (this.windowNum > 0) {
        this.updateImeList();
        return;
      }
      try {
        this.extensionWin = await window.createWindow(config);
        console.info(TAG + 'Succeeded in creating the window. Data: ' + JSON.stringify(this.extensionWin));
        this.extensionWin.on('windowEvent', async (data: window.WindowEventType) => {
          console.log(TAG + 'windowEvent:' + JSON.stringify(data));
          if (data === window.WindowEventType.WINDOW_INACTIVE) {
            await this.releaseContext();
          }
        });
        await this.extensionWin.moveWindowTo(rect.left, rect.top);
        await this.extensionWin.resize(rect.width, rect.height);
        await this.extensionWin.setUIContent('pages/index');
        await this.extensionWin.showWindow();
        this.windowNum++;
        console.log(TAG + 'window create successfully');
      } catch (exception) {
        console.error('Failed to create the window. Cause: ' + JSON.stringify(exception));
      }
    } catch {
      console.info(TAG + 'window create failed');
    }
  }

  private async getInputMethods(): Promise<void> {
    let inputMethodList: Array<inputMethod.InputMethodProperty> = [];
    try {
      let enableList = await inputMethod.getSetting().getInputMethods(true);
      let disableList = await inputMethod.getSetting().getInputMethods(false);
      inputMethodList = enableList.concat(disableList);
      AppStorage.setOrCreate('inputMethodList', inputMethodList);
    } catch {
      console.log(TAG + 'getInputMethods failed');
    }
  }

  private async updateImeList(): Promise<void> {
    await this.getInputMethods().then(async () => {
      if (this.extensionWin) {
        await this.extensionWin.setUIContent('pages/index');
        if (!this.extensionWin.isWindowShowing()) {
          await this.extensionWin.showWindow();
        }
      }
    });
  }

  public async releaseContext(): Promise<void> {
    await this.extensionWin?.destroyWindow();
    await this.mContext?.terminateSelf();
  }
};