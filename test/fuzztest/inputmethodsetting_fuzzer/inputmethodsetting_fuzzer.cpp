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

#include "inputmethodsetting_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "peruser_session.h"
#include "peruser_setting.h"
#include "input_method_system_ability.h"
#include "input_method_setting.h"
#include "input_method_controller.h"
#include "global.h"

#include "message_parcel.h"

using namespace OHOS::MiscServices;
namespace OHOS {
    class TextListener : public OnTextChangedListener {
    public:
        TextListener() {}
        ~TextListener() {}
        void InsertText(const std::u16string& text) {}
        void DeleteBackward(int32_t length) {}
        void SetKeyboardStatus(bool status) {}
        void DeleteForward(int32_t length) {}
        void SendKeyEventFromInputMethod(const KeyEvent& event) {}
        void SendKeyboardInfo(const KeyboardInfo& status) {}
        void MoveCursor(const Direction direction) {}
    };
    bool FuzzInputMethodSetting(const uint8_t* rawData, size_t size)
    {
        sptr<InputMethodController> imc = InputMethodController::GetInstance();
        sptr<OnTextChangedListener> textListener = new TextListener();
        imc->Attach(textListener);

        constexpr int32_t MAIN_USER_ID = 100;
        PerUserSetting *setting = new PerUserSetting(MAIN_USER_ID);
        InputMethodSetting *methodSetting = setting->GetInputMethodSetting();

        InputMethodSetting setting_ = *methodSetting;
        std::u16string imeId = Str8ToStr16(std::string(rawData, rawData + size));
        std::vector<int32_t> types;
        for (size_t i = 0; i < size; ++i) {
            types.push_back(static_cast<int32_t>(*rawData));
        }
        setting_.GetCurrentInputMethod();
        setting_.SetCurrentInputMethod(imeId);
        setting_.GetEnabledInputMethodList();
        setting_.AddEnabledInputMethod(imeId, types);
        setting_.RemoveEnabledInputMethod(imeId);
        setting_.GetEnabledKeyboardTypes(imeId);
        setting_.GetCurrentKeyboardType();
        setting_.SetCurrentKeyboardType(static_cast<int32_t>(*rawData));
        setting_.GetCurrentSysKeyboardType();
        setting_.SetCurrentSysKeyboardType(static_cast<int32_t>(*rawData));
        setting_.FindKey(imeId);
        setting_.ClearData();

        delete setting;
        setting = nullptr;
        return true;
    }
}
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzInputMethodSetting(data, size);
    return 0;
}