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

let componentUtils = requireNapi('arkui.componentUtils');
let display = requireNapi('display');
let inputMethod = requireNapi('inputMethod');
let inputMethodEngine = requireNapi('inputMethodEngine');
let settings = requireNapi('settings');

function __Divider__divider() {
  Divider.height('1px');
  Divider.color('#10000000');
  Divider.margin({ left: 12, right: 12 });
}

function __Text__textStyle() {
  Text.width('100%');
  Text.fontWeight(400);
  Text.maxLines(1);
}

const TAG = 'InputMethodListDialog';
const BIG_IMAGE_SIZE = 30;
const NORMAL_IMAGE_SIZE = 24;
const SCREEN_SIZE = 1920;
const BIG_SCREEN_DPI = 1280;
const NORMAL_SCREEN_DPI = 360;
const BIG_DIALOG_WIDTH = 196;
const NORMAL_DIALOG_WIDTH = 156;
const BIG_FONT_SIZE = 20;
const NORMAL_FONT_SIZE = 16;
const BIG_ITEM_HEIGHT = 60;
const NORMAL_ITEM_HEIGHT = 48;
const NORMAL_IMAGE_BUTTON_WIDTH = 40;
const NORMAL_IMAGE_BUTTON_HEIGHT = 32;
const BIG_IMAGE_BUTTON_WIDTH = 50;
const BIG_IMAGE_BUTTON_HEIGHT = 40;
const NORMAL_COLUMN_PADDING = 4;
const BIG_COLUMN_PADDING = 5;
const NORMAL_IMAGE_RADIUS = 8;
const BIG_IMAGE_RADIUS = 10;
const NORMAL_FONT_PADDING = 12;
const BIG_FONT_PADDING = 20;
const NORMAL_ITEM_RADIUS = 16;
const BIG_ITEM_RADIUS = 12;

export class InputMethodListDialog extends ViewPU {
  constructor(t, e, i, o = -1, n = void 0) {
    super(t, i, o);
    'function' === typeof n && (this.paramsGenerator_ = n);
    this.borderBgColor = '#00ffffff';
    this.listBgColor = '#ffffff';
    this.pressedColor = '#1A000000';
    this.selectedColor = '#220A59F7';
    this.fontColor = '#E6000000';
    this.selectedFontColor = '#0A59F7';
    this.__listItemHeight = new ObservedPropertySimplePU(48, this, 'listItemHeight');
    this.__listItemRadius = new ObservedPropertySimplePU(8, this, 'listItemRadius');
    this.__inputMethods = new ObservedPropertyObjectPU([], this, 'inputMethods');
    this.__fontSize = new ObservedPropertySimplePU(16, this, 'fontSize');
    this.__fontPadding = new ObservedPropertySimplePU(12, this, 'fontPadding');
    this.__dialogWidth = new ObservedPropertySimplePU(156, this, 'dialogWidth');
    this.__imageSize = new ObservedPropertySimplePU(24, this, 'imageSize');
    this.__imageBtnWidth = new ObservedPropertySimplePU(40, this, 'imageBtnWidth');
    this.__imageBtnHeight = new ObservedPropertySimplePU(32, this, 'imageBtnHeight');
    this.__columnPadding = new ObservedPropertySimplePU(4, this, 'columnPadding');
    this.__imageRadius = new ObservedPropertySimplePU(8, this, 'imageRadius');
    this.__subTypes = new ObservedPropertyObjectPU([], this, 'subTypes');
    this.__showHand = new ObservedPropertySimplePU(!1, this, 'showHand');
    this.__inputMethodConfig = new ObservedPropertyObjectPU(void 0, this, 'inputMethodConfig');
    this.__maxHeight = new ObservedPropertySimplePU(1, this, 'maxHeight');
    this.__defaultInputMethod = new ObservedPropertyObjectPU(void 0, this, 'defaultInputMethod');
    this.__currentInputMethod = new ObservedPropertyObjectPU(void 0, this, 'currentInputMethod');
    this.__currentSub = new ObservedPropertyObjectPU(void 0, this, 'currentSub');
    this.__viewOpacity = new ObservedPropertySimplePU(0, this, 'viewOpacity');
    this.__patternMode = this.createStorageLink('patternMode', 0, 'patternMode');
    this.controller = new CustomDialogController({ builder: void 0 }, this);
    this.patternOptions = void 0;
    this.setInitiallyProvidedValue(e);
  }

