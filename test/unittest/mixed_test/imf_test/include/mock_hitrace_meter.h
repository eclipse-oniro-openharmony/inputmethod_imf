/*
* Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef INPUTMETHOD_IMF_MOCK_HITRACE_METER_H
#define INPUTMETHOD_IMF_MOCK_HITRACE_METER_H

#include "inputmethod_trace.h"
#include "mock_hitrace_meter.h"  // 假设我们有一个Mock类来模拟hitrace_meter中的函数

namespace OHOS {
namespace MiscServices {
constexpr uint64_t HITRACE_TAG_MISC = (1ULL << 41); // Notification module tag.

MockHiTraceMeter* g_mockHiTraceMeter = nullptr;

void InitHiTrace()
{
    if (g_mockHiTraceMeter) {
        g_mockHiTraceMeter->UpdateTraceLabel();
    } else {
        UpdateTraceLabel();
    }
}

void ValueTrace(const std::string &name, int64_t count)
{
    if (g_mockHiTraceMeter) {
        g_mockHiTraceMeter->CountTrace(HITRACE_TAG_MISC, name, count);
    } else {
        CountTrace(HITRACE_TAG_MISC, name, count);
    }
}

void StartAsync(const std::string &value, int32_t taskId)
{
    if (g_mockHiTraceMeter) {
        g_mockHiTraceMeter->StartAsyncTrace(HITRACE_TAG_MISC, value, taskId);
    } else {
        StartAsyncTrace(HITRACE_TAG_MISC, value, taskId);
    }
}

void FinishAsync(const std::string &value, int32_t taskId)
{
    if (g_mockHiTraceMeter) {
        g_mockHiTraceMeter->FinishAsyncTrace(HITRACE_TAG_MISC, value, taskId);
    } else {
        FinishAsyncTrace(HITRACE_TAG_MISC, value, taskId);
    }
}

InputMethodSyncTrace::InputMethodSyncTrace(const std::string &value)
{
    if (g_mockHiTraceMeter) {
        g_mockHiTraceMeter->StartTrace(HITRACE_TAG_MISC, value);
    } else {
        StartTrace(HITRACE_TAG_MISC, value);
    }
}

InputMethodSyncTrace::InputMethodSyncTrace(const std::string &value, const std::string &id)
{
    auto info = value + "_" + id;
    if (g_mockHiTraceMeter) {
        g_mockHiTraceMeter->StartTrace(HITRACE_TAG_MISC, info);
    } else {
        StartTrace(HITRACE_TAG_MISC, info);
    }
}

InputMethodSyncTrace::~InputMethodSyncTrace()
{
    if (g_mockHiTraceMeter) {
        g_mockHiTraceMeter->FinishTrace(HITRACE_TAG_MISC);
    } else {
        FinishTrace(HITRACE_TAG_MISC);
    }
}

void SetMockHiTraceMeter(MockHiTraceMeter* mockHiTraceMeter)
{
    g_mockHiTraceMeter = mockHiTraceMeter;
}
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_IMF_MOCK_HITRACE_METER_H
