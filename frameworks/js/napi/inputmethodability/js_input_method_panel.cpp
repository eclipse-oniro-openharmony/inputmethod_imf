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

#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_base_context.h"
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
    napi_value panelType = nullptr;
    napi_value typeSoftKeyboard = nullptr;
    napi_value typeStatusBar = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(PanelType::SOFT_KEYBOARD), &typeSoftKeyboard));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(PanelType::STATUS_BAR), &typeStatusBar));
    NAPI_CALL(env, napi_create_object(env, &panelType));
    NAPI_CALL(env, napi_set_named_property(env, panelType, "SOFT_KEYBOARD", typeSoftKeyboard));
    NAPI_CALL(env, napi_set_named_property(env, panelType, "STATUS_BAR", typeStatusBar));
    return panelType;
}

napi_value JsInputMethodPanel::GetJsPanelFlagProperty(napi_env env)
{
    napi_value panelFlag = nullptr;
    napi_value flagFixed = nullptr;
    napi_value flagFloating = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(PanelFlag::FLG_FIXED), &flagFixed));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(PanelFlag::FLG_FLOATING), &flagFloating));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(PanelFlag::FLG_CANDIDATE_COLUMN), &flagFloating));
    NAPI_CALL(env, napi_create_object(env, &panelFlag));
    NAPI_CALL(env, napi_set_named_property(env, panelFlag, "FLAG_FIXED", flagFixed));
    NAPI_CALL(env, napi_set_named_property(env, panelFlag, "FLAG_FLOATING", flagFloating));
    NAPI_CALL(env, napi_set_named_property(env, panelFlag, "FLAG_CANDIDATE", flagFloating));
    return panelFlag;
}
} // namespace MiscServices
} // namespace OHOS