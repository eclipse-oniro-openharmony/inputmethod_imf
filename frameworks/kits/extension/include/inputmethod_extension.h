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

#ifndef FOUNDATION_ABILITYRUNTIME_OHOS_INPUTMETHOD_EXTENSION_H
#define FOUNDATION_ABILITYRUNTIME_OHOS_INPUTMETHOD_EXTENSION_H

#include "extension_base.h"
#include "inputmethod_extension_context.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @brief Basic inputmethod components.
 */
class InputMethodExtension : public ExtensionBase<InputMethodExtensionContext> {
public:
    InputMethodExtension() = default;
    virtual ~InputMethodExtension() = default;

    /**
   * @brief Create and init context.
   *
   * @param record the extension record.
   * @param application the application info.
   * @param handler the extension handler.
   * @param token the remote token.
   * @return The created context.
   */
    virtual std::shared_ptr<InputMethodExtensionContext>
    CreateAndInitContext(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
   * @brief Init the extension.
   *
   * @param record the extension record.
   * @param application the application info.
   * @param handler the extension handler.
   * @param token the remote token.
   */
    virtual void Init(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
   * @brief Create Extension.
   *
   * @param runtime The runtime.
   * @return The InputMethodExtension instance.
   */
    static InputMethodExtension *Create(const std::unique_ptr<Runtime> &runtime);

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const AppExecFwk::Configuration &config) override;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITYRUNTIME_OHOS_INPUTMETHOD_EXTENSION_H