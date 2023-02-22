/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "global.h"

#include <cstdio>

namespace OHOS {
namespace MiscServices {
void LogTimeStamp()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    struct tm nowTime;
    localtime_r(&tv.tv_sec, &nowTime);
    int32_t millSec = 1000;
    printf("%02d-%02d %02d:%02d:%02d.%03d\t", nowTime.tm_mon, nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min,
        nowTime.tm_sec, (int)tv.tv_usec / millSec);
}
} // namespace MiscServices
} // namespace OHOS
