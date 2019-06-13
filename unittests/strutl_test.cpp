#include <string>

#include "gtest/gtest.h"
#include "strutl.h"

using namespace std;
using namespace strutl;

TEST(strutl, testMatch)
{
    EXPECT_TRUE(match(string("one"), string("one")));
    EXPECT_TRUE(match(string("123"), string("123")));
    EXPECT_FALSE(match(string("1"), string("0")));
    EXPECT_TRUE(match(string("123"), string("*")));
    EXPECT_TRUE(match(string("123"), string("???")));
    EXPECT_TRUE(match(string("123"), string("1*")));
    EXPECT_TRUE(match(string("header"), string("header")));
    EXPECT_TRUE(match(string("header"), string("header*")));
    EXPECT_TRUE(match(string("XXX${NAME}YYY"), string("*${*}*")));
}

TEST(strutl, testToken)
{
    const string str("1:2:3");
    const string sep(":");
    const string::size_type tokens_no = 3;
    Tokens tokens = split(str, sep);
    EXPECT_EQ(tokens.size(), tokens_no);
    EXPECT_STREQ(tokens[0].c_str(), "1");
    EXPECT_STREQ(tokens[1].c_str(), "2");
    EXPECT_STREQ(tokens[2].c_str(), "3");
}

TEST(strutl, testTokenFail)
{
    const string str("1:2:3");
    const string sep(",");
    const string::size_type tokens_no = 3;
    Tokens tokens = split(str, sep);
    EXPECT_NE(tokens.size(), tokens_no);
}
