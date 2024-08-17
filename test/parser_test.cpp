#include <gtest/gtest.h>
#include "errorHandler.h"
#include "parser.h"

class ParserTests : public ::testing::Test
{
protected:
    Parser parserInstance;

    virtual void SetUp() {
        parserInstance.lineNumber = 1;
        parserInstance.cursor = 0;
        parserInstance.currCmd.type = CMD_UNDEFINED;
    }

    virtual void TearDown() {
        parser_close(&parserInstance);
    }

    void parser_setContent(const char* str) {
        parserInstance.content = strdup(str);
        parserInstance.contentLen = strlen(str);
    }
};

TEST(ParserSetupTest, GivenValidFileNameThenParserIsConstructedSuccessfully) 
{
    ErrorCode err;
    Parser p;
    err = parser_new(&p, "test/testFiles/test_1.vm");
    ASSERT_EQ(err, OK);
    parser_close(&p);
}

TEST_F(ParserTests, GivenValidArithmeticCommandsThenParsingSucceeds) 
{
    ErrorCode err;
    const char* program = "// Test file 1\n  add\n  eq\n  not\n";
    parser_setContent(program);

    err = parser_advance(&parserInstance);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(parserInstance.currCmd.type, CMD_ARITHMETIC);
    EXPECT_STREQ(parserInstance.currCmd.Arg1, "add");

    err = parser_advance(&parserInstance);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(parserInstance.currCmd.type, CMD_ARITHMETIC);
    EXPECT_STREQ(parserInstance.currCmd.Arg1, "eq");

    err = parser_advance(&parserInstance);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(parserInstance.currCmd.type, CMD_ARITHMETIC);
    EXPECT_STREQ(parserInstance.currCmd.Arg1, "not");
}

TEST_F(ParserTests, GivenValidTwoArgCommandsThenParsingSucceeds)
{
    ErrorCode err;
    const char* program = "  push constant 3\n  pop local 2\n";
    parser_setContent(program);

    err = parser_advance(&parserInstance);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(parserInstance.currCmd.type, CMD_PUSH);
    EXPECT_STREQ(parserInstance.currCmd.Arg1, "constant");
    EXPECT_STREQ(parserInstance.currCmd.Arg2, "3");

    err = parser_advance(&parserInstance);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(parserInstance.currCmd.type, CMD_POP);
    EXPECT_STREQ(parserInstance.currCmd.Arg1, "local");
    EXPECT_STREQ(parserInstance.currCmd.Arg2, "2");
}

TEST_F(ParserTests, GivenNoSpaceBetweenArgumentsThenParsingFails)
{
    ErrorCode err;
    const char* program = "labelHELLO";
    parser_setContent(program);

    err = parser_advance(&parserInstance);
    EXPECT_EQ(err, ERR_UNEXPEC_TOKEN);
}

TEST_F(ParserTests, GivenValidFunctionCommandThenParsingSucceeds)
{
    ErrorCode err;
    const char* program = "    function functionName 3\n";
    parser_setContent(program);

    err = parser_advance(&parserInstance);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(parserInstance.currCmd.type, CMD_FUNCTION);
    EXPECT_STREQ(parserInstance.currCmd.Arg1, "functionName");
    EXPECT_STREQ(parserInstance.currCmd.Arg2, "3");
}

TEST_F(ParserTests, GivenValidReturnCommandThenParsingSucceeds)
{
    ErrorCode err;
    const char* program = "    return\n    return";
    parser_setContent(program);

    err = parser_advance(&parserInstance);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(parserInstance.currCmd.type, CMD_RETURN);

    err = parser_advance(&parserInstance);
    ASSERT_EQ(err, OK);
    EXPECT_EQ(parserInstance.currCmd.type, CMD_RETURN);
}

