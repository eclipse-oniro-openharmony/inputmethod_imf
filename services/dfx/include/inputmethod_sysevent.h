/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#ifndef INPUTMETHOD_SYSEVENT_H
#define INPUTMETHOD_SYSEVENT_H

#include <string>
#include <map>

#include "global.h"

namespace OHOS {
namespace MiscServices {
enum IMEBehaviour : int32_t {
    START_IME = 0,
    CHANGE_IME,
};

enum OperateIMEInfoCode : int32_t {
    IME_SHOW_ATTACH = 0,
    IME_SHOW_ENEDITABLE,
    IME_SHOW_NORMAL,
    IME_UNBIND,
    IME_HIDE_UNBIND,
    IME_HIDE_UNEDITABLE,
    IME_HIDE_NORMAL,
    IME_HIDE_UNFOCUSED,
    IME_HIDE_SELF,
};

class InputmethodSysevent {
public:
    static void FaultReporter(int32_t userId, std::string bundleName, int32_t errCode);
    static void CreateComponentFailed(int32_t userId, int32_t errCode);
    static void BehaviourReporter(IMEBehaviour ActiveName);
    static void OperateSoftkeyboardBehaviour(OperateIMEInfoCode infoCode);

private:
    using TimerCallback = std::function<void()>;
    static void InvokeInputmethodStatistic();
    static std::string GetOperateInfo(OperateIMEInfoCode infoCode);
    static std::string GetOperateAction(OperateIMEInfoCode infoCode);

private:
    static std::map<std::string, int32_t> inputmethodBehaviour_;
    static const std::map<int32_t, std::string> oprateInfo_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // INPUTMETHOD_SYSEVENT_H