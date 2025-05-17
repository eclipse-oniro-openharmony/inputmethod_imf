/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "input_method_manager_command.h"
#include <getopt.h>
#include <iostream>

#include "input_method_controller.h"

class InputMethodManagerCommand {
public:
    int32_t ParseCommand(int32_t argc, char *argv[]);
    void ShowUsage();
};
namespace OHOS {
namespace MiscServices {
struct ImeMangerInfo {
    std::string bundleName;
    std::string extensionName;
};

static std::string EnabledStatusToString(EnabledStatus status) {
    switch(status) {
        case EnabledStatus::DISABLED:
            return "DISABLED";
        case EnabledStatus::BASIC_MODE:
            return "BASIC_MODE";
        case EnabledStatus::FULL_EXPERIENCE_MODE:
            return "FULL_EXPERIENCE_MODE";
        default:
            return "UNKNOWN";
    }
}

bool ParseImeArgument(const char* optarg, ImeMangerInfo& info, const std::string& type = "extension")
{
    constexpr int32_t MAX_OPTIONS_STR_LEN = 128 + 1 + 127; // related to budleName:extension max len
    if (optarg == nullptr || strnlen(optarg, MAX_OPTIONS_STR_LEN + 1) > MAX_OPTIONS_STR_LEN) {
        std::cout << "Invalid argument!" << std::endl;
        return false;
    }
    
    std::string argStr(optarg);
    size_t colonPos = argStr.find(':');
    if (colonPos == std::string::npos) {
        std::cout << "Invalid format. Use 'bundle:"<< type <<"'" << std::endl;
        return false;
    }
    
    info.bundleName = argStr.substr(0, colonPos);
    info.extensionName = argStr.substr(colonPos + 1);
    return true;
}

bool ValidateImeExists(const std::string& bundle, const std::string& extension = "")
{
    std::vector<Property> methods;
    auto controller = InputMethodController::GetInstance();
    if (controller == nullptr) {
        std::cout << "Error: InputMethodController instance is null." << std::endl;
        return false;
    }
    controller->ListInputMethod(methods);
    for (const auto& m : methods) {
        if(extension.empty()) {
            if(m.name == bundle){
                return true;
            }
        } else {
            if(m.name == bundle && m.id == extension) {
                return true;
            }
        }
    }

    std::cout << "Error: The input method does not exist." << std::endl;
    return false;
}

void HandleStatusChange(const char* optarg, EnabledStatus status, const std::string& successMsg)
{
    ImeMangerInfo info;
    if (!ParseImeArgument(optarg, info)) {
        return;
    }
    if (!ValidateImeExists(info.bundleName, info.extensionName)) {
        return;
    }
    auto controller = InputMethodController::GetInstance();
    if (controller == nullptr) {
        std::cout << "Error: InputMethodController instance is null." << std::endl;
        return;
    }
    auto ret = controller->EnableIme(info.bundleName, info.extensionName, status);
    if (ret == ErrorCode::NO_ERROR) {
        std::cout << successMsg << ". IME:" << optarg << std::endl;
    } else {
        std::cout << "Operation failed.Error code:" << ret << std::endl;
    }
}

void HandleSwitchIme(const char* optarg)
{
    ImeMangerInfo info;
    std::string type = "subName";
    if (!ParseImeArgument(optarg, info, type)) {
        return;
    }
    if (!ValidateImeExists(info.bundleName)) {
        return;
    }
    auto controller = InputMethodController::GetInstance();
    if (controller == nullptr) {
        std::cout << "Error: InputMethodController instance is null." << std::endl;
        return;
    }
    int32_t ret = controller->SwitchInputMethod(
        SwitchTrigger::NATIVE_SA, info.bundleName, info.extensionName);
    if (ret == 0) {
        std::cout << "Succeeded in switching the input method. IME:" << optarg << std::endl;
    } else {
        std::cout << "Could not switch the input method. IME:" << optarg << std::endl;
    }
}

int32_t InputMethodManagerCommand::ParseCommand(int32_t argc, char *argv[])
{
    int32_t optCode = 0;
    while ((optCode = getopt(argc, argv, "b:d:e:f:ghls:")) != -1) {
        switch (optCode) {
            case 'e':
                HandleStatusChange(optarg, EnabledStatus::BASIC_MODE, "Succeeded in enabling IME");
                break;
            case 'd':
                HandleStatusChange(optarg, EnabledStatus::DISABLED, "Succeeded in disabling IME");
                break;
            case 'f':
                HandleStatusChange(optarg, EnabledStatus::FULL_EXPERIENCE_MODE,
                    "Succeeded in setting IME to basic mode");
                break;
            case 'b':
                HandleStatusChange(optarg, EnabledStatus::BASIC_MODE, "Succeeded in setting IME to full mode");
                break;
            case 's':
                HandleSwitchIme(optarg);
                break;
            case 'g': {
                auto controller = InputMethodController::GetInstance();
                if (controller == nullptr) {
	                std::cout << "Error: InputMethodController instance is null." << std::endl;
	                return false;
                }
                auto propertyData = controller->GetCurrentInputMethod();
                if (propertyData) {
                    std::cout << "The current input method is: " << propertyData->name << std::endl;
                } else {
                    std::cout << "Error: can not get current inputmethod" << std::endl;
                }
                break;
            }
            case 'l': {
                auto controller = InputMethodController::GetInstance();
                if (controller == nullptr) {
	                std::cout << "Error: InputMethodController instance is null." << std::endl;
	                return false;
                }
                std::vector<Property> methods;
                controller->ListInputMethod(true, methods);
                for (auto& m : methods) {
                    std::cout << "bundle: " << m.name << ", extension: " << m.id
                              << ", status: " << EnabledStatusToString(m.status) << std::endl;
                }
                break;
            }
            case 'h':
                ShowUsage();
                break;
            case '?':
                std::cout << "Invalid command!" << std::endl;
                ShowUsage();
                break;
            default:
                break;
        }
    }
    return 0;
}

void InputMethodManagerCommand::ShowUsage()
{
    std::cout << "\nInput Method Manager Command Line Tool\n"
              << "Usage: ime [OPTION] [ARGUMENT]\n\n"
              << "Options:\n"
              << "  -e <bundle:extension>  Enable the specified input method\n"
              << "  -d <bundle:extension>  Disable the specified input method\n"
              << "  -f <bundle:extension>  Set the input method to full experience mode\n"
              << "  -b <bundle:extension>  Set the input method to basic mode\n"
              << "  -s <bundle:subName>  Switch to the specified input method\n"
              << "  -g               Get current input method\n"
              << "  -l               List all input methods\n"
              << "  -h               Show this help message\n";
}
} // namespace MiscServices
} // namespace OHOS