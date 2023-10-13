/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "panelstatuschange_fuzzer.h"

#include "imf_sa_stub_fuzz_util.h"
#include "inputmethod_service_ipc_interface_code.h"

using namespace OHOS::MiscServices;
namespace OHOS {
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    ImfSaStubFuzzUtil::FuzzInputMethodSystemAbility(data, size, InputMethodInterfaceCode::PANEL_STATUS_CHANGE);
    return 0;
}
