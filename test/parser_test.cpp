#include <gtest/gtest.h>
#include "errorHandler.h"
#include "parser.h"

TEST(ParserTests, GivenValidFileNameThenParserIsConstructedSuccessfully) 
{
    ErrorCode err;
    Parser p;
    err = parser_new(&p, "test/testFiles/test_1.vm");
    ASSERT_EQ(err, OK);
}

TEST(ParserTests, GivenValidProgramThenParsedCommandsAreCorrect) 
{
    ErrorCode err;
    Parser p;
    err = parser_new(&p, "test/testFiles/test_1.vm");
    ASSERT_EQ(err, OK);

    err = parser_advance(&p);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(p.currCmd.type, CMD_PUSH);
    EXPECT_STREQ(p.currCmd.Arg1, "constant");
    EXPECT_STREQ(p.currCmd.Arg2, "3");

    err = parser_advance(&p);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(p.currCmd.type, CMD_POP);
    EXPECT_STREQ(p.currCmd.Arg1, "local");
    EXPECT_STREQ(p.currCmd.Arg2, "2");

    err = parser_advance(&p);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(p.currCmd.type, CMD_ARITHMETIC);
    EXPECT_STREQ(p.currCmd.Arg1, "add");

    err = parser_advance(&p);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(p.currCmd.type, CMD_ARITHMETIC);
    EXPECT_STREQ(p.currCmd.Arg1, "eq");

    err = parser_advance(&p);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(p.currCmd.type, CMD_ARITHMETIC);
    EXPECT_STREQ(p.currCmd.Arg1, "not");
}
