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

#ifndef IMF_HISYSEVENT_UTIL_H
#define IMF_HISYSEVENT_UTIL_H
#include <unordered_set>
#include <vector>

#include "imf_hisysevent_info.h"
namespace OHOS {
namespace MiscServices {
class ImfHiSysEventUtil {
public:
    static void ReportClientAttachFault(const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info);
    static void ReportClientShowFault(const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info);
    static void ReportImeStartInputFault(const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info);
    static void ReportBaseTextOperationFault(
        const std::string &selfName, int64_t faultNum, const HiSysOriginalInfo &info);
    static void ReportStatisticsEvent(const std::string &eventName, const std::vector<std::string> &imeNames,
        const std::vector<std::string> &appNames, const std::vector<std::string> &statistics);
    static void ReportStatisticsEvent(const std::string &eventName, const std::string &imeName,
        const std::vector<std::string> &appNames, const std::vector<std::string> &statistics);
    static std::string GetAppName(uint32_t tokenId);
    static std::string AddIfAbsent(const std::string &bundleName, std::vector<std::string> &bundleNames);
};
} // namespace MiscServices
} // namespace OHOS

#endif // IMF_HISYSEVENT_UTIL_H