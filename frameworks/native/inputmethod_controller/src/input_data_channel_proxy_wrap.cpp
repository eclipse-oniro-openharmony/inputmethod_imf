/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "global.h"
#include <string>
#include "string_ex.h"
#include "input_data_channel_proxy_wrap.h"
#include "input_method_tools.h"
#include "variant_util.h"
#include <cinttypes>

namespace OHOS {
namespace MiscServices {
constexpr std::size_t ASYNC_UNANSWERED_MAX_NUMBER = 50;
constexpr uint32_t ASYNC_REPLY_TIMEOUT = 100; // unit ms

InputDataChannelProxyWrap::InputDataChannelProxyWrap(std::shared_ptr<InputDataChannelProxy> channel)
{
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    msgId_ = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    msgId_ = msgId_ ? msgId_ : ++msgId_;

    channel_ = channel;
}
InputDataChannelProxyWrap::~InputDataChannelProxyWrap()
{
    std::lock_guard<std::mutex> lock(channelMutex_);
    if (channel_ != nullptr) {
        channel_ = nullptr;
    }
}

int32_t InputDataChannelProxyWrap::InsertText(const std::string &text, bool isSync, AsyncIpcCallBack callback)
{
    auto work = [text](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->InsertText(text, msgId);
    };
    return Request(callback, work, isSync);
}

int32_t InputDataChannelProxyWrap::DeleteForward(int32_t length, AsyncIpcCallBack callback)
{
    auto work = [length](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->DeleteForward(length, msgId);
    };
    return Request(callback, work, !callback);
}

int32_t InputDataChannelProxyWrap::DeleteBackward(int32_t length, AsyncIpcCallBack callback)
{
    auto work = [length](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->DeleteBackward(length, msgId);
    };
    return Request(callback, work, !callback);
}

int32_t InputDataChannelProxyWrap::GetTextBeforeCursor(int32_t number, std::string &text, AsyncIpcCallBack callback)
{
    auto work = [number](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->GetTextBeforeCursor(number, msgId);
    };
    ChannelOutPut output = nullptr;
    if (callback == nullptr) {
        output = [&text](const ResponseData &data) -> int32_t {
            return VariantUtil::GetValue(data, text) ?
                ErrorCode::NO_ERROR : ErrorCode::ERROR_PARSE_PARAMETER_FAILED;
        };
    }

    return Request(callback, work, !callback, output);
}

int32_t InputDataChannelProxyWrap::GetTextAfterCursor(int32_t number, std::string &text, AsyncIpcCallBack callback)
{
    auto work = [number, text](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->GetTextAfterCursor(number, msgId);
    };
    ChannelOutPut output = nullptr;
    if (callback == nullptr) {
        output = [&text](const ResponseData &data) -> int32_t {
            return VariantUtil::GetValue(data, text) ?
                ErrorCode::NO_ERROR : ErrorCode::ERROR_PARSE_PARAMETER_FAILED;
        };
    }

    return Request(callback, work, !callback, output);
}
void InputDataChannelProxyWrap::SendKeyboardStatus(int32_t status, bool isSync, AsyncIpcCallBack callback)
{
    auto work = [status](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->SendKeyboardStatus(status, msgId);
    };
    Request(callback, work, isSync);
}

int32_t InputDataChannelProxyWrap::SendFunctionKey(int32_t funcKey, AsyncIpcCallBack callback)
{
    auto work = [funcKey](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->SendFunctionKey(funcKey, msgId);
    };
    return Request(callback, work, !callback);
}

int32_t InputDataChannelProxyWrap::MoveCursor(int32_t keyCode, AsyncIpcCallBack callback)
{
    auto work = [keyCode](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->MoveCursor(keyCode, msgId);
    };
    return Request(callback, work, !callback);
}

int32_t InputDataChannelProxyWrap::GetEnterKeyType(int32_t &keyType, AsyncIpcCallBack callback)
{
    auto work = [](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->GetEnterKeyType(msgId);
    };
    ChannelOutPut output = nullptr;
    if (callback == nullptr) {
        output = [&keyType](const ResponseData &data) -> int32_t {
            return VariantUtil::GetValue(data, keyType) ?
                ErrorCode::NO_ERROR : ErrorCode::ERROR_PARSE_PARAMETER_FAILED;
        };
    }

    return Request(callback, work, !callback, output);
}

int32_t InputDataChannelProxyWrap::GetInputPattern(int32_t &inputPattern, AsyncIpcCallBack callback)
{
    auto work = [](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->GetInputPattern(msgId);
    };
    ChannelOutPut output = nullptr;
    if (callback == nullptr) {
        output = [&inputPattern](const ResponseData &data) -> int32_t {
            return VariantUtil::GetValue(data, inputPattern) ?
                ErrorCode::NO_ERROR : ErrorCode::ERROR_PARSE_PARAMETER_FAILED;
        };
    }

    return Request(callback, work, !callback, output);
}

int32_t InputDataChannelProxyWrap::SelectByRange(int32_t start, int32_t end, AsyncIpcCallBack callback)
{
    auto work = [start, end](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->SelectByRange(start, end, msgId);
    };
    return Request(callback, work, !callback);
}

int32_t InputDataChannelProxyWrap::SelectByMovement(int32_t direction, int32_t cursorMoveSkip,
    AsyncIpcCallBack callback)
{
    auto work = [direction, cursorMoveSkip](
        uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->SelectByMovement(direction, cursorMoveSkip, msgId);
    };
    return Request(callback, work, !callback);
}

int32_t InputDataChannelProxyWrap::HandleExtendAction(int32_t action, AsyncIpcCallBack callback)
{
    auto work = [action](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->HandleExtendAction(action, msgId);
    };
    return Request(callback, work, !callback);
}

int32_t InputDataChannelProxyWrap::GetTextIndexAtCursor(int32_t &index, AsyncIpcCallBack callback)
{
    auto work = [](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->GetTextIndexAtCursor(msgId);
    };
    ChannelOutPut output = nullptr;
    if (callback == nullptr) {
        output = [&index](const ResponseData &data) -> int32_t {
            return VariantUtil::GetValue(data, index) ?
                ErrorCode::NO_ERROR : ErrorCode::ERROR_PARSE_PARAMETER_FAILED;
        };
    }

    return Request(callback, work, !callback, output);
}

int32_t InputDataChannelProxyWrap::GetTextConfig(TextTotalConfig &textConfig, AsyncIpcCallBack callback, bool ipcSync)
{
    auto channel = GetDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("data channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    int32_t ret = 0;
    TextTotalConfigInner inner = InputMethodTools::GetInstance().TextTotalConfigToInner(textConfig);
    if (ipcSync) {
        ret = channel->GetTextConfigSync(inner);
        if (ret == ErrorCode::NO_ERROR) {
            textConfig = InputMethodTools::GetInstance().InnerToTextTotalConfig(inner);
        }
        return ret;
    }

    auto work = [inner](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->GetTextConfig(inner, msgId);
    };
    ChannelOutPut output = nullptr;
    if (callback == nullptr) {
        output = [&textConfig](const ResponseData &data) -> int32_t {
            return VariantUtil::GetValue(data, textConfig) ?
                ErrorCode::NO_ERROR : ErrorCode::ERROR_PARSE_PARAMETER_FAILED;
        };
    }

    return Request(callback, work, !callback, output);
}

void InputDataChannelProxyWrap::NotifyPanelStatusInfo(const PanelStatusInfoInner &info, bool isSync,
    AsyncIpcCallBack callback)
{
    auto work = [info](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->NotifyPanelStatusInfo(info, msgId);
    };
    Request(callback, work, isSync);
}

void InputDataChannelProxyWrap::NotifyKeyboardHeight(uint32_t height, bool isSync, AsyncIpcCallBack callback)
{
    auto work = [height](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->NotifyKeyboardHeight(height, msgId);
    };
    Request(callback, work, isSync);
}

int32_t InputDataChannelProxyWrap::SendPrivateCommand(const Value &value, AsyncIpcCallBack callback)
{
    auto work = [value](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->SendPrivateCommand(value, msgId);
    };
    return Request(callback, work, !callback);
}

int32_t InputDataChannelProxyWrap::SetPreviewText(
    const std::string &text, const RangeInner &range, AsyncIpcCallBack callback)
{
    auto work = [text, range](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->SetPreviewText(text, range, msgId);
    };
    return Request(callback, work, !callback);
}

int32_t InputDataChannelProxyWrap::FinishTextPreview(bool isAsync, AsyncIpcCallBack callback)
{
    auto work = [](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->FinishTextPreview(msgId);
    };
    return Request(callback, work, !isAsync);
}

int32_t InputDataChannelProxyWrap::SendMessage(const ArrayBuffer &arraybuffer, AsyncIpcCallBack callback)
{
    auto work = [arraybuffer](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->SendMessage(arraybuffer, msgId);
    };
    return Request(callback, work, !callback);
}

int32_t InputDataChannelProxyWrap::Request(
    AsyncIpcCallBack callback, ChannelWork work, bool isSync, ChannelOutPut output)
{
    auto channel = GetDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("data channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    uint64_t msgId = 0;
    int32_t ret = BeforeRequest(msgId, callback, isSync);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("BeforeRequest error: %{public}d.", ret);
        return ret;
    }
    ret = work(msgId, channel);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("work error: %{public}d.", ret);
        DelMsg(msgId);
        return ret;
    }
    if (isSync) {
        int32_t errCode = 0;
        ResponseData rspData = std::monostate{};
        ret = WaitResponse(msgId, ASYNC_REPLY_TIMEOUT, errCode, rspData);
        if (ret == ErrorCode::NO_ERROR) {
            ret = errCode;
            if (output != nullptr) {
                ret = output(rspData);
            }
        }
    }
    return ret;
}

std::shared_ptr<InputDataChannelProxy> InputDataChannelProxyWrap::GetDataChannel()
{
    std::lock_guard<std::mutex> lock(channelMutex_);
    return channel_;
}

uint64_t InputDataChannelProxyWrap::GetMsgId()
{
    return ++msgId_ ? msgId_ : ++msgId_;
}

int32_t InputDataChannelProxyWrap::BeforeRequest(uint64_t &msgId, AsyncIpcCallBack callBack, bool isSync)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    if (msgList_.size() >= ASYNC_UNANSWERED_MAX_NUMBER) {
        IMSA_HILOGW("async data channel, too many unanswered msg!");
        return ErrorCode::ERROR_TOO_MANY_UNANSWERED_MESSAGE;
    }
    MsgInfo msgInfo(isSync, GetMsgId(), callBack);
    msgList_.insert({ msgInfo.msgId, msgInfo });
    msgId = msgInfo.msgId;
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelProxyWrap::HandleMsg(
    const uint64_t msgId, int32_t code, const ResponseData &data, int32_t defErrCode)
{
    for (auto it = msgList_.begin(); it != msgList_.end();) {
        if (it->first > msgId) {
            break;
        }
        int32_t currentCode = defErrCode;
        ResponseData currentData = std::monostate{};
        if (it->first == msgId) {
            currentCode = code;
            currentData = data;
        }
        if (it->second.isSync) {
            std::unique_lock<std::mutex> lock(rspMutex_);
            rspMsgId_ = it->first;
            rspData_ = currentData;
            rspErrorCode_ = currentCode;
            it->second.cv->notify_one();
        } else {
            if (it->second.callBack) {
                it->second.callBack(currentCode, currentData);
            }
        }
        it = msgList_.erase(it);
    }
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelProxyWrap::ClearMsg(bool isNotify)
{
    IMSA_HILOGI("start ClearMsg");
    std::lock_guard<std::mutex> lock(dataMutex_);
    if (msgList_.empty()) {
        return ErrorCode::NO_ERROR;
    }
    if (isNotify) {
        int32_t errCode = ErrorCode::ERROR_CLIENT_NULL_POINTER;
        ResponseData data = std::monostate{};
        return HandleMsg(msgList_.rbegin()->first, errCode, data, errCode);
    }
    msgList_.clear();
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelProxyWrap::OnResponse(const uint64_t msgId, int32_t errorCode, const ResponseData &reply)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    return HandleMsg(msgId, errorCode, reply, ErrorCode::ERROR_RESPONSE_TIMEOUT);
}

int32_t InputDataChannelProxyWrap::WaitResponse(const uint64_t msgId, uint32_t timeout, int32_t &errorCode,
    ResponseData &data)
{
    {
        std::unique_lock<std::mutex> lock(rspMutex_);
        auto cv = msgList_.find(msgId);
        if (cv == msgList_.end() || cv->second.cv == nullptr) {
            return ErrorCode::ERROR_CLIENT_NULL_POINTER;
        }
        auto ret = cv->second.cv->wait_for(lock, std::chrono::milliseconds(timeout));
        if (ret == std::cv_status::no_timeout) {
            errorCode = rspErrorCode_;
            data = rspData_;
            return ErrorCode::NO_ERROR;
        }
    }
    DelMsg(msgId);
    return ErrorCode::ERROR_RESPONSE_TIMEOUT;
}

int32_t InputDataChannelProxyWrap::DelMsg(const uint64_t msgId)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    auto it = msgList_.find(msgId);
    if (it != msgList_.end()) {
        msgList_.erase(it);
    }
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS