#include <string>

#include "lexical_cast.h"
#include "gtest/gtest.h"

using namespace std;

TEST(lexical_cast, testInt2String)
{
    int value = 123;
    string result = lexical_cast<string>(value);
    EXPECT_STREQ(result.c_str(), "123");
}

TEST(lexical_cast, testString2Int)
{
    string value("123");
    int result = lexical_cast<int>(value);
    EXPECT_EQ(result, 123);
}
