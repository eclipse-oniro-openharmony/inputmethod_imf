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

#include "cj_inputmethod_extension.h"
#include "ability_handler.h"
#include "configuration_utils.h"
#include "global.h"
#include "inputmethod_extension_ability_service_impl.h"
#include "inputmethod_trace.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "task_manager.h"
#include "tasks/task_ams.h"
#include "tasks/task_imsa.h"

namespace OHOS {
namespace AbilityRuntime {

CjInputMethodExtension *CjInputMethodExtension::cjInputMethodExtension = nullptr;
using namespace OHOS::AppExecFwk;
using namespace OHOS::MiscServices;

constexpr int32_t SUCCESS_CODE = 0;

extern "C" __attribute__((visibility("default"))) InputMethodExtension *OHOS_ABILITY_CjInputMethodExtension()
{
    return new (std::nothrow) CjInputMethodExtension();
}

CjInputMethodExtension::CjInputMethodExtension() { }

CjInputMethodExtension::~CjInputMethodExtension()
{
    IMSA_HILOGD("~CjInputMethodExtension");
    auto context = GetContext();
    if (context != nullptr) {
        context->Unbind();
    }
    cjObj_.Destroy();
}

void CjInputMethodExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    IMSA_HILOGD("initalize CjInputMethodExtension");
    InputMethodExtension::Init(record, application, handler, token);
    // init and bindContext
    int32_t ret = cjObj_.Init(abilityInfo_->name, this);
    if (ret != SUCCESS_CODE) {
        IMSA_HILOGE("initalization failed");
        return;
    }
    handler_ = handler;
    InitDisplayCache();
    ListenWindowManager();
    IMSA_HILOGI("initalize success");
}

void CjInputMethodExtension::ListenWindowManager()
{
    IMSA_HILOGD("register window manager service listener.");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        IMSA_HILOGE("failed to get SaMgr!");
        return;
    }

    auto cjInputMethodExtension = std::static_pointer_cast<CjInputMethodExtension>(shared_from_this());
    displayListener_ = sptr<CjInputMethodExtensionDisplayListener>::MakeSptr(cjInputMethodExtension);
    if (displayListener_ == nullptr) {
        IMSA_HILOGE("failed to create display listener!");
        return;
    }

    auto listener = sptr<SystemAbilityStatusChangeListener>::MakeSptr(displayListener_);
    if (listener == nullptr) {
        IMSA_HILOGE("failed to create status change listener!");
        return;
    }

    auto ret = abilityManager->SubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, listener);
    if (ret != 0) {
        IMSA_HILOGE("failed to subscribe system ability, ret: %{public}d!", ret);
    }
}

void CjInputMethodExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &config) { }

void CjInputMethodExtension::ConfigurationUpdated() { }

void CjInputMethodExtension::SystemAbilityStatusChangeListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string &deviceId)
{
    IMSA_HILOGD("add systemAbilityId: %{public}d.", systemAbilityId);
    if (systemAbilityId == WINDOW_MANAGER_SERVICE_ID) {
        Rosen::DisplayManager::GetInstance().RegisterDisplayListener(listener_);
    }
}

