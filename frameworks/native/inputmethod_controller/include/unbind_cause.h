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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_UNBIND_CAUSE_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_UNBIND_CAUSE_H

namespace OHOS {
namespace MiscServices {
enum class UnBindCause : uint32_t {
    CLIENT_DIED = 0, //IMA停止输入core->StopInput()，removeclient() 不更新isshowkeyboard
    CLIENT_UNFOCUSED, // 清理imc的agent、监听、绑定标记等等(client->OnInputStop)，core->StopInput()， removeclient() 不更新isshowkeyboard
    CLIENT_CLOSE_SELF, //清理imc的agent、监听、绑定标记等等(发起close时已清理)，core->StopInput()， removeclient() 不更新isshowkeyboard
    IME_DIED,   //清理imc的agent   不更新isshowkeyboard
    IME_SWITCH, //清理imc的agent，core->StopInput(), 不更新isshowkeyboard
    IME_CLEAR_SELF,   //清理imc的agent，core->StopInput()  仅限proxy,不更新isshowkeyboard
};
} // namespace MiscServices
} // namespace OHOS
#endif
