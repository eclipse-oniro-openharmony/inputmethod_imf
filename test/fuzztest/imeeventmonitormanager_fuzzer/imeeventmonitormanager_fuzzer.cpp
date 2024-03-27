/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "imeeventmonitormanager_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "global.h"
#include "input_attribute.h"
#include "input_method_controller.h"
#include "ime_event_monitor_manager.h"
#include "key_event.h"
#include "message_parcel.h"
#include "text_listener.h"


using namespace OHOS::MiscServices;
namespace OHOS {
constexpr size_t THRESHOLD = 10;
void FuzzRegisterImeEventListener(const uint8_t *rawData, size_t size)
{
    std::shared_ptr<JsGetInputMethodSetting> listener_ = new JsGetInputMethodSetting();
    std::set<EventType> eventType = {};
    ImeEventMonitorManager::GetInstance().RegisterImeEventListener({ eventType }, listener_);
}

void FuzzUnRegisterImeEventListener(const uint8_t *rawData, size_t size)
{
    std::shared_ptr<JsGetInputMethodSetting> listener_ = new JsGetInputMethodSetting();
    std::set<EventType> eventType = {};
    ImeEventMonitorManager::GetInstance().UnRegisterImeEventListener({ eventType }, listener_);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }

    /* Run your code on data */
    OHOS::FuzzRegisterImeEventListener(data, size);
    OHOS::FuzzUnRegisterImeEventListener(data, size);
    return 0;
}
