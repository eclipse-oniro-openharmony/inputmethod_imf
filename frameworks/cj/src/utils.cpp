/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "utils.h"

namespace OHOS::MiscServices {
char* Utils::MallocCString(const std::string &origin)
{
    if (origin.empty()) {
        return nullptr;
    }
    auto len = origin.length() + 1;
    char *res = static_cast<char *>(malloc(sizeof(char) * len));
    if (res == nullptr) {
        return nullptr;
    }
    return std::char_traits<char>::copy(res, origin.c_str(), len);
}

void Utils::InputMethodProperty2C(CInputMethodProperty &props, const Property &property)
{
    props.name = Utils::MallocCString(property.name);
    props.id = Utils::MallocCString(property.id);
    props.label = Utils::MallocCString(property.label);
    props.labelId = property.labelId;
    props.icon = Utils::MallocCString(property.icon);
    props.iconId = property.iconId;
}

Property Utils::C2InputMethodProperty(CInputMethodProperty props)
{
    Property property;
    property.name = std::string(props.name);
    property.id = std::string(props.id);
    property.label = std::string(props.label);
    property.labelId = props.labelId;
    property.icon = std::string(props.icon);
    property.iconId = props.iconId;
    return property;
}

void Utils::InputMethodSubProperty2C(CInputMethodSubtype &props, const SubProperty &property)
{
    props.name = Utils::MallocCString(property.name);
    props.id = Utils::MallocCString(property.id);
    props.label = Utils::MallocCString(property.label);
    props.labelId = property.labelId;
    props.icon = Utils::MallocCString(property.icon);
    props.iconId = property.iconId;
    props.mode = Utils::MallocCString(property.mode);
    props.locale = Utils::MallocCString(property.locale);
    props.language = Utils::MallocCString(property.language);
}
}