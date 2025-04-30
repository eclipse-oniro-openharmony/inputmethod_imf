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
#include "input_method_controller_impl.h"
#include "input_method_text_changed_listener.h"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "input_method_controller.h"
#include "ani_common.h"
#include "js_utils.h"
#include <cstdint>
#include "string_ex.h"

namespace OHOS {
namespace MiscServices {
using namespace taihe;
std::mutex InputMethodControllerImpl::controllerMutex_;
std::shared_ptr<InputMethodControllerImpl> InputMethodControllerImpl::controller_ { nullptr };
std::shared_ptr<InputMethodControllerImpl> InputMethodControllerImpl::GetInstance()
{
    if (controller_ == nullptr) {
        std::lock_guard<std::mutex> lock(controllerMutex_);
        if (controller_ == nullptr) {
            auto controller = std::make_shared<InputMethodControllerImpl>();
            controller_ = controller;
            InputMethodController::GetInstance()->SetControllerListener(controller_);
        }
    }
    return controller_;
}

void InputMethodControllerImpl::HideSoftKeyboardSync()
{
    int32_t errCode = InputMethodController::GetInstance()->HideSoftKeyboard();
    if (errCode != ErrorCode::NO_ERROR) {
        set_business_error(JsUtils::Convert(errCode), JsUtils::ToMessage(JsUtils::Convert(errCode)));
    }
}

void InputMethodControllerImpl::ShowTextInputHasParam(RequestKeyboardReason_t requestKeyboardReason)
{
    AttachOptions attachOptions;
    attachOptions.requestKeyboardReason = static_cast<RequestKeyboardReason>(requestKeyboardReason.get_value());
    int32_t errCode = InputMethodController::GetInstance()->ShowTextInput(attachOptions, ClientType::JS);
    if (errCode != ErrorCode::NO_ERROR) {
        set_business_error(JsUtils::Convert(errCode), JsUtils::ToMessage(JsUtils::Convert(errCode)));
    }
}

void InputMethodControllerImpl::ShowTextInputSync()
{
    ShowTextInputHasParam(RequestKeyboardReason_t::key_t::NONE);
}

void InputMethodControllerImpl::HideTextInputSync()
{
    int32_t errCode = InputMethodController::GetInstance()->HideTextInput();
    if (errCode != ErrorCode::NO_ERROR) {
        set_business_error(JsUtils::Convert(errCode), JsUtils::ToMessage(JsUtils::Convert(errCode)));
    }
}

void InputMethodControllerImpl::AttachSync(bool showKeyboard, TextConfig_t const& textConfig)
{
    AttachWithReason(showKeyboard, textConfig, RequestKeyboardReason_t::key_t::NONE);
}

void InputMethodControllerImpl::AttachWithReason(bool showKeyboard, TextConfig_t const& textConfig, RequestKeyboardReason_t requestKeyboardReason)
{
    AttachOptions attachOptions;
    attachOptions.isShowKeyboard = showKeyboard;
    attachOptions.requestKeyboardReason = static_cast<RequestKeyboardReason>(requestKeyboardReason.get_value());
    TextConfig config;
    config.inputAttribute.inputPattern = textConfig.inputAttribute.textInputType.get_value();
    config.inputAttribute.enterKeyType = textConfig.inputAttribute.enterKeyType.get_value();
    config.cursorInfo.left = textConfig.cursorInfo.left;
    config.cursorInfo.top = textConfig.cursorInfo.top;
    config.cursorInfo.width = textConfig.cursorInfo.width;
    config.cursorInfo.height = textConfig.cursorInfo.height;
    config.range.start = textConfig.selection.start;
    config.range.end = textConfig.selection.end;
    config.windowId = textConfig.windowId;
    // TODO isTextPreviewSupported

    int32_t errCode = InputMethodController::GetInstance()->Attach(InputMethodTextChangedListener::GetInstance(), attachOptions, config, ClientType::JS);
    if (errCode != ErrorCode::NO_ERROR) {
        set_business_error(JsUtils::Convert(errCode), JsUtils::ToMessage(JsUtils::Convert(errCode)));
    }
}

void InputMethodControllerImpl::DetachSync()
{
    int32_t errCode = InputMethodController::GetInstance()->Close();
    if (errCode != ErrorCode::NO_ERROR) {
        set_business_error(JsUtils::Convert(errCode), JsUtils::ToMessage(JsUtils::Convert(errCode)));
    }
}

void InputMethodControllerImpl::RegisterListener(std::string const& type, callbackType&& cb, uintptr_t opq)
{
    std::lock_guard<std::mutex> lock(mutex_);
    ani_object callbackObj = reinterpret_cast<ani_object>(opq);
    ani_ref callbackRef;
    ani_env *env = taihe::get_env();
    if (env == nullptr || ANI_OK != env->GlobalReference_Create(callbackObj, &callbackRef)) {
        return;
    }
    auto& cbVec = jsCbMap_[type]; 
    bool isDuplicate = std::any_of(cbVec.begin(), cbVec.end(),
        [env, callbackRef](const CallbackObject& obj) {
            ani_boolean isEqual = false;
            return (ANI_OK == env->Reference_StrictEquals(callbackRef, obj.ref, &isEqual)) && isEqual;
        }
    );
    if (isDuplicate) {
        env->GlobalReference_Delete(callbackRef);
        return;
    }
    cbVec.emplace_back(cb, callbackRef);
}
void InputMethodControllerImpl::UnRegisterListener(std::string const& type, taihe::optional_view<uintptr_t> opq)
{
    std::lock_guard<std::mutex> lock(mutex_);
    const auto iter = jsCbMap_.find(type);
    if (iter == jsCbMap_.end()) {
        return;
    }
    
    if (!opq.has_value()) {
        jsCbMap_.erase(iter);
        return;
    }
    
    ani_env* env = taihe::get_env();
    if (env == nullptr) {
        return;
    }

    GlobalRefGuard guard(env, reinterpret_cast<ani_object>(opq.value()));
    if (!guard) {
        return;
    }
    
    const auto pred = [env, targetRef = guard.get()](const CallbackObject& obj) {
        ani_boolean is_equal = false;
        return (ANI_OK == env->Reference_StrictEquals(targetRef, obj.ref, &is_equal)) && is_equal;
    };
    auto& callbacks = iter->second;
    const auto it = std::find_if(callbacks.begin(), callbacks.end(), pred);
    if (it != callbacks.end()) {
        callbacks.erase(it);
    }
    if (callbacks.empty()) {
        jsCbMap_.erase(iter);
    }
}


void InputMethodControllerImpl::InsertTextCallback(const std::u16string &text)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["insertText"];
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<void(taihe::string_view)>>(cb.callback);
        taihe::string textStr = Str16ToStr8(text);
        func(textStr);
    }
}
void InputMethodControllerImpl::DeleteLeftCallback(int32_t length)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["deleteLeft"];
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<void(int32_t)>>(cb.callback);
        func(length);
    }
}
void InputMethodControllerImpl::DeleteRightCallback(int32_t length)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["deleteRight"];
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<void(int32_t)>>(cb.callback);
        func(length);
    }
}
void InputMethodControllerImpl::SendKeyboardStatusCallback(const KeyboardStatus &status)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["sendKeyboardStatus"];
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<void(KeyboardStatus_t)>>(cb.callback);
        KeyboardStatus_t state = EnumConvert::ConvertKeyboardStatus(status);
        func(state);
    }
}
void InputMethodControllerImpl::SendFunctionKeyCallback(const FunctionKey &key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["sendFunctionKey"];
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<void(FunctionKey_t const&)>>(cb.callback);
        FunctionKey_t funcKey{.enterKeyType = EnumConvert::ConvertEnterKeyType(key.GetEnterKeyType())};
        func(funcKey);
    }
}
void InputMethodControllerImpl::MoveCursorCallback(const Direction &direction)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["moveCursor"];
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<void(Direction_t)>>(cb.callback);
        Direction_t directionType = EnumConvert::ConvertDirection(direction);
        func(directionType);
    }
}
void InputMethodControllerImpl::HandleExtendActionCallback(int32_t action)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["handleExtendAction"];
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<void(ExtendAction_t)>>(cb.callback);
        ExtendAction_t actionType = EnumConvert::ConvertExtendAction(action);
        func(actionType);
    }
}
std::u16string InputMethodControllerImpl::GetLeftTextOfCursorCallback(int32_t number)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["getLeftTextOfCursor"];
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<taihe::string_view(int32_t)>>(cb.callback);
        taihe::string s = func(number);
        return Str8ToStr16(std::string(s));
    }
    return Str8ToStr16(std::string());
}
std::u16string InputMethodControllerImpl::GetRightTextOfCursorCallback(int32_t number)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["getRightTextOfCursor"];
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<taihe::string_view(int32_t)>>(cb.callback);
        taihe::string s = func(number);
        return Str8ToStr16(std::string(s));
    }
    return Str8ToStr16(std::string());
}
int32_t InputMethodControllerImpl::GetTextIndexAtCursorCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["getTextIndexAtCursor"];
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<int32_t()>>(cb.callback); 
        return func();
    }
    return 0;
}

void InputMethodControllerImpl::OnSelectByRange(int32_t start, int32_t end)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["selectByRange"]; 
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<void(Range_t const&)>>(cb.callback);
        Range_t range{.start = start, .end = end};
        func(range);
    }
}
void InputMethodControllerImpl::OnSelectByMovement(int32_t direction)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto& cbVec = jsCbMap_["selectByMovement"]; 
    for(auto &cb : cbVec) {
        auto& func = std::get<taihe::callback<void(Movement_t const&)>>(cb.callback);
        Movement_t movement{.direction  = EnumConvert::ConvertDirection(static_cast<Direction>(direction))};
        func(movement);
    }
}
}
}