/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "input_method_info.h"

namespace OHOS {
namespace MiscServices {
    using namespace std;
    /*! Constructor
    */
    InputMethodInfo::InputMethodInfo()
    {
    }

    /*! Destructor
    */
    InputMethodInfo::~InputMethodInfo()
    {
        for (int32_t i = 0; i < (int32_t)mTypes.size(); i++) {
            delete mTypes[i];
        }
        mTypes.clear();
    }

    /*! Constructor
    \param property the source property will be copied to this instance.
    */
    InputMethodInfo::InputMethodInfo(const InputMethodInfo & property)
    {
        mImeId = property.mImeId;
        mPackageName = property.mPackageName;
        mAbilityName = property.mAbilityName;
        mConfigurationPage = property.mConfigurationPage;
        isSystemIme = property.isSystemIme;
        mDefaultImeId = property.mDefaultImeId;
        labelId = property.labelId;
        descriptionId = property.descriptionId;
        label = property.label;
        description = property.description;

        for (int i = 0; i < (int)mTypes.size(); i++) {
            KeyboardType *type = new KeyboardType(*property.mTypes[i]);
            mTypes.push_back(type);
        }
    }

    /*! operator=
    \param property the source property will be copied to this instance.
    \return return this
    */
    InputMethodInfo &InputMethodInfo::operator =(const InputMethodInfo & property)
    {
        if (this == &property) {
            return *this;
        }
        mImeId = property.mImeId;
        mPackageName = property.mPackageName;
        mAbilityName = property.mAbilityName;
        mConfigurationPage = property.mConfigurationPage;
        isSystemIme = property.isSystemIme;
        mDefaultImeId = property.mDefaultImeId;
        labelId = property.labelId;
        descriptionId = property.descriptionId;
        label = property.label;
        description = property.description;

        for (int i = 0; i < (int)mTypes.size(); i++) {
            KeyboardType *type = new KeyboardType(*property.mTypes[i]);
            mTypes.push_back(type);
        }
        return *this;
    }
} // namespace MiscServices
} // namespace OHOS
