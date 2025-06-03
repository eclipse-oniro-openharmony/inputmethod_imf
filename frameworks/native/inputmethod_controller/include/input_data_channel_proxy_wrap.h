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

namespace OHOS {
namespace MiscServices {
using AsyncIpcCallBack = std::function<void(int32_t, ResponseData&)>;
using ChannelWork = std::function<int32_t(uint64_t msgId, std::shared_ptr<InputDataChannelProxy> channel)>;
using ChannelOutPut = std::function<int32_t(const ResponseData &data)>;
class InputDataChannelProxyWrap {
public:

    InputDataChannelProxyWrap(std::shared_ptr<InputDataChannelProxy> channel);
    ~InputDataChannelProxyWrap();

    int32_t InsertText(const std::string &text, bool isSync = false, AsyncIpcCallBack callback = nullptr);
    int32_t DeleteForward(int32_t length, AsyncIpcCallBack callback = nullptr);
    int32_t DeleteBackward(int32_t length, AsyncIpcCallBack callback = nullptr);
    int32_t GetTextBeforeCursor(int32_t number, std::string &text, AsyncIpcCallBack callback = nullptr);
    int32_t GetTextAfterCursor(int32_t number, std::string &text, AsyncIpcCallBack callback = nullptr);
    void SendKeyboardStatus(int32_t status, bool isSync = false, AsyncIpcCallBack callback = nullptr);
    int32_t SendFunctionKey(int32_t funcKey, AsyncIpcCallBack callback = nullptr);
    int32_t MoveCursor(int32_t keyCode, AsyncIpcCallBack callback = nullptr);
    int32_t GetEnterKeyType(int32_t &keyType, AsyncIpcCallBack callback = nullptr);
    int32_t GetInputPattern(int32_t &inputPattern, AsyncIpcCallBack callback = nullptr);
    int32_t SelectByRange(int32_t start, int32_t end, AsyncIpcCallBack callback = nullptr);
    int32_t SelectByMovement(int32_t direction, int32_t cursorMoveSkip, AsyncIpcCallBack callback = nullptr);
    int32_t HandleExtendAction(int32_t action, AsyncIpcCallBack callback = nullptr);
    int32_t GetTextIndexAtCursor(int32_t &index, AsyncIpcCallBack callback = nullptr);
    int32_t GetTextConfig(TextTotalConfig &textConfig, AsyncIpcCallBack callback = nullptr, bool ipcSync = false);
    void NotifyPanelStatusInfo(
        const PanelStatusInfoInner &info, bool isSync = false, AsyncIpcCallBack callback = nullptr);
    void NotifyKeyboardHeight(uint32_t height, bool isSync = false, AsyncIpcCallBack callback = nullptr);
    int32_t SendPrivateCommand(const Value &value, AsyncIpcCallBack callback = nullptr);
    int32_t SetPreviewText(const std::string &text, const RangeInner &range, AsyncIpcCallBack callback = nullptr);
    int32_t FinishTextPreview(bool isAsync, AsyncIpcCallBack callback = nullptr);
    int32_t SendMessage(const ArrayBuffer &arraybuffer, AsyncIpcCallBack callback = nullptr);

public:
    int32_t ClearMsg(bool isNotify);
    int32_t OnResponse(const uint64_t msgId, int32_t errorCode, const ResponseData &reply);

private:
    int32_t BeforeRequest(uint64_t &msgId, AsyncIpcCallBack callBack = nullptr, bool isSync = true);
    int32_t WaitResponse(const uint64_t msgId, uint32_t timeout, int32_t &errorCode, ResponseData &data);
    int32_t DelMsg(const uint64_t msgId);
    uint64_t GetMsgId();
    int32_t HandleMsg(const uint64_t msgId, int32_t code, const ResponseData &data, int32_t defErrCode);
    std::shared_ptr<InputDataChannelProxy> GetDataChannel();
    int32_t Request(AsyncIpcCallBack callback, ChannelWork work, bool isSync, ChannelOutPut output = nullptr);

private:
    struct MsgInfo {
        bool isSync = false;
        uint64_t msgId = 0;
        AsyncIpcCallBack callBack = nullptr;
        std::shared_ptr<std::condition_variable> cv = nullptr;
        MsgInfo(bool isSync, uint64_t msgId, AsyncIpcCallBack callBack)
        {
            this->isSync = isSync;
            this->msgId = msgId;
            this->callBack = callBack;
            if (isSync) {
                cv = std::make_shared<std::condition_variable>();
            }
        }
    };

    int32_t rspErrorCode_ = 0;
    uint64_t msgId_ = 0;
    uint64_t rspMsgId_ = 0;
    std::mutex dataMutex_;
    std::mutex rspMutex_;
    std::mutex channelMutex_;
    ResponseData rspData_;
    std::map<uint64_t, MsgInfo> msgList_;
    std::shared_ptr<InputDataChannelProxy> channel_ = nullptr;
};
} // namespace MiscServices
} // namespace OHOS
#endif // FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_INPUT_DATA_CHANNEL_PROXY_WRAP_H
