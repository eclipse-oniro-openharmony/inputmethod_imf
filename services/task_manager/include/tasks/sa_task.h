/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef IMF_SERVICES_SA_TASK_H
#define IMF_SERVICES_SA_TASK_H

#include "caller_info.h"
#include "iima_response_channel.h"
#include "iimc_response_channel.h"
#include "sa_action.h"
#include "sa_action_report.h"
#include "service_response_data.h"

namespace OHOS {
namespace MiscServices {
constexpr uint32_t INVALID_SEQ_ID = 0;
enum class SaTaskType : int32_t {
    TYPE_CRITICAL_CHANGE = 0, // tasks bringing critical changes or updates
    TYPE_SWITCH_IME = 1,      // tasks causing ime switch
    TYPE_HIGHER_REQUEST = 2,  // tasks with a higher priority request
    TYPE_NORMAL_REQUEST = 3,  // tasks with a normal priority request
    TYPE_RESUME = 4,          // tasks creating resume inner tasks
    TYPE_QUERY = 5,           // tasks used for querying
    TYPE_INNER = 6,           // tasks acting on the current paused task

    TYPE_TOTAL_COUNT,
    TYPE_INVALID = -1,
};

#define TASK_TYPE_OFFSET(src) ((src)*10000)

enum class SaTaskCode : uint32_t {
    TASK_CRITICAL_CHANGE_BEGIN = TASK_TYPE_OFFSET(static_cast<uint32_t>(SaTaskType::TYPE_CRITICAL_CHANGE)),
    // user change
    ON_USER_STARTED,
    ON_USER_REMOVED,
    ON_USER_STOPPED,
    // scb status change
    ON_WMS_CONNECTED,
    ON_WMS_DISCONNECTED,
    // changes update ime info
    ON_BUNDLE_SCAN_FINISHED,
    ON_DATA_SHARE_READY,
    // change current ime state
    ON_EXTENSION,
    ON_IME_ENABLED_STATE_CHANGED,
    ON_IME_LIFE_CYCLE_STOP,
    // on critical sa started
    ON_ACCOUNT_SA_STARTED,
    ON_MEM_SA_STARTED,
    ON_WMS_SA_STARTED,
    TASK_CRITICAL_CHANGE_END,

    TASK_SWITCH_IME_BEGIN = TASK_TYPE_OFFSET(static_cast<uint32_t>(SaTaskType::TYPE_SWITCH_IME)),
    // proxy ime register from IPC
    REGISTER_PROXY_IME,
    UNREGISTER_PROXY_IME,
    UNREGISTERED_PROXY_IME,
    // switch ime requests from IPC
    SWITCH_INPUT_METHOD,
    START_INPUT_TYPE,
    EXIT_CURRENT_INPUT_TYPE,
    // events which may switch ime
    ON_PACKAGE_REMOVED,
    ON_SCREEN_UNLOCKED,
    TASK_SWITCH_IME_END,

    TASK_HIGHER_REQUEST_BEGIN = TASK_TYPE_OFFSET(static_cast<uint32_t>(SaTaskType::TYPE_HIGHER_REQUEST)),
    // events which influence user typing or panel showing
    ON_FOCUSED,
    ON_UNFOCUSED,
    ON_COMMON_EVENT_SA_STARTED,
    ON_DISPLAY_ID_CHANGED,
    ON_MMI_SA_STARTED,
    TASK_HIGHER_REQUEST_END,

    TASK_NORMAL_REQUEST_BEGIN = TASK_TYPE_OFFSET(static_cast<uint32_t>(SaTaskType::TYPE_NORMAL_REQUEST)),
    // operate keyboard from IPC
    START_INPUT,
    SHOW_CURRENT_INPUT,
    HIDE_CURRENT_INPUT,
    STOP_INPUT_SESSION,
    SHOW_INPUT,
    HIDE_INPUT,
    RELEASE_INPUT,
    REQUEST_SHOW_INPUT,
    REQUEST_HIDE_INPUT,
    CONNECT_SYSTEM_CMD,
    SHOW_CURRENT_INPUT_DEPRECATED,
    HIDE_CURRENT_INPUT_DEPRECATED,
    HIDE_KEYBOARD_SELF,
    ON_PASTEBOARD_SA_STARTED,
    // data transaction or info setting requests from IPC
    SET_CALLING_WINDOW,
    SEND_PRIVATE_DATA,
    PANEL_STATUS_CHANGE,
    UPDATE_LISTEN_EVENT_FLAG,
    DISPLAY_OPTIONAL_INPUT_METHOD,
    TASK_NORMAL_REQUEST_END,

    TASK_RESUME_BEGIN = TASK_TYPE_OFFSET(static_cast<uint32_t>(SaTaskType::TYPE_RESUME)),
    // resume pause for ime start
    INIT_CONNECT,
    SET_CORE_AND_AGENT,
    // resume pause for ime stop
    ON_IME_DIED,
    TASK_RESUME_END,

