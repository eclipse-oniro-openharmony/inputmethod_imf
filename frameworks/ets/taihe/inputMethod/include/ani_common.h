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

#ifndef INPUT_METHOD_TAIHE_ANI_COMMON_H
#define INPUT_METHOD_TAIHE_ANI_COMMON_H

#include "input_method_property.h"
#include "input_method_utils.h"
#include "ohos.inputMethod.impl.hpp"
#include "ohos.inputMethod.proj.hpp"
#include "taihe/runtime.hpp"
using InputMethodProperty_t = ohos::inputMethod::InputMethodProperty;
using InputMethodSubtype_t = ohos::inputMethodSubtype::InputMethodSubtype;
using PanelInfo_t = ohos::inputMethod::Panel::PanelInfo;
using InputWindowInfo_t = ohos::inputMethod::InputWindowInfo;
using RequestKeyboardReason_t = ohos::inputMethod::RequestKeyboardReason;
using TextConfig_t = ohos::inputMethod::TextConfig;
using Range_t = ohos::inputMethod::Range;
using Movement_t = ohos::inputMethod::Movement;
using KeyboardStatus_t = ohos::inputMethod::KeyboardStatus;
using FunctionKey_t = ohos::inputMethod::FunctionKey;
using ExtendAction_t = ohos::inputMethod::ExtendAction;
using EnterKeyType_t = ohos::inputMethod::EnterKeyType;
using Direction_t = ohos::inputMethod::Direction;
namespace OHOS {
namespace MiscServices {
constexpr const int32_t SELECT_ALL = 0;
constexpr const int32_t CUT = 3;
constexpr const int32_t COPY = 4;
constexpr const int32_t PASTE = 5;

class PropertyConverter {
public:
    static InputMethodProperty_t ConvertProperty(const std::shared_ptr<Property> &obj)
    {
        return ConvertPropertyImpl(*obj);
    }

    static InputMethodProperty_t ConvertProperty(const Property &obj)
    {
        return ConvertPropertyImpl(obj);
    }

    static InputMethodSubtype_t ConvertSubProperty(const std::shared_ptr<SubProperty> &obj)
    {
        return ConvertSubPropertyImpl(*obj);
    }

    static InputMethodSubtype_t ConvertSubProperty(const SubProperty &obj)
    {
        return ConvertSubPropertyImpl(obj);
    }

private:
    template<typename T>
    static InputMethodProperty_t ConvertPropertyImpl(T &&obj)
    {
        static_assert(std::is_same_v<std::decay_t<T>, Property>, "Invalid type for Property conversion");

        InputMethodProperty_t result{};
        result.packageName = std::forward<T>(obj).name;
        result.name = std::forward<T>(obj).name;
        result.methodId = obj.id;
        result.id = obj.id;
        result.label = taihe::optional<taihe::string>(std::in_place_t{}, obj.label);
        result.labelId = taihe::optional<uint32_t>(std::in_place_t{}, obj.labelId);
        result.icon = taihe::optional<taihe::string>(std::in_place_t{}, obj.icon);
        result.iconId = taihe::optional<uint32_t>(std::in_place_t{}, obj.iconId);
        return result;
    }

