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
#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <variant>

#include "block_data.h"
#include "input_data_channel_proxy.h"
#include "input_method_utils.h"

namespace OHOS {
namespace MiscServices {
struct ResponseInfo {
    int32_t dealRet_{ ErrorCode::NO_ERROR };
    ResponseData data_{ std::monostate{} };
};
struct ResponseHandler {
    static constexpr uint32_t SYNC_REPLY_TIMEOUT = 3000; // unit ms
    AsyncIpcCallBack callback_ = nullptr;
    std::shared_ptr<BlockData<ResponseInfo>> syncBlockData_ = nullptr;
    ResponseHandler(bool isSync, const AsyncIpcCallBack &callback)
    {
        callBack_ = callBack;
        if (isSync) {
            syncBlockData_ = std::make_shared<BlockData<ResponseInfo>>(SYNC_REPLY_TIMEOUT);
        }
    }
};
using AsyncIpcCallBack = std::function<void(int32_t, const ResponseData &)>;
using ChannelWork = std::function<int32_t(uint64_t msgId, const std::shared_ptr<InputDataChannelProxy> &channel)>;
using SyncOutput = std::function<void(const ResponseData &)>;
class InputDataChannelProxyWrap {
public:
    explicit InputDataChannelProxyWrap(const std::shared_ptr<InputDataChannelProxy> &channel);
    ~InputDataChannelProxyWrap();

    int32_t InsertText(const std::string &text, bool isSync = false, const AsyncIpcCallBack &callback = nullptr);
    int32_t DeleteForward(int32_t length, const AsyncIpcCallBack &callback = nullptr);
    int32_t DeleteBackward(int32_t length, const AsyncIpcCallBack &callback = nullptr);
    int32_t GetTextBeforeCursor(int32_t number, std::string &text, const AsyncIpcCallBack &callback = nullptr);
    int32_t GetTextAfterCursor(int32_t number, std::string &text, const AsyncIpcCallBack &callback = nullptr);
    int32_t SendFunctionKey(int32_t funcKey, const AsyncIpcCallBack &callback = nullptr);
    int32_t MoveCursor(int32_t keyCode, const AsyncIpcCallBack &callback = nullptr);
    int32_t SelectByRange(int32_t start, int32_t end, const AsyncIpcCallBack &callback = nullptr);
    int32_t SelectByMovement(int32_t direction, int32_t cursorMoveSkip, const AsyncIpcCallBack &callback = nullptr);
    int32_t HandleExtendAction(int32_t action, const AsyncIpcCallBack &callback = nullptr);
    int32_t GetTextIndexAtCursor(int32_t &index, const AsyncIpcCallBack &callback = nullptr);
    int32_t SendPrivateCommand(const Value &value, const AsyncIpcCallBack &callback = nullptr);
    int32_t SetPreviewText(
        const std::string &text, const RangeInner &range, const AsyncIpcCallBack &callback = nullptr);
    int32_t FinishTextPreview(bool isAsync, const AsyncIpcCallBack &callback = nullptr);

public:
    int32_t HandleResponse(uint64_t msgId, const ResponseInfo &rspInfo);
    std::shared_ptr<InputDataChannelProxy> GetDataChannel();

private:
    int32_t AddRspHandler(uint64_t msgId, const std::shared_ptr<ResponseHandler> &handler);
    int32_t WaitResponse(const std::shared_ptr<ResponseHandler> &rspHandler, const SyncOutput &output);
    int32_t DeleteRspHandler(uint64_t msgId);
    uint64_t GenerateMsgId();
    int32_t Request(
        const AsyncIpcCallBack &callback, const ChannelWork &work, bool isSync, const SyncOutput &output = nullptr);
    int32_t ClearRspHandlers();

private:
    std::atomic<uint64_t> msgId_{ 0 };
    std::mutex rspMutex_;
    std::map<uint64_t, std::shared_ptr<ResponseHandler>> rspHandlers_;
    std::mutex channelMutex_;
    std::shared_ptr<InputDataChannelProxy> channel_ = nullptr;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_PROXY_WRAP_H
