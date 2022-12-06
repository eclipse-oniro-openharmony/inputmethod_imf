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

#include "perusersession_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_ex.h>

#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_core.h"
#include "input_client_proxy.h"
#include "input_method_ability.h"
#include "input_method_agent_proxy.h"
#include "input_method_agent_stub.h"
#include "input_method_core_proxy.h"
#include "input_method_info.h"
#include "input_method_property.h"
#include "iremote_broker.h"
#include "message_parcel.h"
#include "peruser_session.h"

using namespace OHOS::MiscServices;
namespace OHOS {
constexpr size_t THRESHOLD = 10;

uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}

bool FuzzPerUserSession(const uint8_t *rawData, size_t size)
{
    Property property;
    SubProperty subProperty;

    int flags = static_cast<int32_t>(*rawData);
    std::string str(rawData, rawData + size);
    bool isShowKeyboard = true;
    constexpr int32_t MAIN_USER_ID = 100;
    sptr<IInputClient> client = new (std::nothrow) InputClientStub();
    sptr<IRemoteObject> object = client->AsObject();
    std::shared_ptr<PerUserSession> userSessions = std::make_shared<PerUserSession>(MAIN_USER_ID);
    sptr<IInputMethodCore> core = new InputMethodCoreProxy(object);
    sptr<IInputMethodAgent> agent = new InputMethodAgentProxy(object);
    InputMethodInfo *ime = new InputMethodInfo();

    userSessions->OnShowKeyboardSelf();
    userSessions->OnInputMethodSwitched(property, subProperty);
    userSessions->GetCurrentSubProperty();
    userSessions->SetCurrentSubProperty(subProperty);
    userSessions->StopInputService(str);
    userSessions->JoinWorkThread();
    userSessions->OnHideKeyboardSelf(flags);
    userSessions->OnStartInput(client, isShowKeyboard);
    userSessions->OnStopInput(client);
    userSessions->OnReleaseInput(client);
    userSessions->OnSetCoreAndAgent(core, agent);

    delete ime;
    ime = nullptr;
    return true;
}
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }
    /* Run your code on data */
    OHOS::FuzzPerUserSession(data, size);
    return 0;
}