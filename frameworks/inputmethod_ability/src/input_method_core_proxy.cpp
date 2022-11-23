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

#include "input_method_core_proxy.h"

#include <string_ex.h>

#include "input_attribute.h"
#include "itypes_util.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
    InputMethodCoreProxy::InputMethodCoreProxy(const OHOS::sptr<OHOS::IRemoteObject> &impl)
        : IRemoteProxy<IInputMethodCore>(impl)
    {
    }

    InputMethodCoreProxy::~InputMethodCoreProxy() = default;

    int32_t InputMethodCoreProxy::initializeInput(sptr<IRemoteObject> &startInputToken, int32_t displayId,
                                                  sptr<IInputControlChannel> &inputControlChannel)
    {
        IMSA_HILOGD("InputMethodCoreProxy::initializeInput");
        auto remote = Remote();
        if (!remote) {
            IMSA_HILOGI("remote is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        if (!startInputToken) {
            IMSA_HILOGI("InputMethodCoreProxy::initializeInput startInputToken is nullptr");
        }

        if (!inputControlChannel) {
            IMSA_HILOGI("InputMethodCoreProxy::initializeInput inputControlChannel is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        IMSA_HILOGI("InputMethodCoreProxy::initializeInput displayId = %{public}d", displayId);
        MessageParcel data, reply;
        data.WriteInterfaceToken(GetDescriptor());
        data.WriteInt32(displayId);
        sptr<IRemoteObject> channelObject = inputControlChannel->AsObject();
        if (!channelObject) {
            IMSA_HILOGI("InputMethodCoreProxy::initializeInput channelObject is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        bool wor = data.WriteRemoteObject(channelObject);
        if (wor) {
            IMSA_HILOGI("InputMethodCoreProxy::initializeInput Success to write inputControlChannel");
        } else {
            IMSA_HILOGI("InputMethodCoreProxy::initializeInput Failed to write inputControlChannel");
        }
        MessageOption option {
            MessageOption::TF_SYNC
        };
        int32_t status = remote->SendRequest(INITIALIZE_INPUT, data, reply, option);
        if (status != ErrorCode::NO_ERROR) {
            IMSA_HILOGI("InputMethodCoreProxy::initializeInput status = %{public}d", status);
            return status;
        }
        int32_t code = reply.ReadException();
        return code;
    }

    int32_t InputMethodCoreProxy::InitInputControlChannel(sptr<IInputControlChannel> &inputControlChannel)
    {
        IMSA_HILOGD("InputMethodCoreProxy::InitInputControlChannel");
        auto remote = Remote();
        if (!remote) {
            IMSA_HILOGI("remote is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        if (!inputControlChannel) {
            IMSA_HILOGI("InputMethodCoreProxy::InitInputControlChannel inputControlChannel is nullptr");
            return ErrorCode::ERROR_EX_NULL_POINTER;
        }
        MessageParcel data, reply;
        data.WriteInterfaceToken(GetDescriptor());
        sptr<IRemoteObject> channelObject = inputControlChannel->AsObject();
        if (!channelObject) {
            IMSA_HILOGI("InputMethodCoreProxy::InitInputControlChannel channelObject is nullptr");
            return ErrorCode::ERROR_EX_NULL_POINTER;
        }
        data.WriteRemoteObject(channelObject);
        MessageOption option {
            MessageOption::TF_SYNC
        };
        int32_t status = remote->SendRequest(INIT_INPUT_CONTROL_CHANNEL, data, reply, option);
        if (status != ErrorCode::NO_ERROR) {
            IMSA_HILOGI("InputMethodCoreProxy::InitInputControlChannel status = %{public}d", status);
            return status;
        }
        int32_t code = reply.ReadException();
        return code;
    }

    void InputMethodCoreProxy::SetClientState(bool state)
    {
    IMSA_HILOGD("InputMethodCoreProxy::SetClientState");
        auto remote = Remote();
        if (!remote) {
            IMSA_HILOGI("remote is nullptr");
            return;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(GetDescriptor())
            && data.WriteBool(state))) {
            IMSA_HILOGI("InputMethodCoreProxy::SetClientState write error");
            return;
        }
        MessageParcel reply;
        MessageOption option {
            MessageOption::TF_SYNC
        };

        int32_t status = remote->SendRequest(SET_CLIENT_STATE, data, reply, option);
        if (status != ErrorCode::NO_ERROR) {
            IMSA_HILOGI("InputMethodCoreProxy::SetClientState status = %{public}d", status);
        }
    }
    bool InputMethodCoreProxy::startInput(const sptr<IInputDataChannel> &inputDataChannel,
                                          const InputAttribute& editorAttribute, bool supportPhysicalKbd)
    {
        IMSA_HILOGD("InputMethodCoreProxy::startInput");
        auto remote = Remote();
        if (!remote) {
            IMSA_HILOGI("remote is nullptr");
            return false;
        }
        if (!inputDataChannel) {
            IMSA_HILOGI("InputMethodCoreProxy::startInput inputDataChannel is nullptr");
            return false;
        }

        MessageParcel data;
        if (!(data.WriteInterfaceToken(GetDescriptor())
            && data.WriteRemoteObject(inputDataChannel->AsObject())
            && InputAttribute::Marshalling(editorAttribute, data)
            && data.WriteBool(supportPhysicalKbd))) {
            IMSA_HILOGI("InputMethodCoreProxy::startInput write error");
            return false;
        }
        MessageParcel reply;
        MessageOption option {
            MessageOption::TF_SYNC
        };

        int32_t status = remote->SendRequest(START_INPUT, data, reply, option);
        if (status != ErrorCode::NO_ERROR) {
            IMSA_HILOGI("InputMethodCoreProxy::startInput status = %{public}d", status);
            return false;
        }

        int32_t code = reply.ReadException();
        return code == ErrorCode::NO_ERROR;
    }

    int32_t InputMethodCoreProxy::stopInput()
    {
        IMSA_HILOGD("InputMethodCoreProxy::stopInput");
        auto remote = Remote();
        if (!remote) {
            IMSA_HILOGI("remote is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        MessageParcel data, reply;
        data.WriteInterfaceToken(GetDescriptor());
        MessageOption option {
            MessageOption::TF_SYNC
        };
        int32_t status = remote->SendRequest(STOP_INPUT, data, reply, option);
        if (status != ErrorCode::NO_ERROR) {
            IMSA_HILOGI("InputMethodCoreProxy::stopInput status = %{public}d", status);
            return status;
        }
        int code = reply.ReadException();
        if (code != ErrorCode::NO_ERROR) {
            IMSA_HILOGI("InputMethodCoreProxy::stopInput code = %{public}d", code);
            return code;
        }
        return reply.ReadInt32();
    }

    int32_t InputMethodCoreProxy::showKeyboard(
        const sptr<IInputDataChannel> &inputDataChannel, bool isShowKeyboard, const SubProperty &subProperty)
    {
        IMSA_HILOGD("InputMethodCoreProxy::showKeyboard");
        return SendRequest(SHOW_KEYBOARD, [&inputDataChannel, &isShowKeyboard, &subProperty](MessageParcel &data) {
            return ITypesUtil::Marshal(data, inputDataChannel->AsObject(), isShowKeyboard, subProperty);
        });
    }

    void InputMethodCoreProxy::StopInputService(std::string imeId)
    {
        IMSA_HILOGD("InputMethodCoreProxy::StopInputService");
        auto remote = Remote();
        if (!remote) {
            IMSA_HILOGI("InputMethodCoreProxy::StopInputService remote is nullptr");
            return;
        }
        MessageParcel data;
        if (!(data.WriteInterfaceToken(GetDescriptor())
            && data.WriteString16(Str8ToStr16(imeId)))) {
            return;
        }
        MessageParcel reply;
        MessageOption option {
            MessageOption::TF_SYNC
        };

        int32_t res = remote->SendRequest(STOP_INPUT_SERVICE, data, reply, option);
        if (res != ErrorCode::NO_ERROR) {
            return;
        }
    }

    bool InputMethodCoreProxy::hideKeyboard(int32_t flags)
    {
        IMSA_HILOGD("InputMethodCoreProxy::hideKeyboard");
        auto remote = Remote();
        if (!remote) {
            return false;
        }

        MessageParcel data;
        if (!(data.WriteInterfaceToken(GetDescriptor()) && data.WriteInt32(flags))) {
            return false;
        }
        MessageParcel reply;
        MessageOption option {
            MessageOption::TF_SYNC
        };

        int32_t res = remote->SendRequest(HIDE_KEYBOARD, data, reply, option);
        if (res != ErrorCode::NO_ERROR) {
            return false;
        }
        return true;
    }

    int32_t InputMethodCoreProxy::setKeyboardType(const KeyboardType& type)
    {
        IMSA_HILOGD("InputMethodCoreProxy::setKeyboardType");
        auto remote = Remote();
        if (!remote) {
            IMSA_HILOGI("remote is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        MessageParcel data, reply;
        data.WriteInterfaceToken(GetDescriptor());
        data.WriteParcelable(&type);
        MessageOption option {
            MessageOption::TF_SYNC
        };
        int32_t status = remote->SendRequest(SET_KEYBOARD_TYPE, data, reply, option);
        if (status != ErrorCode::NO_ERROR) {
            return status;
        }
        int32_t code = reply.ReadException();
        return code;
    }

    int32_t InputMethodCoreProxy::getKeyboardWindowHeight(int32_t &retHeight)
    {
        IMSA_HILOGD("InputMethodCoreProxy::getKeyboardWindowHeight");
        auto remote = Remote();
        if (!remote) {
            IMSA_HILOGI("remote is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        MessageParcel data, reply;
        data.WriteInterfaceToken(GetDescriptor());
        MessageOption option {
            MessageOption::TF_SYNC
        };
        int32_t status = remote->SendRequest(GET_KEYBOARD_WINDOW_HEIGHT, data, reply, option);
        if (status != ErrorCode::NO_ERROR) {
            return status;
        }
        int32_t code = reply.ReadException();
        if (code) {
            return code;
        }
        retHeight = reply.ReadInt32();
        return ErrorCode::NO_ERROR;
    }

    int32_t InputMethodCoreProxy::SetSubtype(const SubProperty &property)
    {
	    IMSA_HILOGD("InputMethodCoreProxy::SetSubtype");
        return SendRequest(
            SET_SUBTYPE, [&property](MessageParcel &data) { return ITypesUtil::Marshal(data, property); });
    }

    int32_t InputMethodCoreProxy::SendRequest(int code, ParcelHandler input, ParcelHandler output)
    {
        IMSA_HILOGD("InputMethodCoreProxy::%{public}s in", __func__);
        MessageParcel data;
        MessageParcel reply;
        MessageOption option{ MessageOption::TF_SYNC };
        if (!data.WriteInterfaceToken(GetDescriptor())) {
            IMSA_HILOGE("InputMethodCoreProxy::write interface token failed");
            return ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT;
        }
        if (input != nullptr && (!input(data))) {
            IMSA_HILOGE("InputMethodCoreProxy::write data failed");
            return ErrorCode::ERROR_EX_PARCELABLE;
        }
        auto ret = Remote()->SendRequest(code, data, reply, option);
        if (ret != NO_ERROR) {
            IMSA_HILOGE("InputMethodCoreProxy::SendRequest failed, ret %{public}d", ret);
            return ret;
        }
        ret = reply.ReadInt32();
        if (ret != NO_ERROR) {
            IMSA_HILOGE("InputMethodCoreProxy::reply error, ret %{public}d", ret);
            return ret;
        }
        if (output != nullptr && (!output(reply))) {
            IMSA_HILOGE("InputMethodCoreProxy::reply parcel error");
            return ErrorCode::ERROR_EX_PARCELABLE;
        }
        return ret;
    }
} // namespace MiscServices
} // namespace OHOS
