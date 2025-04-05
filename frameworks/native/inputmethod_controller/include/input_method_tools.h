/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef INPUT_METHOD_TOOLS_H
#define INPUT_METHOD_TOOLS_H

#include "input_method_utils.h"
#include "input_attribute.h"
#include "input_client_info.h"

namespace OHOS {
namespace MiscServices {

class InputMethodTools {
public:
    static InputMethodTools &GetInstance();
    ~InputMethodTools() = default;
    InputAttributeInner AttributeToInner(const InputAttribute &attribute);
    InputAttribute InnerToAttribute(const InputAttributeInner &inner);
    CursorInfoInner CursorInfoToInner(const CursorInfo &cursorInfo);
    CursorInfo InnerToCursorInfo(const CursorInfoInner &inner);
    RangeInner RangeToInner(const Range &range);
    Range InnerToRange(const RangeInner &inner);
    TextSelectionInner TextSelectionToInner(const TextSelection &textSelection);
    TextSelection InnerToSelection(const TextSelectionInner &inner);
    TextTotalConfigInner TextTotalConfigToInner(const TextTotalConfig &textTotalConfig);
    TextTotalConfig InnerToTextTotalConfig(const TextTotalConfigInner &inner);
    InputClientInfoInner InputClientInfoToInner(const InputClientInfo &inputClientInfo);
    InputClientInfo InnerToInputClientInfo(const InputClientInfoInner &inner);
private:
    InputMethodTools() = default;
};


} // namespace MiscServices
} // namespace OHOS

#endif // INPUT_METHOD_TOOLS_H