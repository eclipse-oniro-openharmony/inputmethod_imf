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

import InputMethodSubtype from '@ohos.InputMethodSubtype';
import inputMethod from '@ohos.inputMethod';
import componentUtils from '@ohos.arkui.componentUtils';
import settings from '@ohos.settings';
import common from '@ohos.app.ability.common';
import display from '@ohos.display';
import bundleManager from '@ohos.bundle.bundleManager';
import inputMethodEngine from '@ohos.inputMethodEngine';

export interface PatternOptions {
  defaultSelected: number;
  patterns: Array<Pattern>
  action: (index: number) => void;
}

export interface Pattern {
  icon: Resource;
  selectedIcon: Resource;
}

@Extend(Divider)
function divider() {
  .height('1px')
  .color('#10000000')
  .margin({ left: 12, right: 12 })
}

@Extend(Text)
function textStyle() {
  .width('100%')
  .fontWeight(400)
  .maxLines(1)
}

const TAG: string = 'InputMethodListDialog';
const BIG_IMAGE_SIZE: number = 30;
const NORMAL_IMAGE_SIZE: number = 24;
const SCREEN_SIZE: number = 1920;
const BIG_SCREEN_DPI: number = 1280;
const NORMAL_SCREEN_DPI: number = 360;
const BIG_DIALOG_WIDTH: number = 196;
const NORMAL_DIALOG_WIDTH: number = 156;
const BIG_FONT_SIZE: number = 20;
const NORMAL_FONT_SIZE: number = 16;
const BIG_ITEM_HEIGHT: number = 60;
const NORMAL_ITEM_HEIGHT: number = 48;
const NORMAL_IMAGE_BUTTON_WIDTH: number = 40;
const NORMAL_IMAGE_BUTTON_HEIGHT: number = 32;
const BIG_IMAGE_BUTTON_WIDTH: number = 50;
const BIG_IMAGE_BUTTON_HEIGHT: number = 40;
const NORMAL_COLUMN_PADDING: number = 4;
const BIG_COLUMN_PADDING: number = 5;
const NORMAL_IMAGE_RADIUS: number = 8;
const BIG_IMAGE_RADIUS: number = 10;
const NORMAL_FONT_PADDING: number = 12;
const BIG_FONT_PADDING: number = 20;
const NORMAL_ITEM_RADIUS: number = 16;
const BIG_ITEM_RADIUS: number = 12;

@CustomDialog
export struct InputMethodListDialog {
  private borderBgColor: ResourceColor = '#00ffffff';
  private listBgColor: ResourceColor = '#ffffff';
  private pressedColor: ResourceColor = '#1A000000'
  private selectedColor: ResourceColor = '#220A59F7';
  private fontColor: ResourceColor = '#E6000000';
  private selectedFontColor: ResourceColor = '#0A59F7';
  @State listItemHeight: number = NORMAL_ITEM_HEIGHT;
  @State listItemRadius: number = NORMAL_IMAGE_RADIUS;
  @State inputMethods: Array<inputMethod.InputMethodProperty> = [];
  @State fontSize: number = NORMAL_FONT_SIZE;
  @State fontPadding: number = NORMAL_FONT_PADDING;
  @State dialogWidth: number = NORMAL_DIALOG_WIDTH;
  @State imageSize: number = NORMAL_IMAGE_SIZE;
  @State imageBtnWidth: number = NORMAL_IMAGE_BUTTON_WIDTH;
  @State imageBtnHeight: number = NORMAL_IMAGE_BUTTON_HEIGHT;
  @State columnPadding: number = NORMAL_COLUMN_PADDING;
  @State imageRadius: number = NORMAL_IMAGE_RADIUS;
  @State subTypes: Array<InputMethodSubtype> = [];
  @State showHand: boolean = false;
  @State inputMethodConfig: bundleManager.ElementName | undefined = undefined;
  @State maxHeight: number = 1;
  @State defaultInputMethod: inputMethod.InputMethodProperty | undefined = undefined;
  @State currentInputMethod: inputMethod.InputMethodProperty | undefined = undefined;
  @State currentSub: InputMethodSubtype | undefined = undefined;
  @State viewOpacity: number = 0;
  @StorageLink('patternMode') patternMode: number | undefined = 0;
  controller: CustomDialogController = new CustomDialogController({ builder: undefined });
  patternOptions?: PatternOptions;

