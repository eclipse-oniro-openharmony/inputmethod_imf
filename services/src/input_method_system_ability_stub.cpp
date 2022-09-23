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

#include "input_method_system_ability_stub.h"

#include <memory>

#include "access_token.h"
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "message_handler.h"

namespace OHOS {
namespace MiscServices {
    using namespace MessageID;
    using namespace Security::AccessToken;
    static const std::string PERMISSION_CONNECT_IME_ABILITY = "ohos.permission.CONNECT_IME_ABILITY";
    /*! Handle the transaction from the remote binder
    \n Run in binder thread
    \param code transaction code number
    \param data the parameters from remote binder
    \param[out] reply the result of the transaction replied to the remote binder
    \param flags the flags of handling transaction
    \return int32_t
    */
    int32_t InputMethodSystemAbilityStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::OnRemoteRequest code = %{public}u", code);
        auto descriptorToken = data.ReadInterfaceToken();
        if (descriptorToken != GetDescriptor()) {
            return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
        }

        switch (code) {
            case PREPARE_INPUT: {
                prepareInput(data);
                reply.WriteInt32(NO_ERROR);
                break;
            }
            case RELEASE_INPUT: {
                MessageParcel *msgParcel = (MessageParcel*) &data;
                releaseInput(*msgParcel);
                reply.WriteInt32(NO_ERROR);
                break;
            }
            case START_INPUT: {
                MessageParcel *msgParcel = (MessageParcel*) &data;
                startInput(*msgParcel);
                reply.WriteInt32(NO_ERROR);
                break;
            }
            case STOP_INPUT: {
                MessageParcel *msgParcel = (MessageParcel*) &data;
                stopInput(*msgParcel);
                reply.WriteInt32(NO_ERROR);
                break;
            }
            case SET_CORE_AND_AGENT: {
                SetCoreAndAgent(data);
                break;
            }
            case GET_DISPLAY_MODE: {
                int32_t mode = 0;
                int32_t status = getDisplayMode(mode);
                if (status == ErrorCode::NO_ERROR) {
                    reply.WriteInt32(NO_ERROR);
                    reply.WriteInt32(mode);
                } else {
                    reply.WriteInt32(ErrorCode::ERROR_EX_ILLEGAL_STATE);
                    reply.WriteInt32(-1);
                }
                break;
            }
            case GET_KEYBOARD_WINDOW_HEIGHT: {
                int32_t height = 0;
                int32_t status = getKeyboardWindowHeight(height);
                if (status == ErrorCode::NO_ERROR) {
                    reply.WriteInt32(NO_ERROR);
                    reply.WriteInt32(height);
                } else {
                    reply.WriteInt32(ErrorCode::ERROR_EX_ILLEGAL_STATE);
                    reply.WriteInt32(-1);
                }
                break;
            }
            case GET_CURRENT_KEYBOARD_TYPE: {
                KeyboardType type;
                int32_t status = getCurrentKeyboardType(&type);
                if (status == ErrorCode::NO_ERROR) {
                    reply.WriteInt32(NO_ERROR);
                    reply.WriteParcelable(&type);
                } else if (status == ErrorCode::ERROR_NULL_POINTER) {
                    reply.WriteInt32(NO_ERROR);
                    reply.WriteInt32(0);
                } else {
                    reply.WriteInt32(ErrorCode::ERROR_EX_ILLEGAL_STATE);
                    reply.WriteInt32(-1);
                }
                break;
            }
            case LIST_INPUT_METHOD: {
                OnListInputMethod(data, reply);
                break;
            }
            case LIST_KEYBOARD_TYPE: {
                std::u16string imeId = data.ReadString16();
                std::vector<KeyboardType*> kbdTypes;
                int32_t ret = listKeyboardType(imeId, &kbdTypes);
                if (ret != ErrorCode::NO_ERROR) {
                    reply.WriteInt32(ErrorCode::ERROR_EX_ILLEGAL_STATE);
                    reply.WriteInt32(-1);
                    return ret;
                }
                reply.WriteInt32(NO_ERROR);
                int32_t size = kbdTypes.size();
                reply.WriteInt32(size);
                for (int32_t i = 0; i < size; i++) {
                    reply.WriteParcelable(kbdTypes[i]);
                }
                kbdTypes.clear();
                break;
            }
            case DISPLAY_OPTIONAL_INPUT_METHOD: {
                displayOptionalInputMethod(data);
                reply.WriteInt32(NO_ERROR);
                break;
            }
            case HIDE_CURRENT_INPUT: {
                HideCurrentInput(data);
                reply.WriteInt32(NO_ERROR);
                break;
            }
            case SHOW_CURRENT_INPUT: {
                int32_t ret = ShowCurrentInput(data);
                reply.WriteInt32(ret);
                break;
            }
            case SWITCH_INPUT_METHOD: {
                int32_t ret = OnSwitchInputMethod(data);
                reply.WriteInt32(ret);
                break;
            }
            case GET_CURRENT_INPUT_METHOD: {
                OnGetCurrentInputMethod(reply);
                break;
            }
            case SHOW_CURRENT_INPUT_DEPRECATED: {
                int32_t ret = ShowCurrentInputDeprecated(data);
                reply.WriteInt32(ret);
                break;
            }
            case HIDE_CURRENT_INPUT_DEPRECATED: {
                int32_t ret = HideCurrentInputDeprecated(data);
                reply.WriteInt32(ret);
                break;
            }
            case DISPLAY_OPTIONAL_INPUT_METHOD_DEPRECATED: {
                int ret = DisplayOptionalInputMethodDeprecated(data);
                reply.WriteInt32(ret);
                break;
            }
            case SET_CORE_AND_AGENT_DEPRECATED: {
                SetCoreAndAgentDeprecated(data);
                break;
            }
            default: {
                return BRemoteObject::OnRemoteRequest(code, data, reply, option);
            }
        }
        return NO_ERROR;
    }

