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

#define private public
#define protected public
#include "peruser_session.h"
#undef private

#include <string_ex.h>

#include <cstddef>
#include <cstdint>
#include <memory>

#include "global.h"
#include "i_input_method_agent.h"
#include "i_input_method_core.h"
#include "input_client_proxy.h"
#include "input_client_stub.h"
#include "input_method_ability.h"
#include "input_method_agent_proxy.h"
#include "input_method_agent_stub.h"
#include "input_method_core_proxy.h"
#include "input_method_core_stub.h"
#include "input_method_info.h"
#include "input_method_property.h"
#include "iremote_broker.h"
#include "message_parcel.h"
#include "unRegistered_type.h"

using namespace OHOS::MiscServices;
namespace OHOS {
constexpr size_t THRESHOLD = 10;
constexpr int32_t MAIN_USER_ID = 100;

uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}

bool InitializeClientInfo(InputClientInfo &clientInfo)
{
    sptr<IInputClient> clientStub = new (std::nothrow) InputClientStub();
    if (clientStub == nullptr) {
        IMSA_HILOGE("failed to create client");
        return false;
    }
    sptr<InputDeathRecipient> deathRecipient = new (std::nothrow) InputDeathRecipient();
    if (deathRecipient == nullptr) {
        IMSA_HILOGE("failed to new deathRecipient");
        return false;
    }
    clientInfo = { .userID = MAIN_USER_ID, .client = clientStub, .deathRecipient = deathRecipient };
    return true;
}

bool FuzzPerUserSession(const uint8_t *rawData, size_t size)
{
    Property property;
    SubProperty subProperty;
    InputClientInfo clientInfo;
    if (!InitializeClientInfo(clientInfo)) {
        return false;
    }
    auto client = iface_cast<IInputClient>(clientInfo.client->AsObject());
    sptr<InputMethodCoreStub> coreStub = new (std::nothrow) InputMethodCoreStub();
    if (coreStub == nullptr) {
        return false;
    }
    auto core = iface_cast<IInputMethodCore>(coreStub->AsObject());
    sptr<InputMethodAgentStub> agentStub = new (std::nothrow) InputMethodAgentStub();
    if (agentStub == nullptr) {
        return false;
    }
    auto agent = iface_cast<IInputMethodAgent>(agentStub);
    static std::shared_ptr<PerUserSession> userSessions = std::make_shared<PerUserSession>(MAIN_USER_ID);

    userSessions->OnRegisterProxyIme(core, agent->AsObject());
    int32_t type = 4;
    userSessions->OnUnRegisteredProxyIme(static_cast<UnRegisteredType>(type), core);
    userSessions->IsProxyImeEnable();

    userSessions->OnPrepareInput(clientInfo);
    userSessions->OnSetCoreAndAgent(core, agent->AsObject());
    userSessions->OnShowCurrentInput();
    sptr<IRemoteObject> agentObject = nullptr;
    clientInfo.isShowKeyboard = false;
    userSessions->OnStartInput(clientInfo, agentObject);
    clientInfo.isShowKeyboard = true;
    userSessions->OnStartInput(clientInfo, agentObject);
    userSessions->NotifyImeChangeToClients(property, subProperty);
    userSessions->OnHideCurrentInput();
    userSessions->OnHideInput(client);
    userSessions->OnReleaseInput(client);
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