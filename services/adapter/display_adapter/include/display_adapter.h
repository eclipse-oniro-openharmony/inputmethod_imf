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

#ifndef INPUTMETHOD_IMF_DISPLAY_ADAPTER_H
#define INPUTMETHOD_IMF_DISPLAY_ADAPTER_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace MiscServices {
class DisplayAdapter final {
public:
    static std::string GetDisplayName(uint64_t displayId);
    static uint64_t GetDefaultDisplayId();
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_DISPLAY_ADAPTER_H