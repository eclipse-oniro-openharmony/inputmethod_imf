/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

let deviceInfo = requireNapi('deviceInfo');
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
  constructor(a, b, c, d = -1, e = undefined, f) {
    super(a, c, d, f);
    if (typeof e === "function") {
      this.paramsGenerator_ = e;
    }
    this.listBgColor = '#ffffff';
    this.pressedColor = '#1A000000';
    this.selectedColor = '#220A59F7';
    this.fontColor = '#E6000000';
    this.selectedFontColor = '#0A59F7';
    this.__listItemHeight = new ObservedPropertySimplePU(NORMAL_ITEM_HEIGHT, this, "listItemHeight");
    this.__listItemRadius = new ObservedPropertySimplePU(NORMAL_IMAGE_RADIUS, this, "listItemRadius");
    this.__inputMethods = new ObservedPropertyObjectPU([], this, "inputMethods");
    this.__fontSize = new ObservedPropertySimplePU(NORMAL_FONT_SIZE, this, "fontSize");
    this.__fontPadding = new ObservedPropertySimplePU(NORMAL_FONT_PADDING, this, "fontPadding");
    this.__dialogWidth = new ObservedPropertySimplePU(NORMAL_DIALOG_WIDTH, this, "dialogWidth");
    this.__imageSize = new ObservedPropertySimplePU(NORMAL_IMAGE_SIZE, this, "imageSize");
    this.__imageBtnWidth = new ObservedPropertySimplePU(NORMAL_IMAGE_BUTTON_WIDTH, this, "imageBtnWidth");
    this.__imageBtnHeight = new ObservedPropertySimplePU(NORMAL_IMAGE_BUTTON_HEIGHT, this, "imageBtnHeight");
    this.__columnPadding = new ObservedPropertySimplePU(NORMAL_COLUMN_PADDING, this, "columnPadding");
    this.__imageRadius = new ObservedPropertySimplePU(NORMAL_IMAGE_RADIUS, this, "imageRadius");
    this.__subTypes = new ObservedPropertyObjectPU([], this, "subTypes");
    this.__showHand = new ObservedPropertySimplePU(false, this, "showHand");
    this.__inputMethodConfig = new ObservedPropertyObjectPU(undefined, this, "inputMethodConfig");
    this.__defaultInputMethod = new ObservedPropertyObjectPU(undefined, this, "defaultInputMethod");
    this.__currentInputMethod = new ObservedPropertyObjectPU(undefined, this, "currentInputMethod");
    this.__currentSub = new ObservedPropertyObjectPU(undefined, this, "currentSub");
    this.__patternMode = this.createStorageLink('patternMode', 0, "patternMode");
    this.__maxListNum = this.createStorageLink('maxListNum', 0, "maxListNum");
    this.activeSubtypes = [];
    this.controller = new CustomDialogController({
      builder: undefined
    }, this);
    this.patternOptions = undefined;
    this.setInitiallyProvidedValue(b);
  }

  setInitiallyProvidedValue(g) {
    if (g.listBgColor !== undefined) {
      this.listBgColor = g.listBgColor;
    }
    if (g.pressedColor !== undefined) {
      this.pressedColor = g.pressedColor;
    }
    if (g.selectedColor !== undefined) {
      this.selectedColor = g.selectedColor;
    }
    if (g.fontColor !== undefined) {
      this.fontColor = g.fontColor;
    }
    if (g.selectedFontColor !== undefined) {
      this.selectedFontColor = g.selectedFontColor;
    }
    if (g.listItemHeight !== undefined) {
      this.listItemHeight = g.listItemHeight;
    }
    if (g.listItemRadius !== undefined) {
      this.listItemRadius = g.listItemRadius;
    }
    if (g.inputMethods !== undefined) {
      this.inputMethods = g.inputMethods;
    }
    if (g.fontSize !== undefined) {
      this.fontSize = g.fontSize;
    }
    if (g.fontPadding !== undefined) {
      this.fontPadding = g.fontPadding;
    }
    if (g.dialogWidth !== undefined) {
      this.dialogWidth = g.dialogWidth;
    }
    if (g.imageSize !== undefined) {
      this.imageSize = g.imageSize;
    }
    if (g.imageBtnWidth !== undefined) {
      this.imageBtnWidth = g.imageBtnWidth;
    }
    if (g.imageBtnHeight !== undefined) {
      this.imageBtnHeight = g.imageBtnHeight;
    }
    if (g.columnPadding !== undefined) {
      this.columnPadding = g.columnPadding;
    }
    if (g.imageRadius !== undefined) {
      this.imageRadius = g.imageRadius;
    }
    if (g.subTypes !== undefined) {
      this.subTypes = g.subTypes;
    }
    if (g.showHand !== undefined) {
      this.showHand = g.showHand;
    }
    if (g.inputMethodConfig !== undefined) {
      this.inputMethodConfig = g.inputMethodConfig;
    }
    if (g.defaultInputMethod !== undefined) {
      this.defaultInputMethod = g.defaultInputMethod;
    }
    if (g.currentInputMethod !== undefined) {
      this.currentInputMethod = g.currentInputMethod;
    }
    if (g.currentSub !== undefined) {
      this.currentSub = g.currentSub;
    }
    if (g.activeSubtypes !== undefined) {
      this.activeSubtypes = g.activeSubtypes;
    }
    if (g.controller !== undefined) {
      this.controller = g.controller;
    }
    if (g.patternOptions !== undefined) {
      this.patternOptions = g.patternOptions;
    }
  }

  updateStateVars(h) {
  }

  purgeVariableDependenciesOnElmtId(i) {
    this.__listItemHeight.purgeDependencyOnElmtId(i);
    this.__listItemRadius.purgeDependencyOnElmtId(i);
    this.__inputMethods.purgeDependencyOnElmtId(i);
    this.__fontSize.purgeDependencyOnElmtId(i);
    this.__fontPadding.purgeDependencyOnElmtId(i);
    this.__dialogWidth.purgeDependencyOnElmtId(i);
    this.__imageSize.purgeDependencyOnElmtId(i);
    this.__imageBtnWidth.purgeDependencyOnElmtId(i);
    this.__imageBtnHeight.purgeDependencyOnElmtId(i);
    this.__columnPadding.purgeDependencyOnElmtId(i);
    this.__imageRadius.purgeDependencyOnElmtId(i);
    this.__subTypes.purgeDependencyOnElmtId(i);
    this.__showHand.purgeDependencyOnElmtId(i);
    this.__inputMethodConfig.purgeDependencyOnElmtId(i);
    this.__defaultInputMethod.purgeDependencyOnElmtId(i);
    this.__currentInputMethod.purgeDependencyOnElmtId(i);
    this.__currentSub.purgeDependencyOnElmtId(i);
    this.__patternMode.purgeDependencyOnElmtId(i);
    this.__maxListNum.purgeDependencyOnElmtId(i);
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
    this.__defaultInputMethod.aboutToBeDeleted();
    this.__currentInputMethod.aboutToBeDeleted();
    this.__currentSub.aboutToBeDeleted();
    this.__patternMode.aboutToBeDeleted();
    this.__maxListNum.aboutToBeDeleted();
    SubscriberManager.Get().delete(this.id__());
    this.aboutToBeDeletedInternal();
  }

  get listItemHeight() {
    return this.__listItemHeight.get();
  }

  set listItemHeight(j) {
    this.__listItemHeight.set(j);
  }

  get listItemRadius() {
    return this.__listItemRadius.get();
  }

  set listItemRadius(k) {
    this.__listItemRadius.set(k);
  }

  get inputMethods() {
    return this.__inputMethods.get();
  }

  set inputMethods(l) {
    this.__inputMethods.set(l);
  }

  get fontSize() {
    return this.__fontSize.get();
  }

  set fontSize(m) {
    this.__fontSize.set(m);
  }

  get fontPadding() {
    return this.__fontPadding.get();
  }

  set fontPadding(n) {
    this.__fontPadding.set(n);
  }

  get dialogWidth() {
    return this.__dialogWidth.get();
  }

  set dialogWidth(o) {
    this.__dialogWidth.set(o);
  }

  get imageSize() {
    return this.__imageSize.get();
  }

  set imageSize(p) {
    this.__imageSize.set(p);
  }

  get imageBtnWidth() {
    return this.__imageBtnWidth.get();
  }

  set imageBtnWidth(q) {
    this.__imageBtnWidth.set(q);
  }

  get imageBtnHeight() {
    return this.__imageBtnHeight.get();
  }

  set imageBtnHeight(r) {
    this.__imageBtnHeight.set(r);
  }

  get columnPadding() {
    return this.__columnPadding.get();
  }

  set columnPadding(s) {
    this.__columnPadding.set(s);
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

  set subTypes(u) {
    this.__subTypes.set(u);
  }

  get showHand() {
    return this.__showHand.get();
  }

  set showHand(v) {
    this.__showHand.set(v);
  }

  get inputMethodConfig() {
    return this.__inputMethodConfig.get();
  }

  set inputMethodConfig(w) {
    this.__inputMethodConfig.set(w);
  }

  get defaultInputMethod() {
    return this.__defaultInputMethod.get();
  }

  set defaultInputMethod(x) {
    this.__defaultInputMethod.set(x);
  }

  get currentInputMethod() {
    return this.__currentInputMethod.get();
  }

  set currentInputMethod(y) {
    this.__currentInputMethod.set(y);
  }

  get currentSub() {
    return this.__currentSub.get();
  }

  set currentSub(z) {
    this.__currentSub.set(z);
  }

  get patternMode() {
    return this.__patternMode.get();
  }

  set patternMode(a1) {
    this.__patternMode.set(a1);
  }

  get maxListNum() {
    return this.__maxListNum.get();
  }

  set maxListNum(b1) {
    this.__maxListNum.set(b1);
  }

  setController(c1) {
    this.controller = c1;
  }

  async getDefaultInputMethodSubType() {
    console.info(`${TAG} getDefaultInputMethodSubType`);
    this.inputMethodConfig = inputMethod.getSystemInputMethodConfigAbility();
    if (this.inputMethodConfig) {
      console.info(`${TAG} inputMethodConfig:  ${JSON.stringify(this.inputMethodConfig)}`);
    }
    this.inputMethods = await inputMethod.getSetting().getInputMethods(true);
    this.defaultInputMethod = inputMethod.getDefaultInputMethod();
    this.currentInputMethod = inputMethod.getCurrentInputMethod();
    let d1 = 0;
    for (let f1 = 0; f1 < this.inputMethods.length; f1++) {
      if (this.inputMethods[f1].name === this.defaultInputMethod.name) {
        d1 = f1;
        break;
      }
    }
    this.inputMethods.splice(d1, 1);
    this.inputMethods.unshift(this.defaultInputMethod);
    this.currentSub = inputMethod.getCurrentInputMethodSubtype();
    console.info(`${TAG} defaultInput: ${JSON.stringify(this.defaultInputMethod)}`);
    if (this.defaultInputMethod.name === this.currentInputMethod.name) {
      if (this.patternOptions) {
        if (AppStorage.get('patternMode') === undefined) {
          if (this.patternOptions.defaultSelected) {
            this.patternMode = this.patternOptions.defaultSelected;
          } else {
            this.patternMode = 0;
          }
          AppStorage.setOrCreate('patternMode', this.patternMode);
        } else {
          this.patternMode = AppStorage.get('patternMode');
        }
        this.showHand = true;
      }
    }
    let e1 = getContext(this);
    try {
      let g1 = await settings.getValue(e1, settings.input.ACTIVATED_INPUT_METHOD_SUB_MODE);
      let h1 = JSON.parse(g1);
      if (h1) {
        console.info(`${TAG} activeSubType: ${JSON.stringify(h1)}`);
        for (let i1 = 0; i1 < this.inputMethods.length; i1++) {
          if (this.inputMethods[i1].name === this.defaultInputMethod.name) {
            this.defaultInputMethod = this.inputMethods[i1];
            let j1 = await inputMethod.getSetting().listInputMethodSubtype(this.inputMethods[i1]);
            console.info(`${TAG} defaultSubTypes: ${JSON.stringify(j1)}`);
            for (let k1 = 0; k1 < j1.length; k1++) {
              for (let l1 = 0; l1 < h1.length; l1++) {
                if (h1[l1].id === j1[k1].id) {
                  this.subTypes.push(j1[k1]);
                  this.activeSubtypes.push(h1[l1]);
                }
              }
            }
          }
        }
        console.info(`${TAG} this.subTypes: ${JSON.stringify(this.subTypes)}`);
        console.info(`${TAG} this.activeSubtypes: ${JSON.stringify(this.activeSubtypes)}`);
      }
    } catch (m1) {
      this.subTypes = [];
      console.info(`${TAG} subTypes is empty, err = ${JSON.stringify(m1)}`);
    }
  }

  async aboutToAppear() {
    console.info(`${TAG} aboutToAppear`);
    if (deviceInfo.deviceType === 'tablet') {
      this.dialogWidth = BIG_DIALOG_WIDTH;
      this.fontSize = BIG_FONT_SIZE;
      this.imageSize = BIG_IMAGE_SIZE;
      this.listItemHeight = BIG_ITEM_HEIGHT;
      this.imageBtnWidth = BIG_IMAGE_BUTTON_WIDTH;
      this.imageBtnHeight = BIG_IMAGE_BUTTON_HEIGHT;
      this.columnPadding = BIG_COLUMN_PADDING;
      this.fontPadding = BIG_FONT_PADDING;
      this.listItemRadius = BIG_ITEM_RADIUS;
    } else {
      this.dialogWidth = NORMAL_DIALOG_WIDTH;
      this.fontSize = NORMAL_FONT_SIZE;
      this.imageSize = NORMAL_IMAGE_SIZE;
      this.listItemHeight = NORMAL_ITEM_HEIGHT;
      this.imageBtnWidth = NORMAL_IMAGE_BUTTON_WIDTH;
      this.imageBtnHeight = NORMAL_IMAGE_BUTTON_HEIGHT;
      this.columnPadding = NORMAL_COLUMN_PADDING;
      this.fontPadding = NORMAL_FONT_PADDING;
      this.listItemRadius = NORMAL_ITEM_RADIUS;
    }
    this.imageRadius = BIG_IMAGE_RADIUS;
    await this.getDefaultInputMethodSubType();
    let n1 = inputMethodEngine.getInputMethodAbility();
    n1.on('keyboardHide', () => {
      this.controller.close();
    });
  }

  isDefaultInputMethodCurrentSubType(p1) {
    var q1, r1, s1;
    return ((q1 = this.defaultInputMethod) === null || q1 === void 0 ? void 0 : q1.name) ===
      ((r1 = this.currentInputMethod) === null || r1 === void 0 ? void 0 : r1.name) &&
      ((s1 = this.currentSub) === null || s1 === void 0 ? void 0 : s1.id) === p1;
  }

  InputMethodItem(t1, u1, v1, w1, x1, y1, z1 = null) {
    this.observeComponentCreation2((b2, c2) => {
      Column.create();
      Column.width('100%');
      Column.onClick(() => {
        y1();
      });
    }, Column);
    this.observeComponentCreation2((e2, f2) => {
      Text.create(t1);
      Text.fontSize(this.fontSize);
      __Text__textStyle();
      Text.padding({ left: this.fontPadding, right: this.fontPadding });
      Text.height(this.listItemHeight);
      Text.borderRadius(this.listItemRadius);
      Text.fontColor(u1);
      ViewStackProcessor.visualState("pressed");
      Text.backgroundColor(w1);
      ViewStackProcessor.visualState("normal");
      Text.backgroundColor(v1);
      ViewStackProcessor.visualState();
    }, Text);
    Text.pop();
    this.observeComponentCreation2((g2, h2) => {
      If.create();
      if (x1) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((k2, l2) => {
            Divider.create();
            __Divider__divider();
          }, Divider);
        });
      } else {
        this.ifElseBranchUpdateFunction(1, () => {
        });
      }
    }, If);
    If.pop();
    Column.pop();
  }

  initialRender() {
    this.observeComponentCreation2((o2, p2) => {
      Column.create();
      Column.width(this.dialogWidth);
      Column.margin({ top: this.columnPadding });
      Column.borderRadius('16vp');
      Column.backgroundColor(this.listBgColor);
      Column.padding(this.columnPadding);
      Column.shadow(ShadowStyle.OUTER_DEFAULT_SM);
    }, Column);
    this.observeComponentCreation2((q2, r2) => {
      If.create();
      if (this.inputMethodConfig && this.inputMethodConfig.bundleName.length > 0) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((u2, v2) => {
            Text.create({
              "id": -1,
              "type": 10003,
              params: ['sys.string.ohos_id_input_method_settings'],
              "bundleName": "",
              "moduleName": ""
            });
            __Text__textStyle();
            Text.padding({ left: this.fontPadding, right: this.fontPadding });
            Text.height(this.listItemHeight);
            Text.borderRadius(this.listItemRadius);
            Text.fontSize(this.fontSize);
            Text.fontColor(this.fontColor);
            ViewStackProcessor.visualState("pressed");
            Text.backgroundColor(this.pressedColor);
            ViewStackProcessor.visualState("normal");
            Text.backgroundColor(this.listBgColor);
            ViewStackProcessor.visualState();
            Text.onClick(() => {
              if (this.inputMethodConfig) {
                let x2 = getContext(this);
                x2.startAbility({
                  bundleName: this.inputMethodConfig.bundleName,
                  moduleName: this.inputMethodConfig.moduleName,
                  abilityName: this.inputMethodConfig.abilityName,
                  uri: 'set_input'
                });
              }
            });
          }, Text);
          Text.pop();
          this.observeComponentCreation2((y2, z2) => {
            Divider.create();
            __Divider__divider();
          }, Divider);
        });
      } else {
        this.ifElseBranchUpdateFunction(1, () => {
        });
      }
    }, If);
    If.pop();
    this.observeComponentCreation2((b3, c3) => {
      Scroll.create();
      Scroll.width('100%');
      Scroll.constraintSize({ maxHeight: this.maxListNum > 0 ? this.maxListNum * this.listItemHeight : '100%' });
      Scroll.scrollBar(BarState.Off);
    }, Scroll);
    this.observeComponentCreation2((d3, e3) => {
      Column.create();
      Column.width('100%');
    }, Column);
    this.observeComponentCreation2((f3, g3) => {
      ForEach.create();
      const h3 = (j3, k3) => {
        const l3 = j3;
        this.InputMethodItem.bind(this)(this.activeSubtypes[k3].name,
          this.isDefaultInputMethodCurrentSubType(l3.id) ? this.selectedFontColor : this.fontColor,
          this.isDefaultInputMethodCurrentSubType(l3.id) ? this.selectedColor : this.listBgColor,
          this.pressedColor,
          this.inputMethods.length > 1 || k3 < this.subTypes.length,
          () => {
            this.switchMethodSub(l3);
          });
      };
      this.forEachUpdateFunction(f3, this.subTypes, h3, (n3) => JSON.stringify(n3), true, false);
    }, ForEach);
    ForEach.pop();
    this.observeComponentCreation2((o3, p3) => {
      ForEach.create();
      const q3 = (s3, t3) => {
        const u3 = s3;
        this.observeComponentCreation2((w3, x3) => {
          If.create();
          if (this.subTypes.length === 0 || (this.defaultInputMethod && u3.name !== this.defaultInputMethod.name)) {
            this.ifElseBranchUpdateFunction(0, () => {
              var z3, a4;
              this.InputMethodItem.bind(this)(this.inputMethods[t3].label,
                ((z3 = this.currentInputMethod) === null || z3 === void 0 ? void 0 : z3.name) === u3.name ?
                this.selectedFontColor : this.fontColor,
                ((a4 = this.currentInputMethod) === null || a4 === void 0 ? void 0 : a4.name) === u3.name ?
                this.selectedColor : this.listBgColor,
                this.pressedColor,
                t3 < this.inputMethods.length - 1,
                () => {
                  this.switchMethod(u3);
                });
            });
          } else {
            this.ifElseBranchUpdateFunction(1, () => {
            });
          }
        }, If);
        If.pop();
      };
      this.forEachUpdateFunction(o3, this.inputMethods, q3, (d4) => JSON.stringify(d4), true, false);
    }, ForEach);
    ForEach.pop();
    Column.pop();
    Scroll.pop();
    this.observeComponentCreation2((e4, f4) => {
      If.create();
      if (this.patternOptions && this.showHand) {
        this.ifElseBranchUpdateFunction(0, () => {
          this.observeComponentCreation2((i4, j4) => {
            Divider.create();
            __Divider__divider();
          }, Divider);
          this.observeComponentCreation2((k4, l4) => {
            Row.create();
            Row.width('100%');
            Row.height(this.listItemHeight);
            Row.justifyContent(FlexAlign.SpaceEvenly);
          }, Row);
          this.observeComponentCreation2((m4, n4) => {
            ForEach.create();
            const o4 = (q4, r4) => {
              const s4 = q4;
              this.observeComponentCreation2((u4, v4) => {
                Row.create();
                Row.justifyContent(FlexAlign.Center);
                Row.size({ width: this.imageBtnWidth, height: this.imageBtnHeight });
                Row.borderRadius(this.imageRadius);
                ViewStackProcessor.visualState("pressed");
                Row.backgroundColor(this.pressedColor);
                ViewStackProcessor.visualState("normal");
                Row.backgroundColor(this.listBgColor);
                ViewStackProcessor.visualState();
                Row.onClick(() => {
                  this.switchPositionPattern(r4);
                });
              }, Row);
              this.observeComponentCreation2((x4, y4) => {
                Image.create(r4 === this.patternMode ? s4.selectedIcon : s4.icon);
                Image.size({ width: this.imageSize, height: this.imageSize });
                Image.objectFit(ImageFit.Contain);
              }, Image);
              Row.pop();
            };
            this.forEachUpdateFunction(m4, this.patternOptions.patterns, o4, (z4) => JSON.stringify(z4), true, false);
          }, ForEach);
          ForEach.pop();
          Row.pop();
        });
      } else {
        this.ifElseBranchUpdateFunction(1, () => {
        });
      }
    }, If);
    If.pop();
    Column.pop();
  }

  switchPositionPattern(b5) {
    if (this.patternOptions) {
      this.patternMode = b5;
      AppStorage.set('patternMode', this.patternMode);
      console.info(`${TAG} this.handMode = ${this.patternMode}`);
      this.patternOptions.action(this.patternMode);
      this.controller.close();
    }
  }

  async switchMethod(c5) {
    if (this.currentInputMethod && this.currentInputMethod.name !== c5.name) {
      let d5 = await inputMethod.getSetting().listInputMethodSubtype(c5);
      inputMethod.switchCurrentInputMethodAndSubtype(c5, d5[0], (f5, g5) => {
        if (g5) {
          this.currentInputMethod = c5;
        }
        this.controller.close();
      });
    }
  }

  switchMethodSub(h5) {
    if (this.currentInputMethod && this.defaultInputMethod) {
      if (this.currentInputMethod.name !== this.defaultInputMethod.name) {
        inputMethod.switchCurrentInputMethodAndSubtype(this.defaultInputMethod, h5, () => {
          this.currentInputMethod = this.defaultInputMethod;
          this.currentSub = h5;
          this.controller.close();
        });
      } else {
        inputMethod.switchCurrentInputMethodSubtype(h5, () => {
          this.currentSub = h5;
          this.controller.close();
        });
      }
    }
  }

  rerender() {
    this.updateDirtyElements();
  }
}

export default {
  InputMethodListDialog
};