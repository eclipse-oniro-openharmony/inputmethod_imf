/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SETTING_LISTENERS_H
#define SETTING_LISTENERS_H

#include "input_method_controller.h"
#include "event_handler.h"
#include "input_method_ffi_structs.h"
#include "global.h"

namespace OHOS::MiscServices {
    class CJGetInputMethodSetting : public ImeEventListener {
    public:
        CJGetInputMethodSetting() = default;
        ~CJGetInputMethodSetting() = default;
        static std::shared_ptr<CJGetInputMethodSetting> GetIMSInstance();
        int32_t Subscribe(uint32_t type, void (*func)(CInputMethodProperty, CInputMethodSubtype));
        int32_t UnSubscribe(uint32_t type);
        void OnImeChange(const Property &property, const SubProperty &subProperty) override;

    private:
        static std::mutex msMutex_;
        static std::shared_ptr<CJGetInputMethodSetting> inputMethod_;
        std::function<void(CInputMethodProperty, CInputMethodSubtype)> callback = nullptr;
    };
}

#endif // SETTING_LISTENERS_H