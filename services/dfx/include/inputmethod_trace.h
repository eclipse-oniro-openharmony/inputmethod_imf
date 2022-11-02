/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_TRACE_H
#define INPUTMETHOD_TRACE_H

#include <string>

namespace OHOS {
namespace MiscServices {
constexpr uint64_t HITRACE_TAG_MISC = (1ULL << 41); // Notification module tag.

void InitHiTrace();
void ValueTrace(const std::string &name, int64_t count);

void StartAsync(uint64_t label, const std::string &value, int32_t taskId);
void FinishAsync(uint64_t label, const std::string &value, int32_t taskId);

class InputmethodTrace {
public:
    explicit InputmethodTrace(const std::string &value);
    virtual ~InputmethodTrace();
};

enum class TraceTaskId : int32_t {
    ONSTART_EXTENSION,
    ONSTART_MIDDLE_EXTENSION,
    ONCREATE_EXTENSION,
    ONCONNECT_EXTENSION,
    ONCONNECT_MIDDLE_EXTENSION,
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_TRACE_H