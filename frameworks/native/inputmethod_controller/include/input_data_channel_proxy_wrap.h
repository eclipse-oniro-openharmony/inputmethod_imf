/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_PROXY_WRAP_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_PROXY_WRAP_H
#include <map>
#include <mutex>
#include <variant>
#include <memory>
#include <condition_variable>
#include "input_data_channel_proxy.h"
#include "input_method_utils.h"
#include "block_data.h"

namespace OHOS {
namespace MiscServices {
using AsyncIpcCallBack = std::function<void(int32_t, const ResponseData&)>;
struct ResponseInfo {
    int32_t dealRet_{ ErrorCode::NO_ERROR };
    ResponseData data_{std::monostate{}};
};
struct ResponseHandler  {
    static constexpr uint32_t SYNC_REPLY_TIMEOUT = 3000; // unit ms
    bool isSync_ = false;
    uint64_t msgId_ = 0;
    AsyncIpcCallBack callBack_ = nullptr;
    std::shared_ptr<BlockData<ResponseInfo>> syncBlockData_ = nullptr;
    ResponseHandler(bool isSync, uint64_t msgId, AsyncIpcCallBack callBack)
    {
        isSync_ = isSync;
        msgId_ = msgId;
        callBack_ = callBack;
        if (isSync) {
            syncBlockData_ = std::make_shared<BlockData<ResponseInfo>>(SYNC_REPLY_TIMEOUT);
        }
    }
};
using ChannelWork = std::function<int32_t(uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel)>;
using SyncOutPut = std::function<int32_t(const ResponseInfo&)>;
class InputDataChannelProxyWrap {
public:

    InputDataChannelProxyWrap(std::shared_ptr<InputDataChannelProxy> channel);
    ~InputDataChannelProxyWrap();

    int32_t InsertText(const std::string &text, bool isSync = false, AsyncIpcCallBack callback = nullptr);
    int32_t DeleteForward(int32_t length, AsyncIpcCallBack callback = nullptr);
    int32_t DeleteBackward(int32_t length, AsyncIpcCallBack callback = nullptr);
    int32_t GetTextBeforeCursor(int32_t number, std::string &text, AsyncIpcCallBack callback = nullptr);
    int32_t GetTextAfterCursor(int32_t number, std::string &text, AsyncIpcCallBack callback = nullptr);
    int32_t SendFunctionKey(int32_t funcKey, AsyncIpcCallBack callback = nullptr);
    int32_t MoveCursor(int32_t keyCode, AsyncIpcCallBack callback = nullptr);
    int32_t SelectByRange(int32_t start, int32_t end, AsyncIpcCallBack callback = nullptr);
    int32_t SelectByMovement(int32_t direction, int32_t cursorMoveSkip, AsyncIpcCallBack callback = nullptr);
    int32_t HandleExtendAction(int32_t action, AsyncIpcCallBack callback = nullptr);
    int32_t GetTextIndexAtCursor(int32_t &index, AsyncIpcCallBack callback = nullptr);
    int32_t GetTextConfig(TextTotalConfig &textConfig, AsyncIpcCallBack callback = nullptr, bool ipcSync = false);
    int32_t SendPrivateCommand(const Value &value, AsyncIpcCallBack callback = nullptr);
    int32_t SetPreviewText(const std::string &text, const RangeInner &range, AsyncIpcCallBack callback = nullptr);
    int32_t FinishTextPreview(bool isAsync, AsyncIpcCallBack callback = nullptr);

public:
    int32_t HandleResponse(const uint64_t msgId, int32_t errorCode, const ResponseData &reply);
    std::shared_ptr<InputDataChannelProxy> GetDataChannel();

private:
    int32_t AddRspHandler(std::shared_ptr<ResponseHandler> &handler, AsyncIpcCallBack callBack, bool isSync);
    int32_t WaitResponse(std::shared_ptr<ResponseHandler> rspHandler, SyncOutPut output);
    int32_t DeleteRspHandler(const uint64_t msgId);
    uint64_t GetMsgId();    
    int32_t Request(AsyncIpcCallBack callback, ChannelWork work, bool isSync, SyncOutPut output = nullptr);
    int32_t HandleMsg(const uint64_t msgId, const ResponseInfo &rspInfo);
    int32_t ClearRspHandlers();

private:
    uint64_t msgId_ = 0;
    std::mutex rspMutex_;
    std::mutex channelMutex_;
    std::map<uint64_t, std::shared_ptr<ResponseHandler>> rspHandlers_;
    std::shared_ptr<InputDataChannelProxy> channel_ = nullptr;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_PROXY_WRAP_H