    template<typename T>
    static InputMethodSubtype_t ConvertSubPropertyImpl(T &&obj)
    {
        static_assert(std::is_same_v<std::decay_t<T>, SubProperty>, "Invalid type for SubProperty conversion");

        InputMethodSubtype_t result{};
        result.name = std::forward<T>(obj).name;
        result.id = obj.id;
        result.locale = obj.locale;
        result.language = obj.language;
        result.label = taihe::optional<taihe::string>(std::in_place_t{}, obj.label);
        result.labelId = taihe::optional<uint32_t>(std::in_place_t{}, obj.labelId);
        result.icon = taihe::optional<taihe::string>(std::in_place_t{}, obj.icon);
        result.iconId = taihe::optional<uint32_t>(std::in_place_t{}, obj.iconId);
        return result;
    }
};

using callbackType = std::variant<taihe::callback<int32_t()>, taihe::callback<taihe::string_view(int32_t)>,
    taihe::callback<void()>, taihe::callback<void(int32_t)>, taihe::callback<void(taihe::string_view)>,
    taihe::callback<void(Range_t const &)>, taihe::callback<void(Movement_t const &)>,
    taihe::callback<void(KeyboardStatus_t const)>, taihe::callback<void(Direction_t const)>,
    taihe::callback<void(FunctionKey_t const &)>, taihe::callback<void(EnterKeyType_t const)>,
    taihe::callback<void(ExtendAction_t const)>, taihe::callback<void(taihe::array_view<InputWindowInfo_t>)>,
    taihe::callback<void(InputMethodProperty_t const &, InputMethodSubtype_t const &)>>;

struct CallbackObject {
    CallbackObject(callbackType cb, ani_ref ref) : callback(cb), ref(ref)
    {
    }
    ~CallbackObject()
    {
        if (auto *env = taihe::get_env()) {
            env->GlobalReference_Delete(ref);
        }
    }
    callbackType callback;
    ani_ref ref;
};

class GlobalRefGuard {
    ani_env *env_ = nullptr;
    ani_ref ref_ = nullptr;

public:
    GlobalRefGuard(ani_env *env, ani_object obj) : env_(env)
    {
        if (!env_)
            return;
        if (ANI_OK != env_->GlobalReference_Create(obj, &ref_)) {
            ref_ = nullptr;
        }
    }
    explicit operator bool() const
    {
        return ref_ != nullptr;
    }
    ani_ref get() const
    {
        return ref_;
    }
    ~GlobalRefGuard()
    {
        if (env_ && ref_) {
            env_->GlobalReference_Delete(ref_);
        }
    }

    GlobalRefGuard(const GlobalRefGuard &) = delete;
    GlobalRefGuard &operator=(const GlobalRefGuard &) = delete;
};

class EnumConvert {
public:
    static Direction_t ConvertDirection(Direction direction)
    {
        switch (direction) {
            case Direction::UP:
                return Direction_t::key_t::CURSOR_UP;
            case Direction::DOWN:
                return Direction_t::key_t::CURSOR_DOWN;
            case Direction::LEFT:
                return Direction_t::key_t::CURSOR_LEFT;
            case Direction::RIGHT:
                return Direction_t::key_t::CURSOR_RIGHT;
            default:
                return Direction_t::key_t::CURSOR_UP;
        }
    }

    static ExtendAction_t ConvertExtendAction(int32_t action)
    {
        switch (action) {
            case SELECT_ALL:
                return ExtendAction_t::key_t::SELECT_ALL;
            case CUT:
                return ExtendAction_t::key_t::CUT;
            case COPY:
                return ExtendAction_t::key_t::COPY;
            case PASTE:
                return ExtendAction_t::key_t::PASTE;
            default:
                return ExtendAction_t::key_t::SELECT_ALL;
        }
    }

    static EnterKeyType_t ConvertEnterKeyType(EnterKeyType type)
    {
        switch (type) {
            case EnterKeyType::UNSPECIFIED:
                return EnterKeyType_t::key_t::UNSPECIFIED;
            case EnterKeyType::NONE:
                return EnterKeyType_t::key_t::NONE;
            case EnterKeyType::GO:
                return EnterKeyType_t::key_t::GO;
            case EnterKeyType::SEARCH:
                return EnterKeyType_t::key_t::SEARCH;
            case EnterKeyType::SEND:
                return EnterKeyType_t::key_t::SEND;
            case EnterKeyType::NEXT:
                return EnterKeyType_t::key_t::NEXT;
            case EnterKeyType::DONE:
                return EnterKeyType_t::key_t::DONE;
            case EnterKeyType::PREVIOUS:
                return EnterKeyType_t::key_t::PREVIOUS;
            case EnterKeyType::NEW_LINE:
                return EnterKeyType_t::key_t::NEWLINE;
            default:
                return EnterKeyType_t::key_t::UNSPECIFIED;
        }
    }

    static KeyboardStatus_t ConvertKeyboardStatus(KeyboardStatus status)
    {
        switch (status) {
            case KeyboardStatus::NONE:
                return KeyboardStatus_t::key_t::NONE;
            case KeyboardStatus::SHOW:
                return KeyboardStatus_t::key_t::SHOW;
            case KeyboardStatus::HIDE:
                return KeyboardStatus_t::key_t::HIDE;
            default:
                return KeyboardStatus_t::key_t::NONE;
        }
    }
};
} // namespace MiscServices
} // namespace OHOS
#endif