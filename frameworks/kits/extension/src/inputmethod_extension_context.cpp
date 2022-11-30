/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "inputmethod_extension_context.h"

#include "ability_connection.h"
#include "ability_manager_client.h"
#include "global.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t InputMethodExtensionContext::CONTEXT_TYPE_ID(std::hash<const char *>{}("InputMethodExtensionContext"));
int InputMethodExtensionContext::ILLEGAL_REQUEST_CODE(-1);

ErrCode InputMethodExtensionContext::StartAbility(const AAFwk::Want &want) const
{
    IMSA_HILOGD("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, ILLEGAL_REQUEST_CODE);
    IMSA_HILOGD("%{public}s ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        IMSA_HILOGE("InputMethodExtensionContext::StartAbility failed: %{public}d", err);
    }
    return err;
}

ErrCode InputMethodExtensionContext::StartAbility(
    const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const
{
    IMSA_HILOGD("%{public}s begin.", __func__);
    ErrCode err =
        AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_, ILLEGAL_REQUEST_CODE);
    IMSA_HILOGD("%{public}s ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        IMSA_HILOGE("InputMethodExtensionContext::StartAbility failed: %{public}d", err);
    }
    return err;
}

bool InputMethodExtensionContext::ConnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    IMSA_HILOGI("%{public}s begin.", __func__);
    ErrCode ret = ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    IMSA_HILOGI("InputMethodExtensionContext::ConnectAbility ret = %{public}d", ret);
    return ret == ERR_OK;
}

ErrCode InputMethodExtensionContext::StartAbilityWithAccount(const AAFwk::Want &want, int accountId) const
{
    IMSA_HILOGI("%{public}s begin, accountId: %{public}d", __func__, accountId);
    ErrCode err =
        AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, ILLEGAL_REQUEST_CODE, accountId);
    IMSA_HILOGD("%{public}s ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        IMSA_HILOGE("InputMethodExtensionContext::StartAbilityWithAccount failed: %{public}d", err);
    }
    return err;
}

ErrCode InputMethodExtensionContext::StartAbilityWithAccount(
    const AAFwk::Want &want, int accountId, const AAFwk::StartOptions &startOptions) const
{
    IMSA_HILOGD("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(
        want, startOptions, token_, ILLEGAL_REQUEST_CODE, accountId);
    IMSA_HILOGD("%{public}s ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        IMSA_HILOGE("InputMethodContext::StartAbilityWithAccount is failed %{public}d", err);
    }
    return err;
}

bool InputMethodExtensionContext::ConnectAbilityWithAccount(
    const AAFwk::Want &want, int accountId, const sptr<AbilityConnectCallback> &connectCallback) const
{
    IMSA_HILOGI("%{public}s begin.", __func__);
    ErrCode ret = ConnectionManager::GetInstance().ConnectAbilityWithAccount(token_, want, accountId, connectCallback);
    IMSA_HILOGI("InputMethodExtensionContext::ConnectAbilityWithAccount ret = %{public}d", ret);
    return ret == ERR_OK;
}

ErrCode InputMethodExtensionContext::DisconnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    IMSA_HILOGI("%{public}s begin.", __func__);
    ErrCode ret = ConnectionManager::GetInstance().DisconnectAbility(token_, want.GetElement(), connectCallback);
    if (ret != ERR_OK) {
        IMSA_HILOGE("%{public}s end DisconnectAbility error, ret=%{public}d", __func__, ret);
    }
    IMSA_HILOGI("%{public}s end DisconnectAbility", __func__);
    return ret;
}

ErrCode InputMethodExtensionContext::TerminateAbility()
{
    IMSA_HILOGI("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    if (err != ERR_OK) {
        IMSA_HILOGE("InputMethodExtensionContext::TerminateAbility failed: %{public}d", err);
    }
    IMSA_HILOGI("%{public}s end.", __func__);
    return err;
}

AppExecFwk::AbilityType InputMethodExtensionContext::GetAbilityInfoType() const
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        IMSA_HILOGE("InputMethodContext::GetAbilityInfoType info == nullptr");
        return AppExecFwk::AbilityType::UNKNOWN;
    }
    return info->type;
}
} // namespace AbilityRuntime
} // namespace OHOS