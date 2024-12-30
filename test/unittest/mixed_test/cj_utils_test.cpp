/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstring>
#include <gtest/gtest.h>
#include <string>

TEST(MallocCString, EmptyString_ReturnsNullptr)
{
    std::string input = "";
    char *result = Utils::MallocCString(input);
    EXPECT_EQ(result, nullptr);
}

TEST(MallocCString, NonEmptyString_ReturnsCString)
{
    std::string input = "Hello, World!";
    char *result = Utils::MallocCString(input);
    EXPECT_NE(result, nullptr);
    EXPECT_STREQ(result, input.c_str());
    free(result); // 释放分配的内存
}

// Unit Test Code:
TEST(InputMethodProperty2C, NormalCase)
{
    Property property = { "name", "id", "label", 1, "icon", 2 };
    CInputMethodProperty props;
    Utils::InputMethodProperty2C(props, property);

    EXPECT_STREQ(props.name, "name");
    EXPECT_STREQ(props.id, "id");
    EXPECT_STREQ(props.label, "label");
    EXPECT_EQ(props.labelId, 1);
    EXPECT_STREQ(props.icon, "icon");
    EXPECT_EQ(props.iconId, 2);

    // 释放内存
    delete[] props.name;
    delete[] props.id;
    delete[] props.label;
    delete[] props.icon;
}

TEST(InputMethodProperty2C, EmptyFields)
{
    Property property = { "", "", "", 0, "", 0 };
    CInputMethodProperty props;
    Utils::InputMethodProperty2C(props, property);

    EXPECT_STREQ(props.name, "");
    EXPECT_STREQ(props.id, "");
    EXPECT_STREQ(props.label, "");
    EXPECT_EQ(props.labelId, 0);
    EXPECT_STREQ(props.icon, "");
    EXPECT_EQ(props.iconId, 0);

    // 释放内存
    delete[] props.name;
    delete[] props.id;
    delete[] props.label;
    delete[] props.icon;
}

TEST(InputMethodProperty2C, NullFields)
{
    Property property = { nullptr, nullptr, nullptr, 0, nullptr, 0 };
    CInputMethodProperty props;
    Utils::InputMethodProperty2C(props, property);

    EXPECT_EQ(props.name, nullptr);
    EXPECT_EQ(props.id, nullptr);
    EXPECT_EQ(props.label, nullptr);
    EXPECT_EQ(props.labelId, 0);
    EXPECT_EQ(props.icon, nullptr);
    EXPECT_EQ(props.iconId, 0);

    // 释放内存
    delete[] props.name;
    delete[] props.id;
    delete[] props.label;
    delete[] props.icon;
}

TEST(C2InputMethodPropertyTest, EmptyInput_ShouldReturnEmptyProperty)
{
    CInputMethodProperty props;
    Property property = Utils::C2InputMethodProperty(props);
    EXPECT_EQ(property.name, "");
    EXPECT_EQ(property.id, "");
    EXPECT_EQ(property.label, "");
    EXPECT_EQ(property.labelId, 0);
    EXPECT_EQ(property.icon, "");
    EXPECT_EQ(property.iconId, 0);
}

TEST(C2InputMethodPropertyTest, NonEmptyInput_ShouldReturnPopulatedProperty)
{
    CInputMethodProperty props;
    props.name = "name";
    props.id = "id";
    props.label = "label";
    props.labelId = 1;
    props.icon = "icon";
    props.iconId = 2;

    Property property = Utils::C2InputMethodProperty(props);
    EXPECT_EQ(property.name, "name");
    EXPECT_EQ(property.id, "id");
    EXPECT_EQ(property.label, "label");
    EXPECT_EQ(property.labelId, 1);
    EXPECT_EQ(property.icon, "icon");
    EXPECT_EQ(property.iconId, 2);
}

TEST(C2InputMethodPropertyTest, MixedInput_ShouldReturnPartiallyPopulatedProperty)
{
    CInputMethodProperty props;
    props.name = "name";
    props.labelId = 1;

    Property property = Utils::C2InputMethodProperty(props);
    EXPECT_EQ(property.name, "name");
    EXPECT_EQ(property.id, "");
    EXPECT_EQ(property.label, "");
    EXPECT_EQ(property.labelId, 1);
    EXPECT_EQ(property.icon, "");
    EXPECT_EQ(property.iconId, 0);
}

TEST(InputMethodSubProperty2C, AllFieldsEmpty)
{
    SubProperty property = { "", "", "", 0, "", 0, "", "", "" };
    CInputMethodSubtype props;
    Utils::InputMethodSubProperty2C(props, property);

    EXPECT_STREQ(props.name, "");
    EXPECT_STREQ(props.id, "");
    EXPECT_STREQ(props.label, "");
    EXPECT_EQ(props.labelId, 0);
    EXPECT_STREQ(props.icon, "");
    EXPECT_EQ(props.iconId, 0);
    EXPECT_STREQ(props.mode, "");
    EXPECT_STREQ(props.locale, "");
    EXPECT_STREQ(props.language, "");
}

TEST(InputMethodSubProperty2C, AllFieldsFilled)
{
    SubProperty property = { "name", "id", "label", 1, "icon", 2, "mode", "locale", "language" };
    CInputMethodSubtype props;
    Utils::InputMethodSubProperty2C(props, property);

    EXPECT_STREQ(props.name, "name");
    EXPECT_STREQ(props.id, "id");
    EXPECT_STREQ(props.label, "label");
    EXPECT_EQ(props.labelId, 1);
    EXPECT_STREQ(props.icon, "icon");
    EXPECT_EQ(props.iconId, 2);
    EXPECT_STREQ(props.mode, "mode");
    EXPECT_STREQ(props.locale, "locale");
    EXPECT_STREQ(props.language, "language");
}

TEST(InputMethodSubProperty2C, MixedFields)
{
    SubProperty property = { "name", "", "label", 1, "", 2, "mode", "", "language" };
    CInputMethodSubtype props;
    Utils::InputMethodSubProperty2C(props, property);

    EXPECT_STREQ(props.name, "name");
    EXPECT_STREQ(props.id, "");
    EXPECT_STREQ(props.label, "label");
    EXPECT_EQ(props.labelId, 1);
    EXPECT_STREQ(props.icon, "");
    EXPECT_EQ(props.iconId, 2);
    EXPECT_STREQ(props.mode, "mode");
    EXPECT_STREQ(props.locale, "");
    EXPECT_STREQ(props.language, "language");
}

TEST(InputMethodSubProperty2C, BoundaryValues)
{
    SubProperty property = { "name", "id", "label", INT_MAX, "icon", INT_MIN, "mode", "locale", "language" };
    CInputMethodSubtype props;
    Utils::InputMethodSubProperty2C(props, property);

    EXPECT_STREQ(props.name, "name");
    EXPECT_STREQ(props.id, "id");
    EXPECT_STREQ(props.label, "label");
    EXPECT_EQ(props.labelId, INT_MAX);
    EXPECT_STREQ(props.icon, "icon");
    EXPECT_EQ(props.iconId, INT_MIN);
    EXPECT_STREQ(props.mode, "mode");
    EXPECT_STREQ(props.locale, "locale");
    EXPECT_STREQ(props.language, "language");
}
