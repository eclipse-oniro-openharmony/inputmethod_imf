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

#include "inputmethod_dump.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

using namespace OHOS::MiscServices;
using ::testing::_;
using ::testing::Invoke;
using ::testing::StrEq;

class MockInputmethodDump : public InputmethodDump {
public:
    MOCK_METHOD1(DumpAllMethodMock, void(int fd));
    void DumpAllMethod(int fd) override
    {
        DumpAllMethodMock(fd);
    }
};

HWTEST_F(InputmethodDumpTest, DumpHelpCommand)
{
    MockInputmethodDump mockDump;
    std::vector<std::string> args = {"-h"};
    int pipefd[2];
    pipe(pipefd);
    mockDump.Dump(pipefd[1], args);
    close(pipefd[1]);

    std::ostringstream oss;
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        oss << buffer;
    }
    close(pipefd[0]);

    EXPECT_EQ(oss.str(), "Usage:dump  <command> [options]\nDescription:\n-h show help\n-a dump all input methods\n");
}

HWTEST_F(InputmethodDumpTest, DumpAllCommand)
{
    MockInputmethodDump mockDump;
    std::vector<std::string> args = {"-a"};
    int pipefd[2];
    pipe(pipefd);
    EXPECT_CALL(mockDump, DumpAllMethodMock(pipefd[1])).Times(1);
    mockDump.Dump(pipefd[1], args);
    close(pipefd[1]);
    close(pipefd[0]);
}

HWTEST_F(InputmethodDumpTest, DumpIllegalCommand)
{
    MockInputmethodDump mockDump;
    std::vector<std::string> args = {"-x"};
    int pipefd[2];
    pipe(pipefd);
    mockDump.Dump(pipefd[1], args);
    close(pipefd[1]);

    std::ostringstream oss;
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        oss << buffer;
    }
    close(pipefd[0]);

    EXPECT_EQ(oss.str(), "input dump parameter error,enter '-h' for usage.\n");
}

HWTEST_F(InputmethodDumpTest, DumpNoCommand)
{
    MockInputmethodDump mockDump;
    std::vector<std::string> args = {};
    int pipefd[2];
    pipe(pipefd);
    mockDump.Dump(pipefd[1], args);
    close(pipefd[1]);

    std::ostringstream oss;
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        oss << buffer;
    }
    close(pipefd[0]);

    EXPECT_EQ(oss.str(), "input dump parameter error,enter '-h' for usage.\n");
}
