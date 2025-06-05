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
constexpr std::size_t ASYNC_UNANSWERED_MAX_NUMBER = 1000;

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
    {
        std::lock_guard<std::mutex> lock(channelMutex_);
        channel_ = nullptr;
    }
    ClearRspHandlers();
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
    SyncOutPut output = nullptr;
    if (callback == nullptr) {
        output = [&text](const ResponseInfo &rspInfo) -> int32_t {
            if (rspInfo.dealRet_ == ErrorCode::NO_ERROR) {
                VariantUtil::GetValue(rspInfo.data_, text);
            }
            return rspInfo.dealRet_;
        };
    }

    return Request(callback, work, !callback, output);
}

int32_t InputDataChannelProxyWrap::GetTextAfterCursor(int32_t number, std::string &text, AsyncIpcCallBack callback)
{
    auto work = [number, text](uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel) -> int32_t {
        return channel->GetTextAfterCursor(number, msgId);
    };
    SyncOutPut output = nullptr;
    if (callback == nullptr) {
        output = [&text](const ResponseInfo &rspInfo) -> int32_t {
            if (rspInfo.dealRet_ == ErrorCode::NO_ERROR) {
                VariantUtil::GetValue(rspInfo.data_, text);
            }
            return rspInfo.dealRet_;
        };
    }

    return Request(callback, work, !callback, output);
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
    SyncOutPut output = nullptr;
    if (callback == nullptr) {
        output = [&index](const ResponseInfo &rspInfo) -> int32_t {
            if (rspInfo.dealRet_ == ErrorCode::NO_ERROR) {
                VariantUtil::GetValue(rspInfo.data_, index);
            }
            return rspInfo.dealRet_;
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
    SyncOutPut output = nullptr;
    if (callback == nullptr) {
        output = [&textConfig](const ResponseInfo &rspInfo) -> int32_t {
            if (rspInfo.dealRet_ == ErrorCode::NO_ERROR) {
                VariantUtil::GetValue(rspInfo.data_, textConfig);
            }
            return rspInfo.dealRet_;
        };
    }

    return Request(callback, work, !callback, output);
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

int32_t InputDataChannelProxyWrap::Request(
    AsyncIpcCallBack callback, ChannelWork work, bool isSync, SyncOutPut output)
{
    auto channel = GetDataChannel();
    if (channel == nullptr) {
        IMSA_HILOGE("data channel is nullptr!");
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    std::shared_ptr<ResponseHandler> handler;
    int32_t ret = AddRspHandler(handler, callback, isSync);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("BeforeRequest error: %{public}d.", ret);
        return ret;
    }
    ret = work(handler->msgId_, channel);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("work error: %{public}d.", ret);
        DeleteRspHandler(handler->msgId_);
        return ret;
    }
    if (handler != nullptr && handler->syncBlockData_ != nullptr) {
        ret = WaitResponse(handler, output);
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

int32_t InputDataChannelProxyWrap::AddRspHandler(
    std::shared_ptr<ResponseHandler> &handler, AsyncIpcCallBack callBack, bool isSync)
{
    std::lock_guard<std::mutex> lock(rspMutex_);
    if (rspHandlers_.size() >= ASYNC_UNANSWERED_MAX_NUMBER) {
        uint64_t msgId = rspHandlers_.begin()->first;
        ResponseInfo rspInfo = { ErrorCode::ERROR_RESPONSE_TIMEOUT, std::monostate{} };
        HandleMsg(msgId, rspInfo);
        IMSA_HILOGW("data channel, too many unanswered msgid:%{public}" PRIu64 "", msgId);
    }
    handler = std::make_shared<ResponseHandler>(isSync, GetMsgId(), callBack);
    rspHandlers_.insert({ handler->msgId_, handler });
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelProxyWrap::ClearRspHandlers()
{
    std::lock_guard<std::mutex> lock(rspMutex_);
    ResponseInfo rspInfo = { ErrorCode::ERROR_CLIENT_NULL_POINTER, std::monostate{} };
    for (const auto &handler : rspHandlers_) {
        if (handler.second == nullptr) {
            continue;
        }
        if (handler.second->syncBlockData_ != nullptr) {
            handler.second->syncBlockData_->SetValue(rspInfo);
            continue;
        }
        if (handler.second->callBack_ != nullptr) {
            handler.second->callBack_(rspInfo.dealRet_, rspInfo.data_);
        }
    }
    rspHandlers_.clear();
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelProxyWrap::HandleMsg(const uint64_t msgId, const ResponseInfo &rspInfo)
{
    auto it = rspHandlers_.find(msgId);
    if (it == rspHandlers_.end()) {
        return ErrorCode::NO_ERROR;
    }
    do {
        if (it->second->syncBlockData_ != nullptr) {
            it->second->syncBlockData_->SetValue(rspInfo);
            break;
        }
        if (it->second->callBack_ == nullptr) {
            break;
        }
        it->second->callBack_(rspInfo.dealRet_, rspInfo.data_);
    } while(false);
    rspHandlers_.erase(it);
    return ErrorCode::NO_ERROR;
}

int32_t InputDataChannelProxyWrap::HandleResponse(const uint64_t msgId, int32_t errorCode, const ResponseData &reply)
{
    ResponseInfo rspInfo = {errorCode, reply};
    std::lock_guard<std::mutex> lock(rspMutex_);
    return HandleMsg(msgId, rspInfo);
}

int32_t InputDataChannelProxyWrap::WaitResponse(std::shared_ptr<ResponseHandler> handler, SyncOutPut output)
{
    if (handler->syncBlockData_ == nullptr) {
        return ErrorCode::ERROR_CLIENT_NULL_POINTER;
    }
    ResponseInfo rspInfo;
    if (!handler->syncBlockData_->GetValue(rspInfo)) {
        return ErrorCode::ERROR_RESPONSE_TIMEOUT;
    }
    return output(rspInfo);
}

int32_t InputDataChannelProxyWrap::DeleteRspHandler(const uint64_t msgId)
{
    std::lock_guard<std::mutex> lock(rspMutex_);
    auto it = rspHandlers_.find(msgId);
    if (it != rspHandlers_.end()) {
        rspHandlers_.erase(it);
    }
    return ErrorCode::NO_ERROR;
}
} // namespace MiscServices
} // namespace OHOS