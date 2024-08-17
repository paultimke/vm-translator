#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "keywords.h"
#include "errorHandler.h"

#define ARG_1           (1)
#define ARG_2           (2)

#define PUSH_STRLEN     (4)
#define POP_STRLEN      (3)
#define LABEL_STRLEN    (5)
#define GOTO_STRLEN     (4)
#define IF_GOTO_STRLEN  (7)

// Local function prototypes
static ErrorCode readInputFile(Parser* p, const char* fileName);
static ErrorCode parser_parseComment(Parser* p);
static ErrorCode parser_parseArg(Parser* p, int arg);
static ErrorCode parser_parseOneArgCommand(Parser* p, CommandType cmdType);
static ErrorCode parser_parseTwoArgCommand(Parser* p, CommandType cmdType);
static bool isEOF(Parser* p);
static bool isEOL(Parser* p);
static uint32_t str2int(const char* str, size_t len);
static void parser_trimLeft(Parser* p);
static void parser_trimLeftInline(Parser *p);
static void parser_consumeChar(Parser* p);
static bool isSpace(const char c);
static bool isInlineSpace(const char c);
static bool isValidSymbolStart(const char c);
static bool isValidSymbolChar(const char c);
static bool lineStartsWith(Parser* p, const char* str);

const char* commandKeywords[CMD_MAX_COMMANDS] = {
    "",         // CMD_UNDEFINED
    "",         // CMD_ARITHMETIC (varies with each one)
    "push",     // CMD_PUSH
    "pop",      // CMD_POP
    "label",    // CMD_LABEL
    "goto",     // CMD_GOTO
    "if-goto",  // CMD_IF
    "function", // CMD_FUNCTION
    "return",   // CMD_RETURN
    "call",     // CMD_CALL
    ""          // CMD_END
};

// ---------------------------- PUBLIC FUNCTIONS --------------------------- //
ErrorCode parser_new(Parser* p, const char* fileName)
{
    ErrorCode err;
    err = readInputFile(p, fileName);
    if (err != OK) return err;

    p->contentLen = strlen(p->content);
    p->lineNumber = 1;
    p->cursor = 0;
    p->currCmd.type = CMD_UNDEFINED;
    return OK;
}

ErrorCode parser_close(Parser* p)
{
    assert(p->content != NULL);
    free((char*)p->content);
    p->content = NULL;
    return OK;
}

bool parser_hasMoreCommands(Parser* p)
{
    return (p->currCmd.type != CMD_END);
}

ErrorCode parser_advance(Parser* p)
{
    ErrorCode err = ERR_UNKNOWN;
    memset(p->currCmd.Arg1, 0, MAX_IDENTIFIER_LEN);
    memset(p->currCmd.Arg2, 0, MAX_IDENTIFIER_LEN);

    while (!isEOF(p)) {
        parser_trimLeft(p);
        if (isEOF(p)) {
            break; 
        }

        // Comments
        if (p->content[p->cursor] == '/') {
            parser_consumeChar(p);
            err = parser_parseComment(p);
            if (err != OK) {
                return err;
            }
            continue;
        }
        // Memory Segment Commands
        else if (lineStartsWith(p, "push")) {
            return parser_parseTwoArgCommand(p, CMD_PUSH);
        }
        else if (lineStartsWith(p, "pop")) {
            return parser_parseTwoArgCommand(p, CMD_POP);
        }
        // Branching commands
        else if (lineStartsWith(p, "label")) {
            return parser_parseOneArgCommand(p, CMD_LABEL);
        }
        else if (lineStartsWith(p, "goto")) {
            return parser_parseOneArgCommand(p, CMD_GOTO);
        }
        else if (lineStartsWith(p, "if-goto")) {
            return parser_parseOneArgCommand(p, CMD_IF);
        }

        // Functions
        else if (lineStartsWith(p, "function")) {
            return parser_parseTwoArgCommand(p, CMD_FUNCTION);
        }
        else if (lineStartsWith(p, "call")) {
            return parser_parseTwoArgCommand(p, CMD_CALL);
        }
        else if (lineStartsWith(p, "return")) {
            // Return doesn't have any arguments
            for (int i = 0; i < strlen("return"); i++) {
                parser_consumeChar(p);
            }
            parser_trimLeftInline(p);
            if (isEOF(p) || isEOL(p)) {
                p->currCmd.type = CMD_RETURN;
                return OK;
            }
            return ERR_UNEXPEC_TOKEN;
        }

        // Arithmetic commands
        for (int i = 0; i < KW_NUM_OF_ARITHMETIC_KEYWORDS; i++) {
            if (lineStartsWith(p, KW_arithmeticProgramKeywords[i])) {
                p->currCmd.type = CMD_ARITHMETIC;
                return parser_parseArg(p, ARG_1);
            }
        }
    }

    p->currCmd.type = CMD_END;
    return OK;
}

