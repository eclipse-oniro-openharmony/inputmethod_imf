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

#include <mutex>
#include <thread>

#ifndef INPUTMETHOD_IMF_INPUT_METHOD_PROPERTY_H
#define INPUTMETHOD_IMF_INPUT_METHOD_PROPERTY_H

namespace OHOS {
namespace MiscServices {
struct Property {
    std::string name;
    std::string id;
    std::string label;
    std::string icon;
    int32_t iconId = 0;
};

struct SubProperty {
    std::string label;
    std::string name;
    std::string id;
    std::string mode;
    std::string locale;
    std::string language;
    std::string icon;
    int32_t iconId = 0;
};
} // namespace MiscServices
} // namespace OHOS

#endif //INPUTMETHOD_IMF_INPUT_METHOD_PROPERTY_H
