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

#include "peruser_session.h"

#include <gtest/gtest.h>

#include <memory>

#include "input_client_info.h"
#include "mock_input_client.h"
#include "mock_input_method_core.h"

using namespace OHOS;
using namespace MiscServices;
using namespace testing;

class PerUserSessionTest : public Test {
protected:
    static void SetUpTestCase()
    {
        userId = 100;
        eventHandler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create(true));
        perUserSession = std::make_unique<PerUserSession>(userId, eventHandler);
    }

    static void TearDownTestCase()
    {
        // clear shared resources after all tests
    }

    void SetUp() override
    {
        // prepare resource before each test
    }

    void TearDown() override
    {
        // clear resource after each test
    }
    int userId;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler;
    std::unique_ptr<PerUserSession> perUserSession;
};
/**
 * @tc.name: AddClientInfoTest
 * @tc.desc: Verify that the PerUserSession::AddClientInfo method
 * returns ErrorCode::NO_ERROR when adding client information
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, AddClientInfoTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;

    EXPECT_EQ(perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: RemoveClientInfoTest
 * @tc.desc: Verify that the PerUserSession::RemoveClientInfo method works normally when removing client information
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, RemoveClientInfoTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);
    perUserSession->RemoveClientInfo(mockClient, false);
}

/**
 * @tc.name: UpdateClientInfoTest
 * @tc.desc: Verify that the PerUserSession::UpdateClientInfo method works normally when updating client information
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, UpdateClientInfoTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);

    std::unordered_map<UpdateFlag, std::variant<bool, uint32_t, ImeType, ClientState, TextTotalConfig>> updateInfos;
    updateInfos[UpdateFlag::ISSHOWKEYBOARD] = true;

    perUserSession->UpdateClientInfo(mockClient, updateInfos);
}

/**
 * @tc.name: HideKeyboardTest
 * @tc.desc: Verify that the PerUserSession::HideKeyboard method returns ErrorCode::NO_ERROR when hiding the keyboard
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, HideKeyboardTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;
    clientInfo.bindImeType = ImeType::IME;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);

    EXPECT_CALL(*mockClient, AsObject()).WillOnce(Return(mockClient->AsObject()));
    EXPECT_EQ(perUserSession->HideKeyboard(mockClient), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: ShowKeyboardTest
 * @tc.desc: Verify that the PerUserSession::ShowKeyboard method returns ErrorCode::NO_ERROR when showing the keyboard
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, ShowKeyboardTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;
    clientInfo.bindImeType = ImeType::IME;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);

    EXPECT_CALL(*mockClient, AsObject()).WillOnce(Return(mockClient->AsObject()));
    EXPECT_EQ(perUserSession->ShowKeyboard(mockClient), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: OnClientDiedTest
 * @tc.desc: Verify that the PerUserSession::OnClientDied method works normally when the client dies
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, OnClientDiedTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;
    clientInfo.bindImeType = ImeType::IME;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);
    perUserSession->OnClientDied(mockClient);
}

/**
 * @tc.name: OnImeDiedTest
 * @tc.desc: Verify that the PerUserSession::OnImeDied method works normally when the input method dies
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, OnImeDiedTest)
{
    auto mockImeCore = std::make_shared<MockInputMethodCore>();
    perUserSession->OnImeDied(mockImeCore, ImeType::IME);
}

/**
 * @tc.name: RemoveImeTest
 * @tc.desc: Verify that the PerUserSession::RemoveIme method
 * returns ErrorCode::ERROR_IME_NOT_STARTED when removing the input method
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, RemoveImeTest)
{
    auto mockImeCore = std::make_shared<MockInputMethodCore>();
    EXPECT_EQ(perUserSession->RemoveIme(mockImeCore, ImeType::IME), ErrorCode::ERROR_IME_NOT_STARTED);
}

/**
 * @tc.name: OnHideCurrentInputTest
 * @tc.desc: Verify that the PerUserSession::OnHideCurrentInput method
 * returns ErrorCode::NO_ERROR when hiding the current input
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, OnHideCurrentInputTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;
    clientInfo.bindImeType = ImeType::IME;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);
    perUserSession->SetCurrentClient(mockClient);
    EXPECT_EQ(perUserSession->OnHideCurrentInput(), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: OnShowCurrentInputTest
 * @tc.desc: Verify that the PerUserSession::OnShowCurrentInput method
 * returns ErrorCode::NO_ERROR when showing the current input
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, OnShowCurrentInputTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;
    clientInfo.bindImeType = ImeType::IME;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);
    perUserSession->SetCurrentClient(mockClient);
    EXPECT_EQ(perUserSession->OnShowCurrentInput(), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: OnHideInputTest
 * @tc.desc: Verify that the PerUserSession::OnHideInput method returns ErrorCode::NO_ERROR when hiding the input
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, OnHideInputTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;
    clientInfo.bindImeType = ImeType::IME;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);
    perUserSession->SetCurrentClient(mockClient);
    EXPECT_EQ(perUserSession->OnHideInput(mockClient), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: OnShowInputTest
 * @tc.desc: Verify that the PerUserSession::OnShowInput method returns ErrorCode::NO_ERROR when showing the input
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, OnShowInputTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;
    clientInfo.bindImeType = ImeType::IME;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);
    perUserSession->SetCurrentClient(mockClient);
    EXPECT_EQ(perUserSession->OnShowInput(mockClient), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: OnHideSoftKeyBoardSelfTest
 * @tc.desc: Verify that the PerUserSession::OnHideSoftKeyBoardSelf method works normally
 * when hiding the soft keyboard
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, OnHideSoftKeyBoardSelfTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;
    clientInfo.bindImeType = ImeType::IME;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);
    perUserSession->SetCurrentClient(mockClient);
    perUserSession->OnHideSoftKeyBoardSelf();
}

/**
 * @tc.name: OnRequestShowInputTest
 * @tc.desc: Verify that the PerUserSession::OnRequestShowInput method
 * returns ErrorCode::ERROR_IME_NOT_STARTED when requesting to show the input
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, OnRequestShowInputTest)
{
    EXPECT_EQ(perUserSession->OnRequestShowInput(), ErrorCode::ERROR_IME_NOT_STARTED);
}

/**
 * @tc.name: OnRequestHideInputTest
 * @tc.desc: Verify that the PerUserSession::OnRequestHideInput method
 * returns ErrorCode::NO_ERROR when requesting to hide the input
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, OnRequestHideInputTest)
{
    EXPECT_EQ(perUserSession->OnRequestHideInput(getpid()), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: OnPrepareInputTest
 * @tc.desc: Verify that the PerUserSession::OnPrepareInput method
 * returns ErrorCode::NO_ERROR when preparing the input
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, OnPrepareInputTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;

    EXPECT_EQ(perUserSession->OnPrepareInput(clientInfo), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: OnReleaseInputTest
 * @tc.desc: Verify that the PerUserSession::OnReleaseInput method returns ErrorCode::NO_ERROR when releasing the input
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, OnReleaseInputTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);
    EXPECT_EQ(perUserSession->OnReleaseInput(mockClient), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: RemoveClientTest
 * @tc.desc: Verify that the PerUserSession::RemoveClient method returns ErrorCode::NO_ERROR when removing the client
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, RemoveClientTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);
    EXPECT_EQ(perUserSession->RemoveClient(mockClient, true), ErrorCode::NO_ERROR);
}

/**
 * @tc.name: DeactivateClientTest
 * @tc.desc: Verify that the PerUserSession::DeactivateClient method works normally when deactivating the client
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, DeactivateClientTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;

    perUserSession->AddClientInfo(mockClient, clientInfo, START_LISTENING);
    perUserSession->DeactivateClient(mockClient);
}

/**
 * @tc.name: IsProxyImeEnableTest
 * @tc.desc: Verify that the PerUserSession::IsProxyImeEnable method
 * returns false when checking if the proxy IME is enabled
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, IsProxyImeEnableTest)
{
    EXPECT_FALSE(perUserSession->IsProxyImeEnable());
}

/**
 * @tc.name: OnStartInputTest
 * @tc.desc: Verify that the PerUserSession::OnStartInput method
 * returns ErrorCode::ERROR_IME_NOT_STARTED when starting the input
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, OnStartInputTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;

    sptr<IRemoteObject> agent;
    EXPECT_EQ(perUserSession->OnStartInput(clientInfo, agent), ErrorCode::ERROR_IME_NOT_STARTED);
}

/**
 * @tc.name: BindClientWithImeTest
 * @tc.desc: Verify that the PerUserSession::BindClientWithIme method
 * returns ErrorCode::ERROR_IME_NOT_STARTED when binding the client with the input method
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, BindClientWithImeTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;

    EXPECT_EQ(perUserSession->BindClientWithIme(std::make_shared<InputClientInfo>(clientInfo), ImeType::IME, true),
        ErrorCode::ERROR_IME_NOT_STARTED);
}

/**
 * @tc.name: UnBindClientWithImeTest
 * @tc.desc: Verify that the PerUserSession::UnBindClientWithIme method works normally
 * when unbinding the client with the input method
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, UnBindClientWithImeTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;

    perUserSession->UnBindClientWithIme(std::make_shared<InputClientInfo>(clientInfo), true);
}

/**
 * @tc.name: StopClientInputTest
 * @tc.desc: Verify that the PerUserSession::StopClientInput method works normally when stopping the client input
 * @tc.type: FUNC
 */
TEST_F(PerUserSessionTest, StopClientInputTest)
{
    auto mockClient = std::make_shared<MockInputClient>();
    InputClientInfo clientInfo;
    clientInfo.client = mockClient;
    clientInfo.config = { /* initialize configuration */ };
    clientInfo.uiExtensionTokenId = 123;

    perUserSession->StopClientInput(std::make_shared<InputClientInfo>(clientInfo));
}