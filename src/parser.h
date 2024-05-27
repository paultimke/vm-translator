#ifndef PARSER_H
#define PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "errorHandler.h"

#define MAX_IDENTIFIER_LEN    (120)

typedef enum {
    CMD_UNDEFINED,
    CMD_ARITHMETIC,
    CMD_PUSH,
    CMD_POP,
    CMD_LABEL,
    CMD_GOTO,
    CMD_IF,
    CMD_FUNCTION,
    CMD_RETURN,
    CMD_CALL,
    CMD_END,

    CMD_MAX_COMMANDS
} CommandType;

typedef struct Command {
    CommandType type;
    char Arg1[MAX_IDENTIFIER_LEN]; // Returns command name when type = CMD_ARITHMETIC
    char Arg2[MAX_IDENTIFIER_LEN]; // Only populated when 2 arguments exist
} Command;

typedef struct Parser {
    const char* content;
    uint64_t contentLen;
    uint64_t cursor;
    uint64_t lineNumber;
    uint64_t lineStart;
    Command currCmd;
} Parser;

/// @brief Creates a parser object given a path to a file with .vm extension
/// @param p Pointer to a parser object
/// @param fileName path to input file
ErrorCode parser_new(Parser* p, const char* fileName);

/// @brief Frees allocated memory
/// @param p Pointer to a parser object
ErrorCode parser_close(Parser* p);

ErrorCode parser_advance(Parser* p);

Command parser_getCurrentCommand(Parser* p);

bool parser_hasMoreCommands(Parser* p);

#ifdef __cplusplus
}
#endif

#endif // PARSER_H
