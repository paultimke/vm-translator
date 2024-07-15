#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include "codeWriter.h"
#include "errorHandler.h"
#include "parser.h"

#define EXIT_ON_ERR(err)    ({ErrorCode e = err; if (e != OK) exit(e);})

// Local file-scoped functions and variables
static Parser parser;
static CodeWriter codeWriter;

static void attemptCleanup(void);
static ErrorCode createOutputFile(FILE** file, const char* name);
static ErrorCode changeFileExtension(char** dst, const char* name);

int main(int argc, char* argv[])
{
    atexit(attemptCleanup);
    if (argc != 2) {
        printf("Use %s <file_path>\n", argv[0]);
        exit(ERR_NO_FILENAME_GIVEN);
    }
    const char* inputFileName = argv[1];

    ErrorCode err;
    EXIT_ON_ERR(parser_new(&parser, inputFileName));
    EXIT_ON_ERR(codeWriter_new(&codeWriter, inputFileName));

    while (parser_hasMoreCommands(&parser)) {
        EXIT_ON_ERR(parser_advance(&parser));
        EXIT_ON_ERR(codeWriter_translateCmd(&codeWriter, &parser.currCmd));
    }

    return 0;
}

static void attemptCleanup(void)
{
    parser_close(&parser);
    codeWriter_close(&codeWriter);
}