  setInitiallyProvidedValue(t) {
    void 0 !== t.borderBgColor && (this.borderBgColor = t.borderBgColor);
    void 0 !== t.listBgColor && (this.listBgColor = t.listBgColor);
    void 0 !== t.pressedColor && (this.pressedColor = t.pressedColor);
    void 0 !== t.selectedColor && (this.selectedColor = t.selectedColor);
    void 0 !== t.fontColor && (this.fontColor = t.fontColor);
    void 0 !== t.selectedFontColor && (this.selectedFontColor = t.selectedFontColor);
    void 0 !== t.listItemHeight && (this.listItemHeight = t.listItemHeight);
    void 0 !== t.listItemRadius && (this.listItemRadius = t.listItemRadius);
    void 0 !== t.inputMethods && (this.inputMethods = t.inputMethods);
    void 0 !== t.fontSize && (this.fontSize = t.fontSize);
    void 0 !== t.fontPadding && (this.fontPadding = t.fontPadding);
    void 0 !== t.dialogWidth && (this.dialogWidth = t.dialogWidth);
    void 0 !== t.imageSize && (this.imageSize = t.imageSize);
    void 0 !== t.imageBtnWidth && (this.imageBtnWidth = t.imageBtnWidth);
    void 0 !== t.imageBtnHeight && (this.imageBtnHeight = t.imageBtnHeight);
    void 0 !== t.columnPadding && (this.columnPadding = t.columnPadding);
    void 0 !== t.imageRadius && (this.imageRadius = t.imageRadius);
    void 0 !== t.subTypes && (this.subTypes = t.subTypes);
    void 0 !== t.showHand && (this.showHand = t.showHand);
    void 0 !== t.inputMethodConfig && (this.inputMethodConfig = t.inputMethodConfig);
    void 0 !== t.maxHeight && (this.maxHeight = t.maxHeight);
    void 0 !== t.defaultInputMethod && (this.defaultInputMethod = t.defaultInputMethod);
    void 0 !== t.currentInputMethod && (this.currentInputMethod = t.currentInputMethod);
    void 0 !== t.currentSub && (this.currentSub = t.currentSub);
    void 0 !== t.viewOpacity && (this.viewOpacity = t.viewOpacity);
    void 0 !== t.controller && (this.controller = t.controller);
    void 0 !== t.patternOptions && (this.patternOptions = t.patternOptions);
  }

  updateStateVars(t) {
  }

  purgeVariableDependenciesOnElmtId(t) {
    this.__listItemHeight.purgeDependencyOnElmtId(t);
    this.__listItemRadius.purgeDependencyOnElmtId(t);
    this.__inputMethods.purgeDependencyOnElmtId(t);
    this.__fontSize.purgeDependencyOnElmtId(t);
    this.__fontPadding.purgeDependencyOnElmtId(t);
    this.__dialogWidth.purgeDependencyOnElmtId(t);
    this.__imageSize.purgeDependencyOnElmtId(t);
    this.__imageBtnWidth.purgeDependencyOnElmtId(t);
    this.__imageBtnHeight.purgeDependencyOnElmtId(t);
    this.__columnPadding.purgeDependencyOnElmtId(t);
    this.__imageRadius.purgeDependencyOnElmtId(t);
    this.__subTypes.purgeDependencyOnElmtId(t);
    this.__showHand.purgeDependencyOnElmtId(t);
    this.__inputMethodConfig.purgeDependencyOnElmtId(t);
    this.__maxHeight.purgeDependencyOnElmtId(t);
    this.__defaultInputMethod.purgeDependencyOnElmtId(t);
    this.__currentInputMethod.purgeDependencyOnElmtId(t);
    this.__currentSub.purgeDependencyOnElmtId(t);
    this.__viewOpacity.purgeDependencyOnElmtId(t);
    this.__patternMode.purgeDependencyOnElmtId(t);
  }