  async getDefaultInputMethodSubType() {
    console.info(`${TAG} getDefaultInputMethodSubType`);
    this.inputMethodConfig = inputMethod.getSystemInputMethodConfigAbility();
    if (this.inputMethodConfig) {
      console.info(`${TAG} inputMethodConfig:  ${JSON.stringify(this.inputMethodConfig)}`);
    }
    this.inputMethods = await inputMethod.getSetting().getInputMethods(true);
    this.defaultInputMethod = inputMethod.getDefaultInputMethod();
    this.currentInputMethod = inputMethod.getCurrentInputMethod();
    let index = 0;
    for (let k = 0;k < this.inputMethods.length; k++) {
      if (this.inputMethods[k].name === this.defaultInputMethod.name) {
        index = k;
        break;
      }
    }
    this.inputMethods.splice(index, 1);
    this.inputMethods.unshift(this.defaultInputMethod);
    this.currentSub = inputMethod.getCurrentInputMethodSubtype();
    console.info(`${TAG} defaultInput: ${JSON.stringify(this.defaultInputMethod)}`);
    if (this.defaultInputMethod.name === this.currentInputMethod.name) {
      if (this.patternOptions) {
        if (AppStorage.get<number>('patternMode') === undefined) {
          if (this.patternOptions.defaultSelected) {
            this.patternMode = this.patternOptions.defaultSelected;
          } else {
            this.patternMode = 0;
          }
          AppStorage.setOrCreate('patternMode', this.patternMode);
        } else {
          this.patternMode = AppStorage.get<number>('patternMode');
        }
        this.showHand = true;
      }
    }
    let listSubTypes = await inputMethod.getSetting().listInputMethodSubtype(this.defaultInputMethod);
    console.info(`${TAG} defaultSubTypes: ${JSON.stringify(listSubTypes)}`);
    let context = getContext(this) as common.UIAbilityContext;
    try {
      let activeSubType = await settings.getValue(context, settings.input.ACTIVATED_INPUT_METHOD_SUB_MODE);
      if (activeSubType) {
        console.info(`${TAG} activeSubType: ${activeSubType}`);
        for (let i = 0;i < this.inputMethods.length; i++) {
          if (this.inputMethods[i].name === this.defaultInputMethod.name) {
            this.defaultInputMethod = this.inputMethods[i];
            let defaultSubTypes = await inputMethod.getSetting().listInputMethodSubtype(this.inputMethods[i]);
            for (let k = 0;k < defaultSubTypes.length; k++) {
              if (activeSubType.indexOf(defaultSubTypes[k].id) !== -1) {
                this.subTypes.push(defaultSubTypes[k]);
              }
            }
          }
        }
      }
    } catch (err) {
      this.subTypes = [];
      console.info(`${TAG} subTypes is empty, err = ${JSON.stringify(err)}`);
    }
    let size = this.subTypes.length == 0 ? this.inputMethods.length : this.subTypes.length + this.inputMethods.length - 1
    size += (this.inputMethodConfig && this.inputMethodConfig.bundleName.length > 0) ? 1 : 0;
    size += this.patternOptions && this.showHand ? 1 : 0;
    let height = size * this.listItemHeight + this.columnPadding * 2;
    console.info(`${TAG} height: ${height}`);
    if (this.maxHeight > height) {
      this.maxHeight = height;
    }
    this.viewOpacity = 1;
  }

