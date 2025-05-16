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

#include "input_method_tools.h"

namespace OHOS {
namespace MiscServices {

InputMethodTools &InputMethodTools::GetInstance()
{
    static InputMethodTools instance;
    return instance;
}

InputAttributeInner InputMethodTools::AttributeToInner(const InputAttribute &attribute)
{
    InputAttributeInner inner;
    inner.inputPattern = attribute.inputPattern;
    inner.inputOption = attribute.inputOption;
    inner.enterKeyType = attribute.enterKeyType;
    inner.isTextPreviewSupported = attribute.isTextPreviewSupported;
    inner.bundleName = attribute.bundleName;
    inner.immersiveMode = attribute.immersiveMode;
    inner.windowId = attribute.windowId;
    inner.callingDisplayId = attribute.callingDisplayId;
    inner.placeholder = attribute.placeholder;
    inner.abilityName = attribute.abilityName;
    return inner;
}

InputAttribute InputMethodTools::InnerToAttribute(const InputAttributeInner &inner)
{
    InputAttribute inputAttribute;
    inputAttribute.inputPattern = inner.inputPattern;
    inputAttribute.inputOption = inner.inputOption;
    inputAttribute.enterKeyType = inner.enterKeyType;
    inputAttribute.isTextPreviewSupported = inner.isTextPreviewSupported;
    inputAttribute.bundleName = inner.bundleName;
    inputAttribute.immersiveMode = inner.immersiveMode;
    inputAttribute.windowId = inner.windowId;
    inputAttribute.callingDisplayId = inner.callingDisplayId;
    inputAttribute.placeholder = inner.placeholder;
    inputAttribute.abilityName = inner.abilityName;
    return inputAttribute;
}

CursorInfoInner InputMethodTools::CursorInfoToInner(const CursorInfo &cursorInfo)
{
    CursorInfoInner inner;
    inner.left = cursorInfo.left;
    inner.top = cursorInfo.top;
    inner.width = cursorInfo.width;
    inner.height = cursorInfo.height;
    return inner;
}

CursorInfo InputMethodTools::InnerToCursorInfo(const CursorInfoInner &inner)
{
    CursorInfo cursorInfo;
    cursorInfo.left = inner.left;
    cursorInfo.top = inner.top;
    cursorInfo.width = inner.width;
    cursorInfo.height = inner.height;
    return cursorInfo;
}

RangeInner InputMethodTools::RangeToInner(const Range &range)
{
    RangeInner inner;
    inner.start = range.start;
    inner.end = range.end;
    return inner;
}
Range InputMethodTools::InnerToRange(const RangeInner &inner)
{
    Range range;
    range.start = inner.start;
    range.end = inner.end;
    return range;
}

TextSelectionInner InputMethodTools::TextSelectionToInner(const TextSelection &textSelection)
{
    TextSelectionInner inner;
    inner.oldBegin = textSelection.oldBegin;
    inner.oldEnd = textSelection.oldEnd;
    inner.newBegin = textSelection.newBegin;
    inner.newEnd = textSelection.newEnd;
    return inner;
}
TextSelection InputMethodTools::InnerToSelection(const TextSelectionInner &inner)
{
    TextSelection textSelection;
    textSelection.oldBegin = inner.oldBegin;
    textSelection.oldEnd = inner.oldEnd;
    textSelection.newBegin = inner.newBegin;
    textSelection.newEnd = inner.newEnd;
    return textSelection;
}

TextTotalConfigInner InputMethodTools::TextTotalConfigToInner(const TextTotalConfig &textTotalConfig)
{
    TextTotalConfigInner inner;
    inner.inputAttribute = AttributeToInner(textTotalConfig.inputAttribute);
    inner.cursorInfo = CursorInfoToInner(textTotalConfig.cursorInfo);
    inner.textSelection = TextSelectionToInner(textTotalConfig.textSelection);
    inner.windowId = textTotalConfig.windowId;
    inner.positionY = textTotalConfig.positionY;
    inner.height = textTotalConfig.height;
    inner.commandValue.valueMap = textTotalConfig.privateCommand;
    inner.requestKeyboardReason = textTotalConfig.requestKeyboardReason;
    return inner;
}

TextTotalConfig InputMethodTools::InnerToTextTotalConfig(const TextTotalConfigInner &inner)
{
    TextTotalConfig textTotalConfig;
    textTotalConfig.inputAttribute = InnerToAttribute(inner.inputAttribute);
    textTotalConfig.cursorInfo = InnerToCursorInfo(inner.cursorInfo);
    textTotalConfig.textSelection = InnerToSelection(inner.textSelection);
    textTotalConfig.windowId = inner.windowId;
    textTotalConfig.positionY = inner.positionY;
    textTotalConfig.height = inner.height;
    textTotalConfig.privateCommand = inner.commandValue.valueMap;
    textTotalConfig.requestKeyboardReason = inner.requestKeyboardReason;
    return textTotalConfig;
}

InputClientInfoInner InputMethodTools::InputClientInfoToInner(const InputClientInfo &inputClientInfo)
{
    InputClientInfoInner inner;
    inner.pid = inputClientInfo.pid;
    inner.uid = inputClientInfo.uid;
    inner.userID = inputClientInfo.userID;
    inner.isShowKeyboard = inputClientInfo.isShowKeyboard;
    inner.bindImeType = inputClientInfo.bindImeType;
    inner.config = TextTotalConfigToInner(inputClientInfo.config);
    inner.eventFlag = inputClientInfo.eventFlag;
    inner.attribute = AttributeToInner(inputClientInfo.attribute);
    inner.client = inputClientInfo.client;
    inner.channel = inputClientInfo.channel;
    inner.deathRecipient = inputClientInfo.deathRecipient;
    inner.state = inputClientInfo.state;
    inner.isNotifyInputStart = inputClientInfo.isNotifyInputStart;
    inner.needHide = inputClientInfo.needHide;
    inner.uiExtensionTokenId = inputClientInfo.uiExtensionTokenId;
    inner.requestKeyboardReason = inputClientInfo.requestKeyboardReason;
    inner.type = inputClientInfo.type;
    inner.name = inputClientInfo.name;
    return inner;
}

InputClientInfo InputMethodTools::InnerToInputClientInfo(const InputClientInfoInner &inner)
{
    InputClientInfo inputClientInfo;
    inputClientInfo.pid = inner.pid;
    inputClientInfo.uid = inner.uid;
    inputClientInfo.userID = inner.userID;
    inputClientInfo.isShowKeyboard = inner.isShowKeyboard;
    inputClientInfo.bindImeType = inner.bindImeType;
    inputClientInfo.config = InnerToTextTotalConfig(inner.config);
    inputClientInfo.eventFlag = inner.eventFlag;
    inputClientInfo.attribute = InnerToAttribute(inner.attribute);
    inputClientInfo.client = inner.client;
    inputClientInfo.channel = inner.channel;
    inputClientInfo.deathRecipient = inner.deathRecipient;
    inputClientInfo.state = inner.state;
    inputClientInfo.isNotifyInputStart = inner.isNotifyInputStart;
    inputClientInfo.needHide = inner.needHide;
    inputClientInfo.uiExtensionTokenId = inner.uiExtensionTokenId;
    inputClientInfo.requestKeyboardReason = inner.requestKeyboardReason;
    inputClientInfo.type = inner.type;
    inputClientInfo.name = inner.name;
    return inputClientInfo;
}

PanelStatusInfoInner InputMethodTools::PanelStatusInfoToInner(const PanelStatusInfo &panelStatusInfo)
{
    PanelStatusInfoInner inner;
    inner.panelInfo = panelStatusInfo.panelInfo;
    inner.visible = panelStatusInfo.visible;
    inner.trigger = panelStatusInfo.trigger;
    inner.sessionId = panelStatusInfo.sessionId;
    return inner;
}

PanelStatusInfo InputMethodTools::InnerToPanelStatusInfo(const PanelStatusInfoInner &inner)
{
    PanelStatusInfo panelStatusInfo;
    panelStatusInfo.panelInfo = inner.panelInfo;
    panelStatusInfo.visible = inner.visible;
    panelStatusInfo.trigger = inner.trigger;
    panelStatusInfo.sessionId = inner.sessionId;
    return panelStatusInfo;
}
} // namespace MiscServices
} // namespace OHOS