  aboutToBeDeleted() {
    this.__listItemHeight.aboutToBeDeleted();
    this.__listItemRadius.aboutToBeDeleted();
    this.__inputMethods.aboutToBeDeleted();
    this.__fontSize.aboutToBeDeleted();
    this.__fontPadding.aboutToBeDeleted();
    this.__dialogWidth.aboutToBeDeleted();
    this.__imageSize.aboutToBeDeleted();
    this.__imageBtnWidth.aboutToBeDeleted();
    this.__imageBtnHeight.aboutToBeDeleted();
    this.__columnPadding.aboutToBeDeleted();
    this.__imageRadius.aboutToBeDeleted();
    this.__subTypes.aboutToBeDeleted();
    this.__showHand.aboutToBeDeleted();
    this.__inputMethodConfig.aboutToBeDeleted();
    this.__maxHeight.aboutToBeDeleted();
    this.__defaultInputMethod.aboutToBeDeleted();
    this.__currentInputMethod.aboutToBeDeleted();
    this.__currentSub.aboutToBeDeleted();
    this.__viewOpacity.aboutToBeDeleted();
    this.__patternMode.aboutToBeDeleted();
    SubscriberManager.Get().delete(this.id__());
    this.aboutToBeDeletedInternal();
  }

  get listItemHeight() {
    return this.__listItemHeight.get();
  }

  set listItemHeight(t) {
    this.__listItemHeight.set(t);
  }

  get listItemRadius() {
    return this.__listItemRadius.get();
  }

  set listItemRadius(t) {
    this.__listItemRadius.set(t);
  }

  get inputMethods() {
    return this.__inputMethods.get();
  }

  set inputMethods(t) {
    this.__inputMethods.set(t);
  }

  get fontSize() {
    return this.__fontSize.get();
  }

  set fontSize(t) {
    this.__fontSize.set(t);
  }

  get fontPadding() {
    return this.__fontPadding.get();
  }

  set fontPadding(t) {
    this.__fontPadding.set(t);
  }

  get dialogWidth() {
    return this.__dialogWidth.get();
  }

  set dialogWidth(t) {
    this.__dialogWidth.set(t);
  }

  get imageSize() {
    return this.__imageSize.get();
  }

  set imageSize(t) {
    this.__imageSize.set(t);
  }

  get imageBtnWidth() {
    return this.__imageBtnWidth.get();
  }

  set imageBtnWidth(t) {
    this.__imageBtnWidth.set(t);
  }

  get imageBtnHeight() {
    return this.__imageBtnHeight.get();
  }

  set imageBtnHeight(t) {
    this.__imageBtnHeight.set(t);
  }

  get columnPadding() {
    return this.__columnPadding.get();
  }

  set columnPadding(t) {
    this.__columnPadding.set(t);
  }

  get imageRadius() {
    return this.__imageRadius.get();
  }

  set imageRadius(t) {
    this.__imageRadius.set(t);
  }

  get subTypes() {
    return this.__subTypes.get();
  }

  set subTypes(t) {
    this.__subTypes.set(t);
  }

  get showHand() {
    return this.__showHand.get();
  }

  set showHand(t) {
    this.__showHand.set(t);
  }

  get inputMethodConfig() {
    return this.__inputMethodConfig.get();
  }

  set inputMethodConfig(t) {
    this.__inputMethodConfig.set(t);
  }

  get maxHeight() {
    return this.__maxHeight.get();
  }

  set maxHeight(t) {
    this.__maxHeight.set(t);
  }

