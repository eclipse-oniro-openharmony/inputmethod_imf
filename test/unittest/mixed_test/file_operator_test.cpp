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
#include "file_operator_test.h"

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>

#include <climits>
#include <fstream>

#include "global.h"

using namespace OHOS::MiscServices;
using namespace testing;

class FileOperatorTest : public Test {
public:
    static void SetUpTestCase()
    {
        // set shared resources before all tests
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
};

/**
 * @tc.name: Create_DirectoryCreationFails_ReturnsFalse
 * @tc.desc: Verify that FileOperator's Create method returns false when directory creation fails
 * @tc.type: FUNC
 */
TEST_F(FileOperatorTest, Create_DirectoryCreationFails_ReturnsFalse)
{
    FileOperator fileOperator;
    EXPECT_CALL(fileOperator, mkdir(_, _)).WillOnce(Return(-1));
    EXPECT_CALL(fileOperator, errno).WillOnce(Return(EEXIST));
    EXPECT_FALSE(fileOperator.Create("testDir", 0777));
}

/**
 * @tc.name: IsExist_FileExists_ReturnsTrue
 * @tc.desc: Verify that FileOperator's IsExist method returns true when the file exists
 * @tc.type: FUNC
 */
TEST_F(FileOperatorTest, IsExist_FileExists_ReturnsTrue)
{
    FileOperator fileOperator;
    EXPECT_CALL(fileOperator, access(_, F_OK)).WillOnce(Return(0));
    EXPECT_TRUE(fileOperator.IsExist("testFile"));
}

/**
 * @tc.name: Read_FileOpenFails_ReturnsFalse
 * @tc.desc: Verify that FileOperator's Read method returns false when file opening fails
 * @tc.type: FUNC
 */
TEST_F(FileOperatorTest, Read_FileOpenFails_ReturnsFalse)
{
    FileOperator fileOperator;
    EXPECT_CALL(fileOperator, ifstream::open(_)).WillOnce(SetErrnoAndReturn(EACCES));
    EXPECT_FALSE(fileOperator.Read("testFile", _));
}

/**
 * @tc.name: Write_FileOpenFails_ReturnsFalse
 * @tc.desc: Verify that FileOperator's Write method returns false when file opening fails
 * @tc.type: FUNC
 */
TEST_F(FileOperatorTest, Write_FileOpenFails_ReturnsFalse)
{
    FileOperator fileOperator;
    EXPECT_CALL(fileOperator, open(_, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(fileOperator, errno).WillOnce(Return(EACCES));
    EXPECT_FALSE(fileOperator.Write("testFile", "content", O_WRONLY, 0666));
}

/**
 * @tc.name: Read_KeyEmpty_ReturnsFalse
 * @tc.desc: Verify that FileOperator's Read method returns false when the key is empty
 * @tc.type: FUNC
 */
TEST_F(FileOperatorTest, Read_KeyEmpty_ReturnsFalse)
{
    FileOperator fileOperator;
    EXPECT_FALSE(fileOperator.Read("testFile", "", _));
}

/**
 * @tc.name: Read_KeyFound_ReturnsTrue
 * @tc.desc: Verify that FileOperator's Read method returns true when the key is found
 * @tc.type: FUNC
 */
TEST_F(FileOperatorTest, Read_KeyFound_ReturnsTrue)
{
    FileOperator fileOperator;
    EXPECT_CALL(fileOperator, GetCfgFiles(_)).WillOnce(Return(cfgFilesMock));
    EXPECT_CALL(fileOperator, Read(_, _)).WillOnce(Return("key=value"));
    EXPECT_TRUE(fileOperator.Read("testFile", "key", _));
}

/**
 * @tc.name: Read_KeyNotFound_ReturnsFalse
 * @tc.desc: Verify that FileOperator's Read method returns false when the key is not found
 * @tc.type: FUNC
 */
TEST_F(FileOperatorTest, Read_KeyNotFound_ReturnsFalse)
{
    FileOperator fileOperator;
    EXPECT_CALL(fileOperator, GetCfgFiles(_)).WillOnce(Return(cfgFilesMock));
    EXPECT_CALL(fileOperator, Read(_, _)).WillOnce(Return(""));
    EXPECT_FALSE(fileOperator.Read("testFile", "key", _));
}

/**
 * @tc.name: GetRealPath_ValidPath_ReturnsRealPath
 * @tc.desc: Verify that FileOperator's GetRealPath method returns the real path when the path is valid
 * @tc.type: FUNC
 */
TEST_F(FileOperatorTest, GetRealPath_ValidPath_ReturnsRealPath)
{
    FileOperator fileOperator;
    EXPECT_CALL(fileOperator, realpath(_, _)).WillOnce(Return(realPathMock));
    EXPECT_EQ(fileOperator.GetRealPath("testPath"), realPathMock);
}

/**
 * @tc.name: GetRealPath_InvalidPath_ReturnsEmptyString
 * @tc.desc: Verify that FileOperator's GetRealPath method returns an empty string when the path is invalid
 * @tc.type: FUNC
 */
TEST_F(FileOperatorTest, GetRealPath_InvalidPath_ReturnsEmptyString)
{
    FileOperator fileOperator;
    EXPECT_CALL(fileOperator, realpath(_, _)).WillOnce(Return(nullptr));
    EXPECT_EQ(fileOperator.GetRealPath("invalidPath"), "");
}