    /*! Prepare input
    \n Send prepareInput command to work thread.
        The handling of prepareInput is in the work thread of PerUserSession.
    \see PerUserSession::OnPrepareInput
    \param data the parcel in which the parameters are saved
    */
    void InputMethodSystemAbilityStub::prepareInput(MessageParcel& data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::prepareInput");
        int32_t pid = IPCSkeleton::GetCallingPid();
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId);
        parcel->WriteInt32(pid);
        parcel->WriteInt32(uid);
        parcel->WriteInt32(data.ReadInt32());
        parcel->WriteRemoteObject(data.ReadRemoteObject());
        parcel->WriteRemoteObject(data.ReadRemoteObject());
        parcel->WriteParcelable(data.ReadParcelable<InputAttribute>());

        Message *msg = new Message(MSG_ID_PREPARE_INPUT, parcel);
        MessageHandler::Instance()->SendMessage(msg);
    }

    int32_t InputMethodSystemAbilityStub::displayOptionalInputMethod(MessageParcel& data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::displayOptionalInputMethod");
        if (!CheckPermission(PERMISSION_CONNECT_IME_ABILITY)) {
            IMSA_HILOGE("Permission denied");
            return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
        }
        int32_t pid = IPCSkeleton::GetCallingPid();
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        auto parcel = new (std::nothrow) MessageParcel();
        if (parcel == nullptr) {
            IMSA_HILOGE("parcel is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        parcel->WriteInt32(userId);
        parcel->WriteInt32(pid);
        parcel->WriteInt32(uid);
        auto msg = new Message(MSG_ID_DISPLAY_OPTIONAL_INPUT_METHOD, parcel);
        if (msg == nullptr) {
            IMSA_HILOGE("msg is nullptr");
            delete parcel;
            return ErrorCode::ERROR_NULL_POINTER;
        }
        MessageHandler::Instance()->SendMessage(msg);
        return ErrorCode::NO_ERROR;
    }

    /*! Release input
    \n Send releaseInput command to work thread.
        The handling of releaseInput is in the work thread of PerUserSession.
    \see PerUserSession::OnReleaseInput
    \param data the parcel in which the parameters are saved
    */
    void InputMethodSystemAbilityStub::releaseInput(MessageParcel& data)
    {
        IMSA_HILOGE("InputMethodSystemAbilityStub::releaseInput");
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId);
        parcel->WriteRemoteObject(data.ReadRemoteObject());

        Message *msg = new Message(MSG_ID_RELEASE_INPUT, parcel);
        MessageHandler::Instance()->SendMessage(msg);
    }

    /*! Start input
    \n Send startInput command to work thread.
        The handling of startInput is in the work thread of PerUserSession.
    \see PerUserSession::OnStartInput
    \param data the parcel in which the parameters are saved
    */
    void InputMethodSystemAbilityStub::startInput(MessageParcel& data)
    {
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId);
        parcel->WriteRemoteObject(data.ReadRemoteObject());
        parcel->WriteBool(data.ReadBool());

        Message *msg = new Message(MSG_ID_START_INPUT, parcel);
        MessageHandler::Instance()->SendMessage(msg);
    }

    /*! Stop input
    \n Send stopInput command to work thread.
        The handling of stopInput is in the work thread of PerUserSession.
    \see PerUserSession::OnStopInput
    \param data the parcel in which the parameters are saved
    */
    void InputMethodSystemAbilityStub::stopInput(MessageParcel& data)
    {
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId);
        parcel->WriteRemoteObject(data.ReadRemoteObject());

        Message *msg = new Message(MSG_ID_STOP_INPUT, parcel);
        MessageHandler::Instance()->SendMessage(msg);
    }

        /*! Prepare input
    \n Send prepareInput command to work thread.
        The handling of prepareInput is in the work thread of PerUserSession.
    \see PerUserSession::OnPrepareInput
    \param data the parcel in which the parameters are saved
    */
    void InputMethodSystemAbilityStub::SetCoreAndAgent(MessageParcel& data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::SetCoreAndAgent");
        if (!CheckPermission(PERMISSION_CONNECT_IME_ABILITY)) {
            IMSA_HILOGE("Permission denied");
            return;
        }
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId);
        parcel->WriteRemoteObject(data.ReadRemoteObject());
        parcel->WriteRemoteObject(data.ReadRemoteObject());

        Message *msg = new Message(MSG_ID_SET_CORE_AND_AGENT, parcel);
        MessageHandler::Instance()->SendMessage(msg);
    }

    int32_t InputMethodSystemAbilityStub::HideCurrentInput(MessageParcel &data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::HideCurrentInput");
        if (!CheckPermission(PERMISSION_CONNECT_IME_ABILITY)) {
            IMSA_HILOGE("Permission denied");
            return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
        }
        int userId = getUserId(IPCSkeleton::GetCallingUid());
        return SendMessageToService(
            MSG_HIDE_CURRENT_INPUT, [userId](MessageParcel &parcel) -> bool { return parcel.WriteInt32(userId); });
    }

    int32_t InputMethodSystemAbilityStub::ShowCurrentInput(MessageParcel &data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::ShowCurrentInput");
        if (!CheckPermission(PERMISSION_CONNECT_IME_ABILITY)) {
            IMSA_HILOGE("Permission denied");
            return ErrorCode::ERROR_STATUS_PERMISSION_DENIED;
        }
        int userId = getUserId(IPCSkeleton::GetCallingUid());
        return SendMessageToService(
            MSG_SHOW_CURRENT_INPUT, [userId](MessageParcel &parcel) -> bool { return parcel.WriteInt32(userId); });
    }

    int32_t InputMethodSystemAbilityStub::OnSwitchInputMethod(MessageParcel &data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::OnSwitchInputMethod");
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);

        auto *parcel = new (std::nothrow) MessageParcel();
        if (parcel == nullptr) {
            return ErrorCode::ERROR_EX_NULL_POINTER;
        }
        parcel->WriteInt32(userId);
        auto property = data.ReadParcelable<InputMethodProperty>();
        parcel->WriteParcelable(property);
        delete property;

        auto *msg = new (std::nothrow) Message(MSG_ID_SWITCH_INPUT_METHOD, parcel);
        if (msg == nullptr) {
            delete parcel;
            return ErrorCode::ERROR_EX_NULL_POINTER;
        }
        MessageHandler::Instance()->SendMessage(msg);
        return ErrorCode::NO_ERROR;
    }

    void InputMethodSystemAbilityStub::OnGetCurrentInputMethod(MessageParcel &reply)
    {
        auto property = GetCurrentInputMethod();
        if (property == nullptr) {
            IMSA_HILOGE("InputMethodSystemAbilityStub::OnGetCurrentInputMethod property is nullptr");
            reply.WriteInt32(ErrorCode::ERROR_GETTING_CURRENT_IME);
            return;
        }
        reply.WriteInt32(NO_ERROR);
        reply.WriteParcelable(property.get());
    }

    void InputMethodSystemAbilityStub::OnListInputMethod(MessageParcel &data, MessageParcel &reply)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::OnListInputMethod");
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        uint32_t status = data.ReadUint32();
        const auto &properties = ListInputMethodByUserId(userId, InputMethodStatus(status));
        reply.WriteInt32(NO_ERROR);
        uint32_t size = properties.size();
        reply.WriteUint32(size);
        for (const auto &property : properties) {
            reply.WriteParcelable(&property);
        }
    }

    int32_t InputMethodSystemAbilityStub::ShowCurrentInputDeprecated(MessageParcel &data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::ShowCurrentInputDeprecated");
        int userId = getUserId(IPCSkeleton::GetCallingUid());
        return SendMessageToService(
            MSG_SHOW_CURRENT_INPUT, [userId](MessageParcel &parcel) -> bool { return parcel.WriteInt32(userId); });
    }

    int32_t InputMethodSystemAbilityStub::HideCurrentInputDeprecated(MessageParcel &data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::HideCurrentInputDeprecated");
        int userId = getUserId(IPCSkeleton::GetCallingUid());
        return SendMessageToService(
            MSG_HIDE_CURRENT_INPUT, [userId](MessageParcel &parcel) -> bool { return parcel.WriteInt32(userId); });
    }

    int32_t InputMethodSystemAbilityStub::DisplayOptionalInputMethodDeprecated(MessageParcel &data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::DisplayOptionalInputMethodDeprecated");
        int32_t pid = IPCSkeleton::GetCallingPid();
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        return SendMessageToService(
            MSG_ID_DISPLAY_OPTIONAL_INPUT_METHOD, [pid, uid, userId](MessageParcel &parcel) -> bool {
                return parcel.WriteInt32(userId) && parcel.WriteInt32(pid) && parcel.WriteInt32(uid);
            });
    }

    void InputMethodSystemAbilityStub::SetCoreAndAgentDeprecated(MessageParcel &data)
    {
        IMSA_HILOGI("InputMethodSystemAbilityStub::SetCoreAndAgentDeprecated");
        int32_t userId = getUserId(IPCSkeleton::GetCallingUid());
        int32_t ret = SendMessageToService(MSG_ID_SET_CORE_AND_AGENT, [userId, &data](MessageParcel &parcel) -> bool {
            return parcel.WriteInt32(userId) && parcel.WriteRemoteObject(data.ReadRemoteObject())
                   && parcel.WriteRemoteObject(data.ReadRemoteObject());
        });
        if (ret != NO_ERROR) {
            IMSA_HILOGE("send message to service failed: %{public}s", ErrorCode::ToString(ret));
        }
    }

    bool InputMethodSystemAbilityStub::CheckPermission(const std::string &permission)
    {
        IMSA_HILOGI("Check Permission");
        AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
        TypeATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
        if (tokenType == TOKEN_INVALID) {
            IMSA_HILOGE("invalid token id %{public}d", tokenId);
            return false;
        }
        int result = AccessTokenKit::VerifyAccessToken(tokenId, permission);
        if (result != PERMISSION_GRANTED) {
            IMSA_HILOGE("grant failed, result: %{public}d", result);
        }
        return result == PERMISSION_GRANTED;
    }

    int32_t InputMethodSystemAbilityStub::SendMessageToService(
        int32_t code, std::function<bool(MessageParcel &)> callback)
    {
        auto parcel = new (std::nothrow) MessageParcel();
        if (parcel == nullptr) {
            IMSA_HILOGE("parcel is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        bool ret = callback(*parcel);
        if (!ret) {
            IMSA_HILOGE("failed to write parcel");
            delete parcel;
            return ErrorCode::ERROR_STATUS_FAILED_TRANSACTION;
        }
        auto msg = new Message(code, parcel);
        if (msg == nullptr) {
            IMSA_HILOGE("msg is nullptr");
            delete parcel;
            return ErrorCode::ERROR_NULL_POINTER;
        }
        MessageHandler::Instance()->SendMessage(msg);
        return ErrorCode::NO_ERROR;
    }

    /*! Get user id from uid
    \param uid the uid from which the remote call is
    \return return user id of the remote caller
    */
    int32_t InputMethodSystemAbilityStub::getUserId(int32_t uid)
    {
        return uid / USER_ID_CHANGE_VALUE;
    }
} // namespace MiscServices
} // namespace OHOS