  get defaultInputMethod() {
    return this.__defaultInputMethod.get();
  }

  set defaultInputMethod(t) {
    this.__defaultInputMethod.set(t);
  }

  get currentInputMethod() {
    return this.__currentInputMethod.get();
  }

  set currentInputMethod(t) {
    this.__currentInputMethod.set(t);
  }

  get currentSub() {
    return this.__currentSub.get();
  }

  set currentSub(t) {
    this.__currentSub.set(t);
  }

  get viewOpacity() {
    return this.__viewOpacity.get();
  }

  set viewOpacity(t) {
    this.__viewOpacity.set(t);
  }

  get patternMode() {
    return this.__patternMode.get();
  }

  set patternMode(t) {
    this.__patternMode.set(t);
  }

  setController(t) {
    this.controller = t;
  }

  async getDefaultInputMethodSubType() {
    console.info(`${TAG} getDefaultInputMethodSubType`);
    this.inputMethodConfig = inputMethod.getSystemInputMethodConfigAbility();
    this.inputMethodConfig && console.info(`${TAG} inputMethodConfig:  ${JSON.stringify(this.inputMethodConfig)}`);
    this.inputMethods = await inputMethod.getSetting().getInputMethods(!0);
    this.defaultInputMethod = inputMethod.getDefaultInputMethod();
    this.currentInputMethod = inputMethod.getCurrentInputMethod();
    let t = 0;
    for (let e = 0; e < this.inputMethods.length; e++) {
      if (this.inputMethods[e].name === this.defaultInputMethod.name) {
        t = e;
        break;
      }
    }
    this.inputMethods.splice(t, 1);
    this.inputMethods.unshift(this.defaultInputMethod);
    this.currentSub = inputMethod.getCurrentInputMethodSubtype();
    console.info(`${TAG} defaultInput: ${JSON.stringify(this.defaultInputMethod)}`);
    if (this.defaultInputMethod.name === this.currentInputMethod.name && this.patternOptions) {
      if (void 0 === AppStorage.get('patternMode')) {
        this.patternOptions.defaultSelected ? this.patternMode = this.patternOptions.defaultSelected :
          this.patternMode = 0;
        AppStorage.setOrCreate('patternMode', this.patternMode);
      } else {
        this.patternMode = AppStorage.get('patternMode');
      }
      this.showHand = !0;
    }
    let e = await inputMethod.getSetting().listInputMethodSubtype(this.defaultInputMethod);
    console.info(`${TAG} defaultSubTypes: ${JSON.stringify(e)}`);
    let i = getContext(this);
    try {
      let t = await settings.getValue(i, settings.input.ACTIVATED_INPUT_METHOD_SUB_MODE);
      if (t) {
        console.info(`${TAG} activeSubType: ${t}`);
        for (let e = 0; e < this.inputMethods.length; e++) {
          if (this.inputMethods[e].name === this.defaultInputMethod.name) {
            this.defaultInputMethod = this.inputMethods[e];
            let i = await inputMethod.getSetting().listInputMethodSubtype(this.inputMethods[e]);
            for (let e = 0; e < i.length; e++) {
              -1 !== t.indexOf(i[e].id) && this.subTypes.push(i[e]);
            }
          }
        }
      }
    } catch (t) {
      this.subTypes = [];
      console.info(`${TAG} subTypes is empty, err = ${JSON.stringify(t)}`);
    }
    let o = 0 === this.subTypes.length ? this.inputMethods.length : this.subTypes.length + this.inputMethods.length - 1;
    o += this.inputMethodConfig && this.inputMethodConfig.bundleName.length > 0 ? 1 : 0;
    o += this.patternOptions && this.showHand ? 1 : 0;
    let n = o * this.listItemHeight + 2 * this.columnPadding;
    console.info(`${TAG} height: ${n}`);
    this.maxHeight > n && (this.maxHeight = n);
    this.viewOpacity = 1;
  }

