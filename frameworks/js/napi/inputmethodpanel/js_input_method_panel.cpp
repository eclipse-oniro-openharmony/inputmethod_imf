/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_input_method_panel.h"

#include "global.h"
#include "js_runtime_utils.h"
#include "js_util.h"
#include "panel_info.h"

namespace OHOS {
namespace MiscServices {
napi_value JsInputMethodPanel::Init(napi_env env, napi_value exports)
{
    napi_set_named_property(env, exports, "PanelType", GetJsPanelTypeProperty(env));
    napi_set_named_property(env, exports, "PanelFlag", GetJsPanelFlagProperty(env));
    return exports;
}

napi_value JsInputMethodPanel::GetJsPanelTypeProperty(napi_env env)
{
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    auto ret = JsUtil::Object::WriteProperty(env, obj, "SOFT_KEYBOARD", static_cast<int32_t>(PanelType::SOFT_KEYBOARD));
    ret = ret && JsUtil::Object::WriteProperty(env, obj, "STATUS_BAR", static_cast<int32_t>(PanelType::STATUS_BAR));
    if (!ret) {
        IMSA_HILOGE("init module inputMethod.Panel.PanelType failed, ret: %{public}d!", ret);
    }
    return obj;
}

napi_value JsInputMethodPanel::GetJsPanelFlagProperty(napi_env env)
{
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    auto ret = JsUtil::Object::WriteProperty(env, obj, "FLAG_FIXED", static_cast<int32_t>(PanelFlag::FLG_FIXED));
    ret = ret &&
          JsUtil::Object::WriteProperty(env, obj, "FLAG_FLOATING", static_cast<int32_t>(PanelFlag::FLG_FLOATING));
    ret = ret && JsUtil::Object::WriteProperty(env, obj, "FLAG_CANDIDATE",
                                               static_cast<int32_t>(PanelFlag::FLG_CANDIDATE_COLUMN));
    if (!ret) {
        IMSA_HILOGI("init module inputMethod.Panel.PanelFlag failed, ret: %{public}d!", ret);
    }
    return obj;
}
} // namespace MiscServices
} // namespace OHOS