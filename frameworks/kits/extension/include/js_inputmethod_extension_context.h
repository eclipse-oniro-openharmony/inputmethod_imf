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

#ifndef ABILITY_RUNTIME_JS_INPUTMETHOD_EXTENSION_CONTEXT_H
#define ABILITY_RUNTIME_JS_INPUTMETHOD_EXTENSION_CONTEXT_H

#include <memory>

#include "ability_connect_callback.h"
#include "event_handler.h"
#include "inputmethod_extension_context.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsInputMethodExtensionContext(napi_env env, std::shared_ptr<InputMethodExtensionContext> context);

// need create in main thread.
class JSInputMethodExtensionConnection : public AbilityConnectCallback {
public:
    explicit JSInputMethodExtensionConnection(napi_env env);
    ~JSInputMethodExtensionConnection();
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject,
        int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    // this function need to execute in main thread.
    void CallJsFailed(int32_t errorCode);
    void SetJsConnectionObject(napi_value jsConnectionObject);

private:
    void HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject,
        int resultCode);
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);
    void ReleaseConnection();

private:
    napi_env env_;
    napi_ref jsConnectionObject_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
};

struct ConnectionKey {
    AAFwk::Want want;
    int64_t id;
};

struct key_compare {
    bool operator()(const ConnectionKey &key1, const ConnectionKey &key2) const
    {
        return key1.id < key2.id;
    }
};

static std::map<ConnectionKey, sptr<JSInputMethodExtensionConnection>, key_compare> connects_;
static int64_t serialNumber_ = 0;
static std::mutex g_connectMapMtx;
} // namespace AbilityRuntime
} // namespace OHOS
#endif // ABILITY_RUNTIME_JS_INPUTMETHOD_EXTENSION_CONTEXT_H