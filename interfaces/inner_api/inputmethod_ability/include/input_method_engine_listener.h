/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_IMF_INPUT_METHOD_ENGINE_LISTENER_H
#define INPUTMETHOD_IMF_INPUT_METHOD_ENGINE_LISTENER_H

#include "input_method_property.h"
#include "input_method_utils.h"

namespace OHOS {
namespace MiscServices {
class InputMethodEngineListener {
public:
    virtual ~InputMethodEngineListener() = default;
    virtual void OnKeyboardStatus(bool isShow) = 0;
    virtual void OnInputStart() = 0;
    virtual void OnInputStop() = 0;
    virtual void OnSecurityChange(int32_t security) = 0;
    virtual void OnSetCallingWindow(uint32_t windowId) = 0;
    virtual void OnSetSubtype(const SubProperty &property) = 0;
    virtual void ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand) = 0;
    virtual void OnInputFinish()
    {
    }
    virtual bool IsEnable()
    {
        return false;
    }
};
} // namespace MiscServices
} // namespace OHOS

#endif // INPUTMETHOD_IMF_INPUT_METHOD_ENGINE_LISTENER_H
