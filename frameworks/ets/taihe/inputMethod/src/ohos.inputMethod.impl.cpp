#include "ohos.inputMethod.proj.hpp"
#include "ohos.inputMethod.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "input_method_controller_impl.h"
#include "input_method_setting_impl.h"
#include "input_method_controller.h"
#include "ani_common.h"
#include "js_utils.h"

using namespace taihe;
using namespace ohos::inputMethod;
using namespace ohos::inputMethodSubtype;
using namespace OHOS::MiscServices;
namespace {
InputMethodSetting GetSetting() {
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<IMFSettingImpl, InputMethodSetting>();
}

ohos::inputMethod::InputMethodController GetController() {
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<IMFControllerImpl, ohos::inputMethod::InputMethodController>();
}

InputMethodProperty GetDefaultInputMethod() {
    InputMethodProperty inputMethodProperty{};
    std::shared_ptr<Property> property;
    int32_t ret = OHOS::MiscServices::InputMethodController::GetInstance()->GetDefaultInputMethod(property);
    if (ret != ErrorCode::NO_ERROR) {
        taihe::set_business_error(JsUtils::Convert(ret), "failed to get default input method!");
        return inputMethodProperty;
    }
    inputMethodProperty = PropertyConverter::ConvertProperty(property);
    return inputMethodProperty;
}

InputMethodProperty GetCurrentInputMethod() {
    InputMethodProperty inputMethodProperty{};
    std::shared_ptr<Property> property = OHOS::MiscServices::InputMethodController::GetInstance()->GetCurrentInputMethod();
    if (property == nullptr) {
        IMSA_HILOGE("current input method is nullptr!");
        return inputMethodProperty;
    }
    inputMethodProperty = PropertyConverter::ConvertProperty(property);
    return inputMethodProperty;
}

InputMethodSubtype GetCurrentInputMethodSubtype() {
    InputMethodSubtype inputMethodSubtype{};
    std::shared_ptr<SubProperty> subProperty = OHOS::MiscServices::InputMethodController::GetInstance()->GetCurrentInputMethodSubtype();
    if (subProperty == nullptr) {
        IMSA_HILOGE("current input method subtype is nullptr!");
        return inputMethodSubtype;
    }
    inputMethodSubtype = PropertyConverter::ConvertSubProperty(subProperty);
    return inputMethodSubtype;  
}

uintptr_t GetSystemInputMethodConfigAbility() {
    TH_THROW(std::runtime_error, "GetSystemInputMethodConfigAbility not implemented");
}

bool SwitchInputMethodWithTarget(InputMethodProperty const& target) {
    std::string packageName(target.name);
    std::string id(target.id);
    if (packageName.empty() || id.empty()) {
        packageName = target.packageName;
        id = target.methodId;
    }
    int32_t errCode = 
    OHOS::MiscServices::InputMethodController::GetInstance()->SwitchInputMethod(SwitchTrigger::CURRENT_IME, packageName, id);
    if (errCode != ErrorCode::NO_ERROR) {
        taihe::set_business_error(JsUtils::Convert(errCode), JsUtils::ToMessage(JsUtils::Convert(errCode)));
        return false;
    }
    return true;
}

void SwitchInputMethodSync(string_view bundleName, optional_view<string> subtypeId) {
    std::string id;
    if (subtypeId.has_value()) {
        id = subtypeId.value();
    }
    int32_t errCode = 
    OHOS::MiscServices::InputMethodController::GetInstance()->SwitchInputMethod(SwitchTrigger::SYSTEM_APP, std::string(bundleName), id);
    if (errCode != ErrorCode::NO_ERROR) {
        taihe::set_business_error(JsUtils::Convert(errCode), JsUtils::ToMessage(JsUtils::Convert(errCode)));
    }
}

bool SwitchCurrentInputMethodSubtypeSync(InputMethodProperty const& target) {
    std::string name(target.name);
    std::string id(target.id);
    int32_t errCode =
    OHOS::MiscServices::InputMethodController::GetInstance()->SwitchInputMethod(SwitchTrigger::CURRENT_IME, name, id);
    if (errCode != ErrorCode::NO_ERROR) {
        taihe::set_business_error(JsUtils::Convert(errCode), JsUtils::ToMessage(JsUtils::Convert(errCode)));
        return false;
    }
    return true;
}
}  // namespace

TH_EXPORT_CPP_API_GetSetting(GetSetting);
TH_EXPORT_CPP_API_GetController(GetController);
TH_EXPORT_CPP_API_GetDefaultInputMethod(GetDefaultInputMethod);
TH_EXPORT_CPP_API_GetCurrentInputMethod(GetCurrentInputMethod);
TH_EXPORT_CPP_API_GetCurrentInputMethodSubtype(GetCurrentInputMethodSubtype);
TH_EXPORT_CPP_API_GetSystemInputMethodConfigAbility(GetSystemInputMethodConfigAbility);
TH_EXPORT_CPP_API_SwitchInputMethodWithTarget(SwitchInputMethodWithTarget);
TH_EXPORT_CPP_API_SwitchInputMethodSync(SwitchInputMethodSync);
TH_EXPORT_CPP_API_SwitchCurrentInputMethodSubtypeSync(SwitchCurrentInputMethodSubtypeSync);
