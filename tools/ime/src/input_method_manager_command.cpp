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
    void ShowUsage(int32_t argc);
};
namespace OHOS {
namespace MiscServices {

static std::string EnabledStatusToString(EnabledStatus status)
{
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

bool ValidateImeExists(const std::string& bundle)
{
    std::vector<Property> methods;
    auto controller = InputMethodController::GetInstance();
    if (controller == nullptr) {
        std::cout << "Error: InputMethodController instance is null." << std::endl;
        return false;
    }
    controller->ListInputMethod(methods);
    for (const auto& m : methods) {
        if(m.name == bundle){
            return true;
        }
    }
    std::cout << "Error: The input method does not exist." << std::endl;
    return false;
}

std::string GetBundleName(const char* optarg)
{
    constexpr int32_t MAX_OPTIONS_STR_LEN = 128; // related to budleName max len
    if (optarg == nullptr) {
        std::cout << "Error: Invalid argument!" << std::endl;
        return "";
    }
    if (strnlen(optarg, MAX_OPTIONS_STR_LEN + 1) > MAX_OPTIONS_STR_LEN) {
        std::cout << "Error: Invalid argument!" << std::endl;
        return "";
    }
    std::string bundleName(optarg);
    if (bundleName.empty()) {
        std::cout << "Error: Invalid argument!" << std::endl;
        return "";
    }
    return bundleName;
}

void HandleStatusChange(const char* optarg, int32_t argc, char *argv[], EnabledStatus status,
                        const std::string& successMsg)
{
    std::string bundleName = GetBundleName(optarg);
    if (bundleName.empty()) {
        return;
    }
    if (!ValidateImeExists(bundleName)) {
        return;
    }

    if (status == EnabledStatus::BASIC_MODE && optind < argc) {
        if (optind + 1 == argc) {
            std::string nextArg = argv[optind];
            if (nextArg == "-b" || nextArg == "-f") {
                status = (nextArg == "-f") ? EnabledStatus::FULL_EXPERIENCE_MODE : EnabledStatus::BASIC_MODE;
                optind++;
            } else {
                std::cout << "Error: Invalid mode after -e. Use -b or -f" << std::endl;
                return;
            }
        } else if (optind + 1 > argc) {
            status = EnabledStatus::BASIC_MODE;
        } else {
            std::cout << "Error: Invalid mode after -e. Use -b or -f" << std::endl;
            return;
        }
    }

    if (status == EnabledStatus::DISABLED && optind < argc) {
        std::cout << "Error: Invalid command!" << std::endl;
        return;
    }
    auto controller = InputMethodController::GetInstance();
    if (controller == nullptr) {
        std::cout << "Error: InputMethodController instance is null." << std::endl;
        return;
    }
    auto ret = controller->EnableIme(bundleName, "", status);
    if (ret == ErrorCode::NO_ERROR) {
        std::cout << successMsg << ". status:" << EnabledStatusToString(status) << std::endl;
    } else {
        std::cout << "Error: Operation failed.Error code:" << ret << std::endl;
    }
}

void HandleSwitchIme(const char* optarg)
{
    std::string bundleName = GetBundleName(optarg);
    if (bundleName.empty()) {
        return;
    }
    if (!ValidateImeExists(bundleName)) {
        return;
    }
    auto controller = InputMethodController::GetInstance();
    if (controller == nullptr) {
        std::cout << "Error: InputMethodController instance is null." << std::endl;
        return;
    }
    int32_t ret = controller->SwitchInputMethod(SwitchTrigger::NATIVE_SA, bundleName);
    if (ret == 0) {
        std::cout << "Succeeded in switching the input method. IME:" << bundleName << std::endl;
    } else {
        std::cout << "Error: Operation failed. Error code:" << ret << std::endl;
    }
}

void HandleGetCurrentIme(int32_t argc)
{
    if (optind < argc) {
        std::cout << "Error: Invalid command!" << std::endl;
        return;
    }
    auto controller = InputMethodController::GetInstance();
    if (controller == nullptr) {
        std::cout << "Error: InputMethodController instance is null." << std::endl;
        return;
    }
    auto propertyData = controller->GetCurrentInputMethod();
    if (propertyData != nullptr) {
        std::cout << "The current input method is: " << propertyData->name << ", status: "
                  << EnabledStatusToString(propertyData->status) << std::endl;
    } else {
        std::cout << "Error: can not get current input method." << std::endl;
    }
}

void HandleListIme(int32_t argc)
{
    if (optind < argc) {
        std::cout << "Error: Invalid command!" << std::endl;
        return;
    }
    auto controller = InputMethodController::GetInstance();
    if (controller == nullptr) {
        std::cout << "Error: InputMethodController instance is null." << std::endl;
        return;
    }
    std::vector<Property> methods;
    auto ret = controller->ListInputMethod(true, methods);
    if (ret != ErrorCode::NO_ERROR) {
        std::cout << "Error: ListInputMethod failed. Error code:" << ret << std::endl;
        return;
    }
    for (const auto& m : methods) {
        std::cout << "bundle: " << m.name << ", status: " << EnabledStatusToString(m.status) << std::endl;
    }
}
int32_t InputMethodManagerCommand::ParseCommand(int32_t argc, char *argv[])
{
    int32_t optCode = 0;
    while ((optCode = getopt(argc, argv, "d:e:ghls:")) != -1) {
        switch (optCode) {
            case 'e':
                HandleStatusChange(optarg, argc, argv, EnabledStatus::BASIC_MODE, "Succeeded in enabling IME");
                return ErrorCode::NO_ERROR;
            case 'd':
                HandleStatusChange(optarg, argc, argv, EnabledStatus::DISABLED, "Succeeded in disabling IME");
                return ErrorCode::NO_ERROR;
            case 's':
                HandleSwitchIme(optarg);
                return ErrorCode::NO_ERROR;
            case 'g':
                HandleGetCurrentIme(argc);
                return ErrorCode::NO_ERROR;
            case 'l':
                HandleListIme(argc);
                return ErrorCode::NO_ERROR;
            case 'h':
                ShowUsage(argc);
                return ErrorCode::NO_ERROR;
            case '?':
                ShowUsage(argc);
                return ErrorCode::NO_ERROR;
            default:
                return ErrorCode::NO_ERROR;
        }
    }
    return ErrorCode::NO_ERROR;
}

void InputMethodManagerCommand::ShowUsage(int32_t argc)
{
    if (optind < argc) {
        std::cout << "Error: Invalid command!" << std::endl;
        return;
    }
    std::cout << "\nInput Method Manager Command Line Tool\n"
              << "Usage: ime [OPTION] [ARGUMENT]\n\n"
              << "Options:\n"
              << "  -e <bundle> [-b | -f] Enable the specified input method to specified mode.\n"
              << "                        If the -b/-f option is not set, the default value is -b.\n"
              << "  -d <bundle>           Disable the specified input method\n"
              << "  -s <bundle>           Switch to the specified input method\n"
              << "  -g                    Get current input method\n"
              << "  -l                    List all input methods\n"
              << "  -h                    Show this help message\n";
}
} // namespace MiscServices
} // namespace OHOS