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

#ifndef INTERFACE_KITS_VEC_COPY_VISITOR_H
#define INTERFACE_KITS_VEC_COPY_VISITOR_H

#include <memory>
#include <vector>

#include "global.h"
#include "js_callback_object.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace MiscServices {
enum NextType : int32_t {
    NORMAL = 0,
    THREAD,
};
class VecCopyVisitor {
public:
    explicit VecCopyVisitor(std::vector<std::shared_ptr<JSCallbackObject>> VecCopy) : data_(VecCopy){}
    ~VecCopyVisitor() = default;
    void Next(const int type);
    void First();
    void Clear();
    bool IsDone();
    void Add(std::shared_ptr<JSCallbackObject>);
    std::shared_ptr<JSCallbackObject> GetCurElement();
    napi_value CallJsFunction(const napi_value* param, int paramNum);
private:
    std::vector<std::shared_ptr<JSCallbackObject>> data_;
    size_t cur_ = 0;
};
}
}
#endif // INTERFACE_KITS_VEC_COPY_VISITOR_H