  aboutToAppear() {
    console.info(`${TAG} aboutToAppear`);
    let defaultDisplay = display.getDefaultDisplaySync();
    if (defaultDisplay.width > SCREEN_SIZE) {
      this.dialogWidth = px2vp(defaultDisplay.width) * BIG_DIALOG_WIDTH / BIG_SCREEN_DPI;
      this.fontSize = px2vp(defaultDisplay.width) * BIG_FONT_SIZE / BIG_SCREEN_DPI;
      this.imageSize = px2vp(defaultDisplay.width) * BIG_IMAGE_SIZE / BIG_SCREEN_DPI;
      this.listItemHeight = px2vp(defaultDisplay.width) * BIG_ITEM_HEIGHT / NORMAL_SCREEN_DPI;
      this.imageBtnWidth = px2vp(defaultDisplay.width) * BIG_IMAGE_BUTTON_WIDTH / BIG_SCREEN_DPI;
      this.imageBtnHeight = px2vp(defaultDisplay.width) * BIG_IMAGE_BUTTON_HEIGHT / BIG_SCREEN_DPI;
      this.columnPadding = px2vp(defaultDisplay.width) * BIG_COLUMN_PADDING / BIG_SCREEN_DPI;
      this.fontPadding = px2vp(defaultDisplay.width) * BIG_FONT_PADDING / BIG_SCREEN_DPI;
      this.listItemRadius = px2vp(defaultDisplay.width) * BIG_ITEM_RADIUS / BIG_SCREEN_DPI;
      this.imageRadius = BIG_IMAGE_RADIUS;

    } else {
      this.dialogWidth = px2vp(defaultDisplay.width) * NORMAL_DIALOG_WIDTH / NORMAL_SCREEN_DPI;
      this.fontSize = px2vp(defaultDisplay.width) * NORMAL_FONT_SIZE / NORMAL_SCREEN_DPI;
      this.imageSize = px2vp(defaultDisplay.width) * NORMAL_IMAGE_SIZE / NORMAL_SCREEN_DPI;
      this.listItemHeight = px2vp(defaultDisplay.width) * NORMAL_ITEM_HEIGHT / NORMAL_SCREEN_DPI;
      this.imageBtnWidth = px2vp(defaultDisplay.width) * NORMAL_IMAGE_BUTTON_WIDTH / NORMAL_SCREEN_DPI;
      this.imageBtnHeight = px2vp(defaultDisplay.width) * NORMAL_IMAGE_BUTTON_HEIGHT / NORMAL_SCREEN_DPI;
      this.columnPadding = px2vp(defaultDisplay.width) * NORMAL_COLUMN_PADDING / NORMAL_SCREEN_DPI;
      this.fontPadding = px2vp(defaultDisplay.width) * NORMAL_FONT_PADDING / NORMAL_SCREEN_DPI;
      this.listItemRadius = px2vp(defaultDisplay.width) * NORMAL_ITEM_RADIUS / NORMAL_SCREEN_DPI;
      this.imageRadius = NORMAL_IMAGE_RADIUS;
    }
    this.getDefaultInputMethodSubType();
    let inputMethodAbility = inputMethodEngine.getInputMethodAbility();
    inputMethodAbility.on('keyboardHide', () => {
      this.controller.close();
    });
  }

  @Styles
  listItemStyle(){
    .padding({ left: this.fontPadding, right: this.fontPadding })
    .height(this.listItemHeight)
    .borderRadius(this.listItemRadius)
  }