  aboutToAppear() {
    console.info(`${TAG} aboutToAppear`);
    let t = display.getDefaultDisplaySync();
    if (t.width > 1920) {
      this.dialogWidth = 196 * px2vp(t.width) / 1280;
      this.fontSize = 20 * px2vp(t.width) / 1280;
      this.imageSize = 30 * px2vp(t.width) / 1280;
      this.listItemHeight = 60 * px2vp(t.width) / 360;
      this.imageBtnWidth = 50 * px2vp(t.width) / 1280;
      this.imageBtnHeight = 40 * px2vp(t.width) / 1280;
      this.columnPadding = 5 * px2vp(t.width) / 1280;
      this.fontPadding = 20 * px2vp(t.width) / 1280;
      this.listItemRadius = 12 * px2vp(t.width) / 1280;
      this.imageRadius = 10;
    } else {
      this.dialogWidth = 156 * px2vp(t.width) / 360;
      this.fontSize = 16 * px2vp(t.width) / 360;
      this.imageSize = 24 * px2vp(t.width) / 360;
      this.listItemHeight = 48 * px2vp(t.width) / 360;
      this.imageBtnWidth = 40 * px2vp(t.width) / 360;
      this.imageBtnHeight = 32 * px2vp(t.width) / 360;
      this.columnPadding = 4 * px2vp(t.width) / 360;
      this.fontPadding = 12 * px2vp(t.width) / 360;
      this.listItemRadius = 16 * px2vp(t.width) / 360;
      this.imageRadius = 8;
    }
    this.getDefaultInputMethodSubType();
    inputMethodEngine.getInputMethodAbility().on('keyboardHide', (() => {
      this.controller.close();
    }));
  }

