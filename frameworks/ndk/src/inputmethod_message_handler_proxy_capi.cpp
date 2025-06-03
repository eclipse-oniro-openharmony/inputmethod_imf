/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "global.h"
#include "native_inputmethod_types.h"
using namespace OHOS::MiscServices;
InputMethod_MessageHandlerProxy *OH_MessageHandlerProxy_Create(void)
{
    return new (std::nothrow) InputMethod_MessageHandlerProxy();
}

void OH_MessageHandlerProxy_Destroy(InputMethod_MessageHandlerProxy *proxy)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return;
    }
    delete proxy;
}

InputMethod_ErrorCode OH_MessageHandlerProxy_SetOnTerminatedFunc(
    InputMethod_MessageHandlerProxy *proxy, OH_MessageHandlerProxy_OnTerminatedFunc onTerminatedFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (onTerminatedFunc == nullptr) {
        IMSA_HILOGE("onTerminatedFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->onTerminatedFunc = onTerminatedFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_MessageHandlerProxy_GetOnTerminatedFunc(
    InputMethod_MessageHandlerProxy *proxy, OH_MessageHandlerProxy_OnTerminatedFunc *onTerminatedFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (onTerminatedFunc == nullptr) {
        IMSA_HILOGE("onTerminatedFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *onTerminatedFunc = proxy->onTerminatedFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_MessageHandlerProxy_SetOnMessageFunc(
    InputMethod_MessageHandlerProxy *proxy, OH_MessageHandlerProxy_OnMessageFunc onMessageFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    if (onMessageFunc == nullptr) {
        IMSA_HILOGE("onMessageFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }

    proxy->onMessageFunc = onMessageFunc;
    return IME_ERR_OK;
}

InputMethod_ErrorCode OH_MessageHandlerProxy_GetOnMessageFunc(
    InputMethod_MessageHandlerProxy *proxy, OH_MessageHandlerProxy_OnMessageFunc *onMessageFunc)
{
    if (proxy == nullptr) {
        IMSA_HILOGE("proxy is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    if (onMessageFunc == nullptr) {
        IMSA_HILOGE("onMessageFunc is nullptr");
        return IME_ERR_NULL_POINTER;
    }
    *onMessageFunc = proxy->onMessageFunc;
    return IME_ERR_OK;
}