  build() {
    Stack({ alignContent: Alignment.BottomStart }) {
      Column() {
        if (this.inputMethodConfig && this.inputMethodConfig.bundleName.length > 0) {
          Text($r('sys.string.ohos_id_input_method_settings'))
            .textStyle()
            .listItemStyle()
            .fontSize(this.fontSize)
            .fontColor(this.fontColor)
            .stateStyles({
              pressed: {
                .backgroundColor(this.pressedColor)
              },
              normal: {
                .backgroundColor(this.listBgColor)
              }
            })
            .onClick(() => {
              if (this.inputMethodConfig) {
                let context = getContext(this) as common.UIAbilityContext;
                context.startAbility({
                  bundleName: this.inputMethodConfig.bundleName,
                  moduleName: this.inputMethodConfig.moduleName,
                  abilityName: this.inputMethodConfig.abilityName
                });
              }
            })
          Divider()
            .divider()
        }
        Scroll() {
          Column() {
            ForEach(this.subTypes, (item: InputMethodSubtype, index: number) => {
              Column() {
                Text(item.label)
                  .fontSize(this.fontSize)
                  .textStyle()
                  .listItemStyle()
                  .fontColor((this.currentSub && this.currentSub.id === item.id && this.currentSub.name === item.name)
                    ? this.selectedFontColor : this.fontColor)
                  .stateStyles({
                    pressed: {
                      .backgroundColor(this.pressedColor)
                    },
                    normal: {
                      .backgroundColor(
                        (this.currentSub && this.currentSub.id === item.id && this.currentSub.name === item.name)
                          ? this.selectedColor : this.listBgColor)
                    }
                  })
                if (this.inputMethods.length > 1 || index < this.subTypes.length) {
                  Divider()
                    .divider()
                }
              }
              .width('100%')
              .onClick(() => {
                this.switchMethodSub(item);
              })
            }, (item: inputMethod.InputMethodProperty) => JSON.stringify(item));

            ForEach(this.inputMethods, (item: inputMethod.InputMethodProperty, index: number) => {
              if (this.subTypes.length === 0 || (this.defaultInputMethod && item.name !== this.defaultInputMethod.name)) {
                Text(item.label)
                  .fontSize(this.fontSize)
                  .textStyle()
                  .listItemStyle()
                  .fontColor((this.currentSub && this.currentSub.id === item.id && this.currentSub.name === item.name)
                    ? this.selectedFontColor : this.fontColor)
                  .stateStyles({
                    pressed: {
                      .backgroundColor(this.pressedColor)
                    },
                    normal: {
                      .backgroundColor((this.currentInputMethod && this.currentInputMethod.name === item.name)
                        ? this.selectedColor : this.listBgColor)
                    }
                  })
                  .onClick(() => {
                    this.switchMethod(item);
                  })
                if (index < this.inputMethods.length - 1) {
                  Divider()
                    .divider()
                }
              }
            }, (item: inputMethod.InputMethodProperty) => JSON.stringify(item));
          }
          .width('100%')
        }
        .width('100%')
        .scrollBar(BarState.Off)
        .layoutWeight(1)

        if (this.patternOptions && this.showHand) {
          Divider()
            .divider()
          Row() {
            ForEach(this.patternOptions.patterns, (item: Pattern, index: number) => {
              Row() {
                Image(index === this.patternMode ? item.selectedIcon : item.icon)
                  .size({ width: this.imageSize, height: this.imageSize })
                  .objectFit(ImageFit.Contain)
              }
              .justifyContent(FlexAlign.Center)
              .size({ width: this.imageBtnWidth, height: this.imageBtnHeight })
              .borderRadius(this.imageRadius)
              .stateStyles({
                pressed: {
                  .backgroundColor(this.pressedColor)
                },
                normal: {
                  .backgroundColor(this.listBgColor)
                }
              })
              .onClick(() => {
                this.switchPositionPattern(index);
              })
            }, (item: Resource) => JSON.stringify(item));
          }
          .width('100%')
          .height(this.listItemHeight)
          .justifyContent(FlexAlign.SpaceEvenly)
        }
      }
      .width(this.dialogWidth)
      .margin({ top: this.columnPadding })
      .borderRadius('16vp')
      .backgroundColor(this.listBgColor)
      .height(this.maxHeight)
      .padding(this.columnPadding)
      .shadow(ShadowStyle.OUTER_DEFAULT_SM)
    }
    .id('inputDialog')
    .height('100%')
    .width('100%')
    .opacity(this.viewOpacity)
    .backgroundColor(this.borderBgColor)
    .transition(TransitionEffect.translate({ y: 400 })
      .combine(TransitionEffect.scale({ x: 1, y: 0 }))
      .animation({ duration: 500 }))
    .onAppear(() => {
      let component = componentUtils.getRectangleById('inputDialog');
      this.maxHeight = px2vp(component.size.height + component.windowOffset.y) - 10;
    })
    .onClick(() => {
      this.controller.close();
    })
  }

  switchPositionPattern(mode: number) {
    if (this.patternOptions) {
      this.patternMode = mode;
      AppStorage.set('patternMode', this.patternMode);
      console.info(`${TAG} this.handMode = ${this.patternMode}`);
      this.patternOptions.action(this.patternMode);
      this.controller.close();
    }
  }

  async switchMethod(inputProperty: inputMethod.InputMethodProperty) {
    if (this.currentInputMethod && this.currentInputMethod.name !== inputProperty.name) {
      let subTypes = await inputMethod.getSetting().listInputMethodSubtype(inputProperty);
      inputMethod.switchCurrentInputMethodAndSubtype(inputProperty, subTypes[0], (err: Error, result: boolean) => {
        if (result) {
          this.currentInputMethod = inputProperty;
        }
        this.controller.close();
      })
    }
  }

  switchMethodSub(inputSub: InputMethodSubtype) {
    if (this.currentInputMethod && this.defaultInputMethod) {
      if (this.currentInputMethod.name !== this.defaultInputMethod.name) {
        inputMethod.switchCurrentInputMethodAndSubtype(this.defaultInputMethod, inputSub, () => {
          this.currentInputMethod = this.defaultInputMethod;
          this.currentSub = inputSub;
          this.controller.close();
        })
      } else {
        inputMethod.switchCurrentInputMethodSubtype(inputSub, () => {
          this.currentSub = inputSub;
          this.controller.close();
        })
      }
    }
  }
}