  initialRender() {
    this.observeComponentCreation2(((t, e) => {
      Stack.create({ alignContent: Alignment.BottomStart });
      Stack.id('inputDialog');
      Stack.height('100%');
      Stack.width('100%');
      Stack.opacity(this.viewOpacity);
      Stack.backgroundColor(this.borderBgColor);
      Stack.transition(TransitionEffect.translate({ y: 400 }).combine(TransitionEffect.scale({
        x: 1,
        y: 0
      })).animation({ duration: 500 }));
      Stack.onAppear((() => {
        let t = componentUtils.getRectangleById('inputDialog');
        this.maxHeight = px2vp(t.size.height + t.windowOffset.y) - 10;
      }));
      Stack.onClick((() => {
        this.controller.close();
      }))
    }), Stack);
    this.observeComponentCreation2(((t, e) => {
      Column.create();
      Column.width(this.dialogWidth);
      Column.margin({ top: this.columnPadding });
      Column.borderRadius('16vp');
      Column.backgroundColor(this.listBgColor);
      Column.height(this.maxHeight);
      Column.padding(this.columnPadding);
      Column.shadow(ShadowStyle.OUTER_DEFAULT_SM);
    }), Column);
    this.observeComponentCreation2(((t, e) => {
      If.create();
      this.inputMethodConfig && this.inputMethodConfig.bundleName.length > 0 ? this.ifElseBranchUpdateFunction(0, (() => {
        this.observeComponentCreation2(((t, e) => {
          Text.create({
            id: -1,
            type: 10003,
            params: ['sys.string.ohos_id_input_method_settings'],
            bundleName: '',
            moduleName: ''
          });
          __Text__textStyle();
          Text.padding({ left: this.fontPadding, right: this.fontPadding });
          Text.height(this.listItemHeight);
          Text.borderRadius(this.listItemRadius);
          Text.fontSize(this.fontSize);
          Text.fontColor(this.fontColor);
          ViewStackProcessor.visualState('pressed');
          Text.backgroundColor(this.pressedColor);
          ViewStackProcessor.visualState('normal');
          Text.backgroundColor(this.listBgColor);
          ViewStackProcessor.visualState();
          Text.onClick((() => {
            if (this.inputMethodConfig) {
              getContext(this).startAbility({
                bundleName: this.inputMethodConfig.bundleName,
                moduleName: this.inputMethodConfig.moduleName,
                abilityName: this.inputMethodConfig.abilityName
              });
            }
          }));
        }), Text);
        Text.pop();
        this.observeComponentCreation2(((t, e) => {
          Divider.create();
          __Divider__divider();
        }), Divider);
      })) : this.ifElseBranchUpdateFunction(1, (() => {
      }));
    }), If);
    If.pop();
    this.observeComponentCreation2(((t, e) => {
      Scroll.create();
      Scroll.width('100%');
      Scroll.scrollBar(BarState.Off);
      Scroll.layoutWeight(1);
    }), Scroll);
    this.observeComponentCreation2(((t, e) => {
      Column.create();
      Column.width('100%');
    }), Column);
    this.observeComponentCreation2(((t, e) => {
      ForEach.create();
      this.forEachUpdateFunction(t, this.subTypes, ((t, e) => {
        const i = t;
        this.observeComponentCreation2(((t, e) => {
          Column.create();
          Column.width('100%');
          Column.onClick((() => {
            this.switchMethodSub(i);
          }));
        }), Column);
        this.observeComponentCreation2(((t, e) => {
          Text.create(i.label);
          Text.fontSize(this.fontSize);
          __Text__textStyle();
          Text.padding({ left: this.fontPadding, right: this.fontPadding });
          Text.height(this.listItemHeight);
          Text.borderRadius(this.listItemRadius);
          Text.fontColor(this.currentSub && this.currentSub.id === i.id && this.currentSub.name === i.name ?
          this.selectedFontColor : this.fontColor);
          ViewStackProcessor.visualState('pressed');
          Text.backgroundColor(this.pressedColor);
          ViewStackProcessor.visualState('normal');
          Text.backgroundColor(this.currentSub && this.currentSub.id === i.id && this.currentSub.name === i.name ?
          this.selectedColor : this.listBgColor);
          ViewStackProcessor.visualState();
        }), Text);
        Text.pop();
        this.observeComponentCreation2(((t, i) => {
          If.create();
          this.inputMethods.length > 1 || e < this.subTypes.length ? this.ifElseBranchUpdateFunction(0, (() => {
            this.observeComponentCreation2(((t, e) => {
              Divider.create();
              __Divider__divider();
            }), Divider);
          })) : this.ifElseBranchUpdateFunction(1, (() => {
          }));
        }), If);
        If.pop();
        Column.pop();
      }), (t => JSON.stringify(t)),!0,!1);
    }), ForEach);
    ForEach.pop();
    this.observeComponentCreation2(((t, e) => {
      ForEach.create();
      this.forEachUpdateFunction(t, this.inputMethods, ((t, e) => {
        const i = t;
        this.observeComponentCreation2(((t, o) => {
          If.create();
          0 === this.subTypes.length || this.defaultInputMethod && i.name !== this.defaultInputMethod.name ?
          this.ifElseBranchUpdateFunction(0, (() => {
            this.observeComponentCreation2(((t, e) => {
              Text.create(i.label);
              Text.fontSize(this.fontSize);
              __Text__textStyle();
              Text.padding({ left: this.fontPadding, right: this.fontPadding });
              Text.height(this.listItemHeight);
              Text.borderRadius(this.listItemRadius);
              Text.fontColor(this.currentSub && this.currentSub.id === i.id && this.currentSub.name === i.name ?
              this.selectedFontColor : this.fontColor);
              ViewStackProcessor.visualState('pressed');
              Text.backgroundColor(this.pressedColor);
              ViewStackProcessor.visualState('normal');
              Text.backgroundColor(this.currentInputMethod && this.currentInputMethod.name === i.name ?
              this.selectedColor : this.listBgColor);
              ViewStackProcessor.visualState();
              Text.onClick((() => {
                this.switchMethod(i);
              }))
            }), Text);
            Text.pop();
            this.observeComponentCreation2(((t, i) => {
              If.create();
              e < this.inputMethods.length - 1 ? this.ifElseBranchUpdateFunction(0, (() => {
                this.observeComponentCreation2(((t, e) => {
                  Divider.create();
                  __Divider__divider();
                }), Divider);
              })) : this.ifElseBranchUpdateFunction(1, (() => {
              }))
            }), If);
            If.pop();
          })) : this.ifElseBranchUpdateFunction(1, (() => {
          }))
        }), If);
        If.pop();
      }), (t => JSON.stringify(t)),!0,!1);
    }), ForEach);
    ForEach.pop();
    Column.pop();
    Scroll.pop();
    this.observeComponentCreation2(((t, e) => {
      If.create();
      this.patternOptions && this.showHand ? this.ifElseBranchUpdateFunction(0, (() => {
        this.observeComponentCreation2(((t, e) => {
          Divider.create();
          __Divider__divider();
        }), Divider);
        this.observeComponentCreation2(((t, e) => {
          Row.create();
          Row.width('100%');
          Row.height(this.listItemHeight);;
          Row.justifyContent(FlexAlign.SpaceEvenly);
        }), Row);
        this.observeComponentCreation2(((t, e) => {
          ForEach.create();
          this.forEachUpdateFunction(t, this.patternOptions.patterns, ((t, e) => {
            const i = t;
            this.observeComponentCreation2(((t, i) => {
              Row.create();
              Row.justifyContent(FlexAlign.Center);
              Row.size({ width: this.imageBtnWidth, height: this.imageBtnHeight });
              Row.borderRadius(this.imageRadius);
              ViewStackProcessor.visualState('pressed');
              Row.backgroundColor(this.pressedColor);
              ViewStackProcessor.visualState('normal');
              Row.backgroundColor(this.listBgColor);
              ViewStackProcessor.visualState();
              Row.onClick((() => {
                this.switchPositionPattern(e);
              }))
            }), Row);
            this.observeComponentCreation2(((t, o) => {
              Image.create(e === this.patternMode ? i.selectedIcon : i.icon);
              Image.size({ width: this.imageSize, height: this.imageSize });
              Image.objectFit(ImageFit.Contain);
            }), Image);
            Row.pop();
          }), (t => JSON.stringify(t)),!0,!1);
        }), ForEach);
        ForEach.pop();
        Row.pop();
      })) : this.ifElseBranchUpdateFunction(1, (() => {
      }));
    }), If);
    If.pop();
    Column.pop();
    Stack.pop();
  }

