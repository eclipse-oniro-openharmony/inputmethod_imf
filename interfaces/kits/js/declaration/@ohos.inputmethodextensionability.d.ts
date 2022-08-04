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

import Want from './@ohos.application.Want';

import InputMethodExtensionContext from "./@ohos.inputmethodextensioncontext";

/**
 * The extension ability class of input method.
 *
 * @since 9
 * @syscap SystemCapability.MiscServices.InputMethodFramework
 * @StageModelOnly
 */
export default class InputMethodExtensionAbility {
    /**
     * Indicates input method extension ability context.
     *
     * @since 9
     * @syscap SystemCapability.MiscServices.InputMethodFramework
     * @StageModelOnly
     */
    context: InputMethodExtensionContext;

    /**
     * Called back when a input method extension is started for initialization.
     *
     * @since 9
     * @syscap SystemCapability.MiscServices.InputMethodFramework
     * @param want Indicates the want of created service extension.
     * @return -
     * @StageModelOnly
     */
    onCreate(want: Want): void;

    /**
     * Called back before a input method extension is destroyed.
     *
     * @since 9
     * @syscap SystemCapability.MiscServices.InputMethodFramework
     * @return -
     * @StageModelOnly
     */
    onDestroy(): void;

}