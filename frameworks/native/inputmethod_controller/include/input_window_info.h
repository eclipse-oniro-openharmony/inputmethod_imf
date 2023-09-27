/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_WINDOW_INFO_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_WINDOW_INFO_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace MiscServices {
enum class InputWindowStatus : uint32_t {
    SHOW,
    HIDE,
    NONE
};

struct InputWindowInfo {
    std::string name; // the name of inputWindow
    int32_t left;     // the abscissa of the upper-left vertex of inputWindow
    int32_t top;      // the ordinate of the upper-left vertex of inputWindow
    uint32_t width;   // the width of inputWindow
    uint32_t height;  // the height of inputWindow
};
} // namespace MiscServices
} // namespace OHOS

#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_WINDOW_INFO_H
