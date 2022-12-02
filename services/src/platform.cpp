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

#include "platform.h"

#include "global.h"
#include "platform_callback_stub.h"

namespace OHOS {
namespace MiscServices {
/*! Constructor
    */
Platform::Platform()
{
}

/*! Destructor
    */
Platform::~Platform()
{
}

/*! Single instance exists in the service
    */
Platform *Platform::Instance()
{
    static Platform *platform = nullptr;
    if (!platform) {
        platform = new Platform();
    }
    return platform;
}

/*! Destroy window token
      \param userId the id of the given user.
      \param packageName the packageName of the given input method engine
      \return ErrorCode
    */
int Platform::DestroyWindowToken(int userId, const std::u16string &packageName)
{
    if (!platformApi) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return platformApi->destroyWindowToken(userId, packageName);
}

/*! Get all the installed input method engines for the given user
      \param userId the id of the given user.
      \param[out] inputMethodProperties the input method engine list installed in the system for the given user
      \return ErrorCode
    */
int Platform::ListInputMethod(int userId, std::vector<InputMethodInfo *> *inputMethodProperties)
{
    return 0;
}

/*! Get input method engine information of the given package for the given user
      \param userId the id of the given user.
      \param packageName the given package name for which to get input method engine information
      \param[out] inputMethodProperty the input method engine information for the given package
      \return ErrorCode
    */
int Platform::GetInputMethodProperty(
    int userId, const std::u16string &packageName, InputMethodInfo *inputMethodProperty)
{
    if (!platformApi) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return platformApi->getInputMethodProperty(userId, packageName, inputMethodProperty);
}

/*! Get the input method setting data for the given user
      \param userId the id of the given user
      \param[out] inputMethodSetting the input method setting data for the given user
      \return ErrorCode
    */

int Platform::GetInputMethodSetting(int userId, InputMethodSetting *inputMethodSetting)
{
    return 0;
}

/*! Set the input method setting data for the given user
      \param userId the id of the given user
      \param inputMethodSetting the input method setting data for the given user
      \return ErrorCode
    */
int Platform::SetInputMethodSetting(int userId, const InputMethodSetting &inputMethodSetting)
{
    if (!platformApi) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    return platformApi->setInputMethodSetting(userId, inputMethodSetting);
}
} // namespace MiscServices
} // namespace OHOS
