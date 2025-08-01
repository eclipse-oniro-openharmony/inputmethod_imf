/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef IMF_SYSTEM_PARAM_ADAPTER_H
#define IMF_SYSTEM_PARAM_ADAPTER_H

#include <string>
namespace OHOS {
namespace MiscServices {
class SystemParamAdapter {
public:
    static constexpr const char *SYSTEM_LANGUAGE_KEY = "persist.global.language";
    static constexpr const char *MEMORY_WATERMARK_KEY = "resourceschedule.memmgr.min.memmory.watermark";
    static SystemParamAdapter &GetInstance();
    int32_t WatchParam(const std::string &key);
    bool GetBoolParam(const std::string &key);

private:
    SystemParamAdapter() = default;
    using CbHandler = void (*)(const char *key, const char *value, void *context);
    static const std::unordered_map<std::string, CbHandler> PARAM_CB_HANDLERS;
    static void OnLanguageChange(const char *key, const char *value, void *context);
    static void OnMemoryChange(const char *key, const char *value, void *context);
    static void HandleSysParamChanged(const char *key, const char *value, const char *expectedKey, int32_t messageId);
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_SYSTEM_PARAM_ADAPTER_H