  switchPositionPattern(t) {
    if (this.patternOptions) {
      this.patternMode = t;
      AppStorage.set('patternMode', this.patternMode);
      console.info(`${TAG} this.handMode = ${this.patternMode}`);
      this.patternOptions.action(this.patternMode);
      this.controller.close();
    }
  }

  async switchMethod(t) {
    if (this.currentInputMethod && this.currentInputMethod.name !== t.name) {
      let e = await inputMethod.getSetting().listInputMethodSubtype(t);
      inputMethod.switchCurrentInputMethodAndSubtype(t, e[0], ((e, i) => {
        i && (this.currentInputMethod = t);
        this.controller.close();
      }));
    }
  }

  switchMethodSub(t) {
    this.currentInputMethod && this.defaultInputMethod &&
      (this.currentInputMethod.name !== this.defaultInputMethod.name ?
      inputMethod.switchCurrentInputMethodAndSubtype(this.defaultInputMethod, t, (() => {
        this.currentInputMethod = this.defaultInputMethod;
        this.currentSub = t;
        this.controller.close();
      })) : inputMethod.switchCurrentInputMethodSubtype(t, (() => {
        this.currentSub = t;
        this.controller.close();
      })));
  }

  rerender() {
    this.updateDirtyElements();
  }
}

export default {
  InputMethodListDialog
};