// -------------------------- PRIVATE FUNCTIONS ----------------------------- //
static ErrorCode readInputFile(Parser* p, const char* fileName)
{
    char* buffer;
    buffer = NULL;

    if (strcmp(fileName + strlen(fileName) - strlen(".vm"), ".vm") != 0) {
        logError(ERR_FILENAME_NOT_VM, NULL);
        return ERR_FILENAME_NOT_VM;
    }

    FILE* f = fopen(fileName, "rb");
    if (!f) {
        fclose(f);
        logError(ERR_CANT_OPEN_INPUT_FILE, fileName);
        return ERR_FILENAME_NOT_VM;
    }

    fseek(f, 0, SEEK_END);
    uint64_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = malloc(fsize + 1);
    if (buffer == NULL) {
        fclose(f);
        logError(ERR_PROG_OUT_OF_MEMORY, NULL);
        return ERR_PROG_OUT_OF_MEMORY;
    }
    fread(buffer, fsize, 1, f);
    fclose(f);
    buffer[fsize] = '\0';

    p->content = buffer;
    return OK;
}

static bool isEOF(Parser* p)
{
    return p->cursor >= p->contentLen;
}

static bool isEOL(Parser* p)
{
    return p->content[p->cursor] == '\n';
}

static bool isSpace(const char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool isInlineSpace(const char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

static bool isValidSymbolStart(const char c)
{
    return isalpha(c) || c == '_';
}

static bool isValidSymbolChar(const char c)
{
    return isalnum(c) || c == '_' || c == '.' || c == '$';
}

static uint32_t str2int(const char* str, size_t len)
{
    uint32_t ret = 0;
    for (int i = 0; i < len; i++) {
        ret = ret*10 + (str[i] - '0');
    }
    return ret;
}

static void parser_trimLeft(Parser* p)
{
    while (!isEOF(p) && isSpace(p->content[p->cursor])) {
        parser_consumeChar(p);
    }
}

static void parser_trimLeftInline(Parser *p)
{
    while (!isEOF(p) && isInlineSpace(p->content[p->cursor])) {
        p->cursor += 1;
    }
}

/// @brief Consumes current character pointed at by the parser
/// and advances to the next character.
static void parser_consumeChar(Parser *p)
{
    if (p->cursor >= p->contentLen)
        return;

    if (isEOL(p)) {
        p->lineNumber += 1;
        p->lineStart = p->cursor + 1;
    }
    p->cursor += 1;
}

/// @brief Returns true if current text pointed at by the parser
/// starts with the given string str. False otherwise
static bool lineStartsWith(Parser* p, const char* str)
{
    size_t len = strlen(str);
    if ((p->contentLen - p->cursor) >= len) {
        return (strncmp(&p->content[p->cursor], str, len) == 0);
    }
    return false;
}

static ErrorCode parser_parseComment(Parser* p) 
{
    if (!isEOF(p) && p->content[p->cursor] == '/') {
        while (!isEOF(p) && !isEOL(p)) {
            parser_consumeChar(p);
        }
        parser_consumeChar(p);
    }
    else {
        parser_logError(p, ERR_UNEXPEC_TOKEN);
        return ERR_UNEXPEC_TOKEN;
    }
    return OK;
}

/// @brief Parses the arguments of the current command and places them
/// on the p->currCmd.Arg1 and p->currCmd.Arg2 fields for the given parser.
/// In case of an arithmetic command, Arg1 holds the name of the command
/// itself.
/// In all other cases, it holds actual arguments like labels or constants
static ErrorCode parser_parseArg(Parser* p, int arg)
{
    uint64_t argStart = p->cursor;
    uint8_t identifierLen = 0;
    while (!isEOF(p) && !isSpace(p->content[p->cursor])) {
        if (identifierLen >= MAX_IDENTIFIER_LEN) {
            parser_logError(p, ERR_MAX_IDENTIFIER_LEN);
            return ERR_MAX_IDENTIFIER_LEN;
        }
        else if (!isValidSymbolChar(p->content[p->cursor])) {
            parser_logError(p, ERR_UNEXPEC_TOKEN);
            return ERR_UNEXPEC_TOKEN;
        }
        parser_consumeChar(p);
        identifierLen++;
    }
    if (arg == ARG_1)
        strncpy(p->currCmd.Arg1, &p->content[argStart], identifierLen);
    else if (arg == ARG_2)
        strncpy(p->currCmd.Arg2, &p->content[argStart], identifierLen);
    else
        return ERR_UNKNOWN;
    
    return OK;
}

/// @ brief Parses a one-argument command by identifying the type of command,
/// verifying syntax is correct and placing the argument on the Arg2 field
/// of the Command struct.
static ErrorCode parser_parseOneArgCommand(Parser* p, CommandType cmdType)
{
    int commandLen = strlen(commandKeywords[cmdType]);
    for (int i = 0; i < commandLen; i++) {
        parser_consumeChar(p);
    }

    // Assert space is given after the command
    if (!isInlineSpace(p->content[p->cursor])) {
        parser_logError(p, ERR_UNEXPEC_TOKEN);
        return ERR_UNEXPEC_TOKEN;
    }
    parser_trimLeftInline(p);

    ErrorCode err = parser_parseArg(p, ARG_1);
    if (err != OK) return err;

    p->currCmd.type = cmdType;
    return OK;
}

/// @ brief Parses a two-argument command by identifying the type of command,
/// verifying syntax is correct and placing the arguments on the respective
/// Arg1 and Arg2 fields of the Command struct
static ErrorCode parser_parseTwoArgCommand(Parser* p, CommandType cmdType)
{
    ErrorCode err = ERR_UNKNOWN;

    int commandLen = strlen(commandKeywords[cmdType]);
    if (commandLen == 0) {
        p->currCmd.type = CMD_UNDEFINED;
        parser_logError(p, ERR_UNEXPEC_TOKEN);
        return ERR_UNEXPEC_TOKEN;
    }
    for (int i = 0; i < commandLen; i++) {
        parser_consumeChar(p);
    }

    // Assert space is given after the command
    if (!isInlineSpace(p->content[p->cursor])) {
        parser_logError(p, ERR_UNEXPEC_TOKEN);
        return ERR_UNEXPEC_TOKEN;
    }
    parser_trimLeftInline(p);

    // Parse first argument
    err = parser_parseArg(p, ARG_1);
    if (err != OK) return err;

    // Assert space is given after the first arg
    if (!isInlineSpace(p->content[p->cursor])) {
        parser_logError(p, ERR_UNEXPEC_TOKEN);
        return ERR_UNEXPEC_TOKEN;
    }
    parser_trimLeftInline(p);

    // Parse second argument
    err = parser_parseArg(p, ARG_2);
    if (err != OK) return err;

    p->currCmd.type = cmdType;
    return OK;
}

