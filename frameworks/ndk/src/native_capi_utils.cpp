/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "native_inputmethod_utils.h"
#include <map>

#include "global.h"
using namespace OHOS::MiscServices;
static const std::map<int32_t, InputMethod_ErrorCode> ERROR_CODE_MAP = {
    { ErrorCode::NO_ERROR,                            IME_ERR_OK                  },
    { ErrorCode::ERROR_CONTROLLER_INVOKING_FAILED,    IME_ERR_CONTROLLER          },
    { ErrorCode::ERROR_REMOTE_CLIENT_DIED,            IME_ERR_IMCLIENT            },
    { ErrorCode::ERROR_CLIENT_NOT_FOUND,              IME_ERR_IMCLIENT            },
    { ErrorCode::ERROR_CLIENT_NULL_POINTER,           IME_ERR_IMCLIENT            },
    { ErrorCode::ERROR_CLIENT_NOT_FOCUSED,            IME_ERR_IMCLIENT            },
    { ErrorCode::ERROR_CLIENT_NOT_EDITABLE,           IME_ERR_EDITABLE            },
    { ErrorCode::ERROR_CLIENT_NOT_BOUND,              IME_ERR_DETACHED            },
    { ErrorCode::ERROR_CLIENT_ADD_FAILED,             IME_ERR_IMCLIENT            },
    { ErrorCode::ERROR_NULL_POINTER,                  IME_ERR_IMMS                },
    { ErrorCode::ERROR_BAD_PARAMETERS,                IME_ERR_IMMS                },
    { ErrorCode::ERROR_SERVICE_START_FAILED,          IME_ERR_IMMS                },
    { ErrorCode::ERROR_KBD_SHOW_FAILED,               IME_ERR_IMMS                },
    { ErrorCode::ERROR_KBD_HIDE_FAILED,               IME_ERR_IMMS                },
    { ErrorCode::ERROR_IME_NOT_STARTED,               IME_ERR_IMMS                },
    { ErrorCode::ERROR_EX_NULL_POINTER,               IME_ERR_IMMS                },
    { ErrorCode::ERROR_PACKAGE_MANAGER,               IME_ERR_PACKAGEMANAGER      },
    { ErrorCode::ERROR_EX_UNSUPPORTED_OPERATION,      IME_ERR_IMMS                },
    { ErrorCode::ERROR_EX_SERVICE_SPECIFIC,           IME_ERR_IMMS                },
    { ErrorCode::ERROR_EX_PARCELABLE,                 IME_ERR_IMMS                },
    { ErrorCode::ERROR_EX_ILLEGAL_ARGUMENT,           IME_ERR_IMMS                },
    { ErrorCode::ERROR_EX_ILLEGAL_STATE,              IME_ERR_IMMS                },
    { ErrorCode::ERROR_IME_START_INPUT_FAILED,        IME_ERR_IMMS                },
    { ErrorCode::ERROR_IME,                           IME_ERR_IMENGINE            },
    { ErrorCode::ERROR_PARAMETER_CHECK_FAILED,        IME_ERR_PARAMCHECK          },
    { ErrorCode::ERROR_ENABLE_IME,                    IME_ERR_IMMS                },
    { ErrorCode::ERROR_NOT_CURRENT_IME,               IME_ERR_IMMS                },
    { ErrorCode::ERROR_GET_TEXT_CONFIG,               IME_ERR_IMCLIENT            },
    { ErrorCode::ERROR_INVALID_PRIVATE_COMMAND_SIZE,  IME_ERR_PARAMCHECK          },
    { ErrorCode::ERROR_TEXT_LISTENER_ERROR,           IME_ERR_IMCLIENT            },
    { ErrorCode::ERROR_INVALID_RANGE,                 IME_ERR_PARAMCHECK          },
    { ErrorCode::ERROR_MSG_HANDLER_NOT_REGIST,        IME_ERR_REQUEST_NOT_ACCEPT  },
    { ErrorCode::ERROR_SECURITY_MODE_OFF,             IME_ERR_BASIC_MODE          },
    { ErrorCode::ERROR_INVALID_ARRAY_BUFFER_SIZE,     IME_ERR_PARAMCHECK          },
    { ErrorCode::ERROR_IMA_CHANNEL_NULLPTR,           IME_ERR_IMCLIENT            },
    { ErrorCode::ERROR_IPC_REMOTE_NULLPTR,            IME_ERR_IMMS                },
    { ErrorCode::ERROR_IMA_NULLPTR,                   IME_ERR_IMMS                },
    { ErrorCode::ERROR_IMSA_INPUT_TYPE_NOT_FOUND,     IME_ERR_IMMS                },
    { ErrorCode::ERROR_IMSA_DEFAULT_IME_NOT_FOUND,    IME_ERR_IMMS                },
    { ErrorCode::ERROR_IMSA_CLIENT_INPUT_READY_FAILED, IME_ERR_IMMS               },
    { ErrorCode::ERROR_IMSA_MALLOC_FAILED,            IME_ERR_IMMS                },
    { ErrorCode::ERROR_IMSA_NULLPTR,                  IME_ERR_IMMS                },
    { ErrorCode::ERROR_IMSA_USER_SESSION_NOT_FOUND,   IME_ERR_IMMS                },
    { ErrorCode::ERROR_IMSA_GET_IME_INFO_FAILED,      IME_ERR_IMMS                },
    { ErrorCode::ERROR_IMSA_IME_TO_START_NULLPTR,      IME_ERR_IMMS               },
    { ErrorCode::ERROR_IMSA_REBOOT_OLD_IME_NOT_STOP,   IME_ERR_IMMS               },
    { ErrorCode::ERROR_IMSA_IME_EVENT_CONVERT_FAILED,  IME_ERR_IMMS               },
    { ErrorCode::ERROR_IMSA_IME_CONNECT_FAILED,        IME_ERR_IMMS               },
    { ErrorCode::ERROR_IMSA_IME_DISCONNECT_FAILED,     IME_ERR_IMMS               },
    { ErrorCode::ERROR_IMSA_IME_START_TIMEOUT,         IME_ERR_IMMS               },
    { ErrorCode::ERROR_IMSA_IME_START_MORE_THAN_EIGHT_SECOND, IME_ERR_IMMS        },
    { ErrorCode::ERROR_IMSA_FORCE_STOP_IME_TIMEOUT,    IME_ERR_IMMS               },
    { ErrorCode::ERROR_IMC_NULLPTR,                    IME_ERR_IMMS               },
    { ErrorCode::ERROR_RESPONSE_TIMEOUT,               IME_ERR_IMCLIENT           },
    { ErrorCode::ERROR_TOO_MANY_UNANSWERED_MESSAGE,    IME_ERR_IMCLIENT           },
    { ErrorCode::ERROR_PARSE_PARAMETER_FAILED,         IME_ERR_IMCLIENT           },
};

InputMethod_ErrorCode ErrorCodeConvert(int32_t code)
{
    IMSA_HILOGD("Convert start.");
    auto iter = ERROR_CODE_MAP.find(code);
    if (iter != ERROR_CODE_MAP.end()) {
        IMSA_HILOGE("ErrorCode: %{public}d", iter->second);
        return iter->second;
    }
    IMSA_HILOGD("Convert end.");
    return IME_ERR_UNDEFINED;
}