void CjInputMethodExtension::OnStart(const AAFwk::Want &want)
{
    auto task = std::make_shared<TaskAmsInit>();
    TaskManager::GetInstance().PostTask(task);
    InputMethodAbility::GetInstance().InitConnect();
    StartAsync("OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_EXTENSION));
    StartAsync("Extension::OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_MIDDLE_EXTENSION));
    Extension::OnStart(want);
    FinishAsync("Extension::OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_MIDDLE_EXTENSION));
    IMSA_HILOGI("CjInputMethodExtension OnStart begin.");
    StartAsync("onCreate", static_cast<int32_t>(TraceTaskId::ONCREATE_EXTENSION));
    cjObj_.OnCreate(want);
    FinishAsync("onCreate", static_cast<int32_t>(TraceTaskId::ONCREATE_EXTENSION));
    TaskManager::GetInstance().PostTask(std::make_shared<TaskImsaSetCoreAndAgent>());
    IMSA_HILOGI("ime bind imf");
    FinishAsync("OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_EXTENSION));
    TaskManager::GetInstance().Complete(task->GetSeqId());
}

void CjInputMethodExtension::InitDisplayCache()
{
    auto foldStatus = Rosen::DisplayManager::GetInstance().GetFoldStatus();
    auto displayPtr = Rosen::DisplayManager::GetInstance().GetDefaultDisplaySync();
    if (displayPtr == nullptr) {
        IMSA_HILOGE("displayPtr is null");
        return;
    }
    cacheDisplay_.SetCacheDisplay(
        displayPtr->GetWidth(), displayPtr->GetHeight(), displayPtr->GetRotation(), foldStatus);
}

void CjInputMethodExtension::OnStop()
{
    InputMethodExtension::OnStop();
    IMSA_HILOGI("CjInputMethodExtension OnStop start.");
    cjObj_.OnDestroy();
    auto context = GetContext();
    if (context == nullptr) {
        IMSA_HILOGE("context is invalid!");
        return;
    }
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(context->GetToken());
    if (ret) {
        IMSA_HILOGI("the input method extension connection is not disconnected.");
    }
    IMSA_HILOGI("CjInputMethodExtension OnStop end.");
}

sptr<IRemoteObject> CjInputMethodExtension::OnConnect(const AAFwk::Want &want)
{
    IMSA_HILOGI("CjInputMethodExtension OnConnect start.");
    Extension::OnConnect(want);
    auto remoteObj = new (std::nothrow) InputMethodExtensionAbilityServiceImpl();
    if (remoteObj == nullptr) {
        IMSA_HILOGE("failed to create InputMethodExtensionAbilityServiceImpl!");
        return nullptr;
    }
    return remoteObj;
}

void CjInputMethodExtension::OnDisconnect(const AAFwk::Want &want) { }

void CjInputMethodExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId) { }

void CjInputMethodExtension::OnCreate(Rosen::DisplayId displayId)
{
    IMSA_HILOGD("enter");
}

void CjInputMethodExtension::OnDestroy(Rosen::DisplayId displayId)
{
    IMSA_HILOGD("exit");
}

void CjInputMethodExtension::CheckNeedAdjustKeyboard(Rosen::DisplayId displayId)
{
    auto displayPtr = Rosen::DisplayManager::GetInstance().GetDefaultDisplaySync();
    if (displayPtr == nullptr) {
        return;
    }
    auto defaultDisplayId = displayPtr->GetId();
    if (displayId != defaultDisplayId) {
        return;
    }
    auto foldStatus = Rosen::DisplayManager::GetInstance().GetFoldStatus();
    auto width = displayPtr->GetWidth();
    auto height = displayPtr->GetHeight();
    auto rotation = displayPtr->GetRotation();
    IMSA_HILOGD("display width: %{public}d, height: %{public}d, rotation: %{public}d, foldStatus: %{public}d",
        width, height, rotation, foldStatus);
    if (!cacheDisplay_.IsEmpty()) {
        if ((cacheDisplay_.displayWidth != width ||
                cacheDisplay_.displayHeight != height) &&
            cacheDisplay_.displayFoldStatus == foldStatus &&
            cacheDisplay_.displayRotation == rotation) {
            InputMethodAbility::GetInstance().AdjustKeyboard();
        }
    }
    cacheDisplay_.SetCacheDisplay(width, height, rotation, foldStatus);
}

void CjInputMethodExtension::OnChange(Rosen::DisplayId displayId)
{
    IMSA_HILOGD("displayId: %{public}" PRIu64 "", displayId);
    auto context = GetContext();
    if (context == nullptr) {
        IMSA_HILOGE("context is invalid!");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        IMSA_HILOGE("configuration is invalid!");
        return;
    }

    bool isConfigChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), isConfigChanged);
    IMSA_HILOGD("OnChange, isConfigChanged: %{public}d, Config after update: %{public}s.", isConfigChanged,
        contextConfig->GetName().c_str());

    if (!isConfigChanged) {
        return;
    }
    auto inputMethodExtension = std::static_pointer_cast<CjInputMethodExtension>(shared_from_this());
    auto task = [inputMethodExtension]() {
        if (inputMethodExtension) {
            inputMethodExtension->ConfigurationUpdated();
        }
    };
    if (handler_ != nullptr) {
        handler_->PostTask(task, "CjInputMethodExtension:OnChange", 0, AppExecFwk::EventQueue::Priority::VIP);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
