/*
* Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "itypes_util.h"
#include "message_parcel.h"

using namespace OHOS;
using namespace OHOS::MiscServices;

TEST(ITypesUtilTest, BoolMarshallingUnmarshalling) {
    bool input = true;
    bool output = false;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input, output);
}

TEST(ITypesUtilTest, Uint32MarshallingUnmarshalling) {
    uint32_t input = 12345;
    uint32_t output = 0;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input, output);
}

TEST(ITypesUtilTest, Int32MarshallingUnmarshalling) {
    int32_t input = -12345;
    int32_t output = 0;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input, output);
}

TEST(ITypesUtilTest, Uint64MarshallingUnmarshalling) {
    uint64_t input = 123456789012345;
    uint64_t output = 0;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input, output);
}

TEST(ITypesUtilTest, DoubleMarshallingUnmarshalling) {
    double input = 123.456;
    double output = 0.0;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_DOUBLE_EQ(input, output);
}

TEST(ITypesUtilTest, StringMarshallingUnmarshalling) {
    std::string input = "Hello, World!";
    std::string output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input, output);
}

TEST(ITypesUtilTest, U16StringMarshallingUnmarshalling) {
    std::u16string input = u"Hello, World!";
    std::u16string output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input, output);
}

TEST(ITypesUtilTest, Uint8VectorMarshallingUnmarshalling) {
    std::vector<uint8_t> input = {1, 2, 3, 4, 5};
    std::vector<uint8_t> output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input, output);
}

TEST(ITypesUtilTest, RemoteObjectMarshallingUnmarshalling) {
    sptr<IRemoteObject> input = new IRemoteObject();
    sptr<IRemoteObject> output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input, output);
}

TEST(ITypesUtilTest, PropertyMarshallingUnmarshalling) {
    Property input = {"name", 1, "label", 2, "icon", 3};
    Property output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input.name, output.name);
    EXPECT_EQ(input.id, output.id);
    EXPECT_EQ(input.label, output.label);
    EXPECT_EQ(input.labelId, output.labelId);
    EXPECT_EQ(input.icon, output.icon);
    EXPECT_EQ(input.iconId, output.iconId);
}

TEST(ITypesUtilTest, SubPropertyMarshallingUnmarshalling) {
    SubProperty input = {"label", 1, "name", 2, 3, "locale", "language", "icon", 4};
    SubProperty output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input.label, output.label);
    EXPECT_EQ(input.labelId, output.labelId);
    EXPECT_EQ(input.name, output.name);
    EXPECT_EQ(input.id, output.id);
    EXPECT_EQ(input.mode, output.mode);
    EXPECT_EQ(input.locale, output.locale);
    EXPECT_EQ(input.language, output.language);
    EXPECT_EQ(input.icon, output.icon);
    EXPECT_EQ(input.iconId, output.iconId);
}

TEST(ITypesUtilTest, InputAttributeMarshallingUnmarshalling) {
    InputAttribute input = {1, 2, 3, true, "bundleName"};
    InputAttribute output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input.inputPattern, output.inputPattern);
    EXPECT_EQ(input.enterKeyType, output.enterKeyType);
    EXPECT_EQ(input.inputOption, output.inputOption);
    EXPECT_EQ(input.isTextPreviewSupported, output.isTextPreviewSupported);
    EXPECT_EQ(input.bundleName, output.bundleName);
}

TEST(ITypesUtilTest, TextTotalConfigMarshallingUnmarshalling) {
    TextTotalConfig input;
    input.inputAttribute = {1, 2, 3, true, "bundleName"};
    input.cursorInfo = {1, 2, 3, 4};
    input.textSelection = {1, 2, 3, 4};
    input.windowId = 123;
    input.positionY = 456;
    input.height = 789;
    input.privateCommand = "privateCommand";

    TextTotalConfig output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input.inputAttribute.inputPattern, output.inputAttribute.inputPattern);
    EXPECT_EQ(input.inputAttribute.enterKeyType, output.inputAttribute.enterKeyType);
    EXPECT_EQ(input.inputAttribute.inputOption, output.inputAttribute.inputOption);
    EXPECT_EQ(input.inputAttribute.isTextPreviewSupported, output.inputAttribute.isTextPreviewSupported);
    EXPECT_EQ(input.inputAttribute.bundleName, output.inputAttribute.bundleName);
    EXPECT_EQ(input.cursorInfo.left, output.cursorInfo.left);
    EXPECT_EQ(input.cursorInfo.top, output.cursorInfo.top);
    EXPECT_EQ(input.cursorInfo.height, output.cursorInfo.height);
    EXPECT_EQ(input.cursorInfo.width, output.cursorInfo.width);
    EXPECT_EQ(input.textSelection.oldBegin, output.textSelection.oldBegin);
    EXPECT_EQ(input.textSelection.oldEnd, output.textSelection.oldEnd);
    EXPECT_EQ(input.textSelection.newBegin, output.textSelection.newBegin);
    EXPECT_EQ(input.textSelection.newEnd, output.textSelection.newEnd);
    EXPECT_EQ(input.windowId, output.windowId);
    EXPECT_EQ(input.positionY, output.positionY);
    EXPECT_EQ(input.height, output.height);
    EXPECT_EQ(input.privateCommand, output.privateCommand);
}

TEST(ITypesUtilTest, InputClientInfoMarshallingUnmarshalling) {
    InputClientInfo input = {1, 2, 3, 4, true, 5, 6, 7, true, true};
    InputClientInfo output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input.pid, output.pid);
    EXPECT_EQ(input.uid, output.uid);
    EXPECT_EQ(input.userID, output.userID);
    EXPECT_EQ(input.isShowKeyboard, output.isShowKeyboard);
    EXPECT_EQ(input.eventFlag, output.eventFlag);
    EXPECT_EQ(input.config, output.config);
    EXPECT_EQ(input.state, output.state);
    EXPECT_EQ(input.isNotifyInputStart, output.isNotifyInputStart);
    EXPECT_EQ(input.needHide, output.needHide);
}

TEST(ITypesUtilTest, ImeWindowInfoMarshallingUnmarshalling) {
    ImeWindowInfo input;
    input.panelInfo.panelFlag = PanelFlag::FLAG_1;
    input.panelInfo.panelType = PanelType::TYPE_1;
    input.windowInfo.name = "name";
    input.windowInfo.top = 1;
    input.windowInfo.left = 2;
    input.windowInfo.width = 3;
    input.windowInfo.height = 4;

    ImeWindowInfo output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input.panelInfo.panelFlag, output.panelInfo.panelFlag);
    EXPECT_EQ(input.panelInfo.panelType, output.panelInfo.panelType);
    EXPECT_EQ(input.windowInfo.name, output.windowInfo.name);
    EXPECT_EQ(input.windowInfo.top, output.windowInfo.top);
    EXPECT_EQ(input.windowInfo.left, output.windowInfo.left);
    EXPECT_EQ(input.windowInfo.width, output.windowInfo.width);
    EXPECT_EQ(input.windowInfo.height, output.windowInfo.height);
}

TEST(ITypesUtilTest, PanelStatusInfoMarshallingUnmarshalling) {
    PanelStatusInfo input;
    input.panelInfo.panelFlag = PanelFlag::FLAG_1;
    input.panelInfo.panelType = PanelType::TYPE_1;
    input.visible = true;
    input.trigger = Trigger::TRIGGER_1;

    PanelStatusInfo output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input.panelInfo.panelFlag, output.panelInfo.panelFlag);
    EXPECT_EQ(input.panelInfo.panelType, output.panelInfo.panelType);
    EXPECT_EQ(input.visible, output.visible);
    EXPECT_EQ(input.trigger, output.trigger);
}

TEST(ITypesUtilTest, SysPanelStatusMarshallingUnmarshalling) {
    SysPanelStatus input;
    input.inputType = InputType::TYPE_1;
    input.flag = 1;
    input.width = 123;
    input.height = 456;

    SysPanelStatus output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input.inputType, output.inputType);
    EXPECT_EQ(input.flag, output.flag);
    EXPECT_EQ(input.width, output.width);
    EXPECT_EQ(input.height, output.height);
}

TEST(ITypesUtilTest, ElementNameMarshallingUnmarshalling) {
    OHOS::AppExecFwk::ElementName input("bundleName", "moduleName", "abilityName");
    OHOS::AppExecFwk::ElementName output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input.GetBundleName(), output.GetBundleName());
    EXPECT_EQ(input.GetModuleName(), output.GetModuleName());
    EXPECT_EQ(input.GetAbilityName(), output.GetAbilityName());
}

TEST(ITypesUtilTest, InputTypeMarshallingUnmarshalling) {
    InputType input = InputType::TYPE_1;
    InputType output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input, output);
}

TEST(ITypesUtilTest, PanelInfoMarshallingUnmarshalling) {
    PanelInfo input;
    input.panelFlag = PanelFlag::FLAG_1;
    input.panelType = PanelType::TYPE_1;

    PanelInfo output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input.panelFlag, output.panelFlag);
    EXPECT_EQ(input.panelType, output.panelType);
}

TEST(ITypesUtilTest, ClientStateMarshallingUnmarshalling) {
    ClientState input = ClientState::STATE_1;
    ClientState output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input, output);
}

TEST(ITypesUtilTest, SwitchTriggerMarshallingUnmarshalling) {
    SwitchTrigger input = SwitchTrigger::TRIGGER_1;
    SwitchTrigger output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input, output);
}

TEST(ITypesUtilTest, PrivateDataValueMarshallingUnmarshalling) {
    PrivateDataValue input = std::string("test");
    PrivateDataValue output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(std::get<std::string>(input), std::get<std::string>(output));

    input = true;
    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(std::get<bool>(input), std::get<bool>(output));

    input = 123;
    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(std::get<int32_t>(input), std::get<int32_t>(output));
}

TEST(ITypesUtilTest, RangeMarshallingUnmarshalling) {
    Range input = {1, 2};
    Range output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input.start, output.start);
    EXPECT_EQ(input.end, output.end);
}

TEST(ITypesUtilTest, ArrayBufferMarshallingUnmarshalling) {
    ArrayBuffer input = {1, 2, 3};
    ArrayBuffer output;
    MessageParcel data;

    EXPECT_TRUE(ITypesUtil::Marshalling(input, data));
    EXPECT_TRUE(ITypesUtil::Unmarshalling(output, data));
    EXPECT_EQ(input.msgId, output.msgId);
    EXPECT_EQ(input.msgParam, output.msgParam);
    EXPECT_EQ(input.jsArgc, output.jsArgc);
}