    TASK_QUERY_BEGIN = TASK_TYPE_OFFSET(static_cast<uint32_t>(SaTaskType::TYPE_QUERY)),
    // Get imf info
    GET_CURRENT_INPUT_METHOD,
    GET_CURRENT_INPUT_METHOD_SUBTYPE,
    GET_DEFAULT_INPUT_METHOD,
    GET_INPUT_METHOD_CONFIG,
    GET_INPUT_METHOD_STATE,
    GET_INPUT_START_INFO,
    GET_SECURITY_MODE,
    // List ime info
    LIST_INPUT_METHOD,
    LIST_INPUT_METHOD_SUBTYPE,
    LIST_CURRENT_INPUT_METHOD_SUBTYPE,
    // Judge ime info
    IS_CURRENT_IME,
    IS_CURRENT_IME_BY_PID,
    IS_DEFAULT_IME,
    IS_DEFAULT_IME_SCREEN,
    IS_DEFAULT_IME_SET,
    IS_INPUT_TYPE_SUPPORTED,
    IS_PANEL_SHOWN,
    IS_SYSTEM_APP,
    IS_SYSTEM_IME_APP,
    // Operate or update ime info
    ENABLE_IME,
    ON_BOOT_COMPLETED,
    ON_DATA_SHARE_CALLBACK,
    ON_PACKAGE_ADDED,
    ON_PACKAGE_CHANGED,
    ON_UPDATE_GLOBAL_ENABLED_TABLE,
    ON_UPDATE_IME_INFO,
    ON_SYSTEM_LANGUAGE_CHANGED,
    TASK_QUERY_END,

    TASK_INNER_BEGIN = TASK_TYPE_OFFSET(static_cast<uint32_t>(SaTaskType::TYPE_INNER)),
    RESUME_WAIT,
    RESUME_TIMEOUT,
    TASK_INNER_END,
};

class SaTask {
public:
    explicit SaTask(SaTaskCode code)
        : code_(code), seqId_(GetNextSeqId()), imaResponseChannel_(nullptr), imcResponseChannel_(nullptr)
    {
    }
    SaTask(SaTaskCode code, uint64_t seqId)
        : code_(code), seqId_(seqId), imaResponseChannel_(nullptr), imcResponseChannel_(nullptr)
    {
    }
    SaTask(SaTaskCode code, SaActionFunc func)
        : code_(code), seqId_(GetNextSeqId()), imaResponseChannel_(nullptr), imcResponseChannel_(nullptr)
    {
        action_ = std::make_unique<SaAction>(func);
    }
    SaTask(SaTaskCode code, std::unique_ptr<SaAction> action)
        : code_(code), seqId_(GetNextSeqId()), imaResponseChannel_(nullptr), imcResponseChannel_(nullptr)
    {
        action_ = std::move(action);
    }
    SaTask(SaTaskCode code, SaActionFunc func, CallerInfo info)
        : code_(code), seqId_(GetNextSeqId()), callerInfo_(info), imaResponseChannel_(nullptr),
          imcResponseChannel_(nullptr)
    {
        action_ = std::make_unique<SaAction>(func);
    }
    SaTask(SaTaskCode code, SaActionFunc func, CallerInfo info, sptr<IImaResponseChannel> channel)
        : code_(code), seqId_(GetNextSeqId()), callerInfo_(info), imaResponseChannel_(channel),
          imcResponseChannel_(nullptr)
    {
        action_ = std::make_unique<SaAction>(func);
    }
    SaTask(SaTaskCode code, SaActionFunc func, CallerInfo info, sptr<IImcResponseChannel> channel)
        : code_(code), seqId_(GetNextSeqId()), callerInfo_(info), imaResponseChannel_(nullptr),
          imcResponseChannel_(channel)
    {
        action_ = std::make_unique<SaAction>(func);
    }
    ~SaTask();

    static uint64_t GetNextSeqId();

    void SetHiSysReporter(const ReportFunc &func);

    RunningState Execute();
    RunningState Resume(uint64_t resumeId);
    RunningState OnTask(const std::shared_ptr<SaTask> &task);

    int32_t PendWaitResult(std::unique_ptr<SaAction> action);
    // Pend an action to the task directly.
    int32_t Pend(std::unique_ptr<SaAction> action);
    // Pend an action to the task, of which properties will inherit curAction_.
    int32_t Pend(SaActionFunc func);
    template<typename... Args> int32_t Pend(Args &&... args)
    {
        return (Pend(std::forward<Args>(args)) && ...);
    }

    // get task info
    SaTaskType GetType() const;
    SaTaskCode GetCode() const;
    uint64_t GetSeqId() const;
    CallerInfo GetCallerInfo() const;
    bool IsRunning() const;
    bool IsPaused() const;
    PauseInfo GetPauseInfo();

    void OnResponse(int32_t retCode);

private:
    RunningState ExecuteInner();
    void InvokeResponse();

protected:
    static constexpr int32_t INVALID_FAIL_CODE = -1;
    RunningState state_{ RUNNING_STATE_IDLE };

    const SaTaskCode code_;
    const uint64_t seqId_;

    CallerInfo callerInfo_;
    sptr<IImaResponseChannel> imaResponseChannel_;
    sptr<IImcResponseChannel> imcResponseChannel_;
    ServiceResponseData responseData_{ std::monostate{} };

    bool hasResponded_{ false };
    int32_t failRet_{ INVALID_FAIL_CODE };
    int32_t retCode_{ ErrorCode::NO_ERROR };

    std::unique_ptr<SaAction> action_{ nullptr };
    std::unique_ptr<SaActionReport> hiSysReporter_{ nullptr };
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_SERVICES_SA_TASK_H
