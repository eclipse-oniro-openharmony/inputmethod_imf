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

#ifndef CJ_INPUTMETHOD_EXTENSION_H
#define CJ_INPUTMETHOD_EXTENSION_H

#include "cj_inputmethod_extension_context.h"
#include "cj_inputmethod_extension_object.h"
#include "configuration.h"
#include "display_manager.h"
#include "inputmethod_extension.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @brief Basic inputmethod components.
 */
class CjInputMethodExtension : public InputMethodExtension {
    struct CacheDisplay {
        int32_t displayWidth = 0;
        int32_t displayHeight = 0;
        Rosen::Rotation displayRotation = Rosen::Rotation::ROTATION_0;
        Rosen::FoldStatus displayFoldStatus = Rosen::FoldStatus::UNKNOWN;
        bool IsEmpty()
        {
            return displayWidth == 0 && displayHeight == 0 && displayRotation == Rosen::Rotation::ROTATION_0 &&
                displayFoldStatus == Rosen::FoldStatus::UNKNOWN;
        };
        void SetCacheDisplay(int32_t width, int32_t height, Rosen::Rotation rotation, Rosen::FoldStatus foldStatus)
        {
            displayWidth = width;
            displayHeight = height;
            displayRotation = rotation;
            displayFoldStatus = foldStatus;
        };
    };

public:
    CjInputMethodExtension();
    ~CjInputMethodExtension() override;
    static CjInputMethodExtension *cjInputMethodExtension;
    /**
     * @brief Create CjInputMethodExtension.
     *
     * @param runtime The runtime.
     * @return The CjInputMethodExtension instance.
     */
    static CjInputMethodExtension *Create(const std::unique_ptr<Runtime> &runtime);

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;

    /**
     * @brief Called when this extension is started. You must override this function if you want to perform some
     *        initialization operations during extension startup.
     *
     * This function can be called only once in the entire lifecycle of an extension.
     * @param Want Indicates the {@link Want} structure containing startup information about the extension.
     */
    void OnStart(const AAFwk::Want &want) override;

    /**
     * @brief Called when this InputMethod extension is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the InputMethod
     *        extension.
     *
     * @return Returns a pointer to the <b>sid</b> of the connected InputMethod extension.
     */
    sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;

    /**
     * @brief Called when all abilities connected to this InputMethod extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     *
     */
    void OnDisconnect(const AAFwk::Want &want) override;

    /**
     * @brief Called back when InputMethod is started.
     * This method can be called only by InputMethod. You can use the StartAbility(ohos.aafwk.content.Want) method
     *        to start InputMethod. Then the system calls back the current method to use the transferred want parameter
     *        to execute its own logic.
     *
     * @param want Indicates the want of InputMethod to start.
     * @param restart Indicates the startup mode. The value true indicates that InputMethod is restarted after being
     *        destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the InputMethod extension has been started. The startId is
     *        incremented.
     * by 1 every time the extension is started. For example, if the extension has been started for six times, the
     * value of startId is 6.
     */
    void OnCommand(const AAFwk::Want &want, bool restart, int startId) override;

    /**
     * @brief Called when this extension enters the <b>STATE_STOP</b> state.
     *
     * The extension in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    void OnStop() override;

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const AppExecFwk::Configuration &config) override;

    /**
     * @brief Called when configuration changed, including system configuration and window configuration.
     *
     */
    void ConfigurationUpdated();

    void SetCjContext(sptr<CjInputMethodExtensionContext> cjContext)
    {
        cjContext_ = cjContext;
    }

private:
    void ListenWindowManager();

    void InitDisplayCache();

    CjInputMethodExtensionObject cjObj_;
    sptr<CjInputMethodExtensionContext> cjContext_ = nullptr;
    std::shared_ptr<AbilityHandler> handler_ = nullptr;
    CacheDisplay cacheDisplay_;

protected:
    class CjInputMethodExtensionDisplayListener : public Rosen::DisplayManager::IDisplayListener {
    public:
        explicit CjInputMethodExtensionDisplayListener(const std::weak_ptr<CjInputMethodExtension> &extension)
        {
            cjInputMethodExtension_ = extension;
        }

        void OnCreate(Rosen::DisplayId displayId) override
        {
            auto inputMethodSptr = cjInputMethodExtension_.lock();
            if (inputMethodSptr != nullptr) {
                inputMethodSptr->OnCreate(displayId);
            }
        }

        void OnDestroy(Rosen::DisplayId displayId) override
        {
            auto inputMethodSptr = cjInputMethodExtension_.lock();
            if (inputMethodSptr != nullptr) {
                inputMethodSptr->OnDestroy(displayId);
            }
        }

        void OnChange(Rosen::DisplayId displayId) override
        {
            auto inputMethodSptr = cjInputMethodExtension_.lock();
            if (inputMethodSptr != nullptr) {
                inputMethodSptr->CheckNeedAdjustKeyboard(displayId);
                inputMethodSptr->OnChange(displayId);
            }
        }

    private:
        std::weak_ptr<CjInputMethodExtension> cjInputMethodExtension_;
    };

    void OnCreate(Rosen::DisplayId displayId);
    void OnDestroy(Rosen::DisplayId displayId);
    void OnChange(Rosen::DisplayId displayId);
    void CheckNeedAdjustKeyboard(Rosen::DisplayId displayId);

private:
    class SystemAbilityStatusChangeListener : public OHOS::SystemAbilityStatusChangeStub {
    public:
        SystemAbilityStatusChangeListener(sptr<CjInputMethodExtensionDisplayListener> displayListener)
            : listener_(displayListener) { };
        ~SystemAbilityStatusChangeListener()
        {
            if (listener_ != nullptr) {
                Rosen::DisplayManager::GetInstance().RegisterDisplayListener(listener_);
            }
        }
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override { }

    private:
        sptr<CjInputMethodExtensionDisplayListener> listener_ = nullptr;
    };

    sptr<CjInputMethodExtensionDisplayListener> displayListener_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // CJ_INPUTMETHOD_EXTENSION_H