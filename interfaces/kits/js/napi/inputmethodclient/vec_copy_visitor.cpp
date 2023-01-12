/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#include "vec_copy_visitor.h"

namespace OHOS {
namespace MiscServices {
void VecCopyVisitor::Next(const int type)
{
    if (cur_ < data_.size()) {
        cur_++;
        if (type == NextType::THREAD) {
            while (data_[cur_]->threadId_ != std::this_thread::get_id()) {
                IMSA_HILOGD("differ threadId.");
                cur_++;
            }
        }
    }
}

bool VecCopyVisitor::IsDone()
{
    return cur_ >= data_.size();
}

void VecCopyVisitor::First()
{
    cur_ = 0;
}

std::shared_ptr<JSCallbackObject> VecCopyVisitor::GetCurElement()
{
    return cur_ < data_.size() ? data_[cur_] : nullptr; 
}

void VecCopyVisitor::Add(std::shared_ptr<JSCallbackObject> element)
{
    data_.push_back(element);
}

void VecCopyVisitor::Clear()
{
    data_.clear();
}

napi_value VecCopyVisitor::CallJsFunction(const napi_value* param, int32_t paramNum)
{
    napi_value callback = nullptr;
    napi_get_reference_value(data_[cur_]->env_, data_[cur_]->callback_, &callback);
    if (callback == nullptr) {
        IMSA_HILOGE("callback is nullptr");
        return nullptr;
    }
    napi_value global = nullptr;
    napi_get_global(data_[cur_]->env_, &global);
    napi_value result;
    napi_status callStatus = napi_call_function(data_[cur_]->env_, global, callback, paramNum, param, &result);
    if (callStatus != napi_ok) {
        IMSA_HILOGE(
            "notify data change failed callStatus:%{public}d callback:%{public}p", callStatus, callback);
        return nullptr;
    }
    return result == nullptr ? nullptr : result;
}
} // namespace MiscServices
} // namespace OHOS
