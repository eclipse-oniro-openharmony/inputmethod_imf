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

#include "input_control_channel_service_impl.h"

#include "ipc_skeleton.h"
#include "inputmethod_message_handler.h"
#include "message_parcel.h"
#include "os_account_adapter.h"

namespace OHOS {
namespace MiscServices {
InputControlChannelServiceImpl::InputControlChannelServiceImpl(int32_t userId)
{
    userId_ = userId;
}

InputControlChannelServiceImpl::~InputControlChannelServiceImpl() {}

ErrCode InputControlChannelServiceImpl::HideKeyboardSelf()
{
    auto userId = OsAccountAdapter::GetOsAccountLocalIdFromUid(IPCSkeleton::GetCallingUid());
    if (userId == OsAccountAdapter::INVALID_USER_ID) {
        return ErrorCode::ERROR_EX_ILLEGAL_STATE;
    }
    MessageParcel *parcel = new (std::nothrow) MessageParcel();
    if (parcel == nullptr) {
        return ErrorCode::ERROR_NULL_POINTER;
    }
    parcel->WriteInt32(userId);
    Message *msg = new (std::nothrow) Message(MessageID::MSG_ID_HIDE_KEYBOARD_SELF, parcel);
    if (msg == nullptr) {
        delete parcel;
        return ErrorCode::ERROR_NULL_POINTER;
    }
    MessageHandler::Instance()->SendMessage(msg);
    return ERR_OK;
}
} // namespace MiscServices
} // namespace OHOS