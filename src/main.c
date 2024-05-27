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
char* outputFileName;
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
    EXIT_ON_ERR(changeFileExtension(&outputFileName, inputFileName));

    ErrorCode err;
    Parser parser;
    CodeWriter codeWriter;
    EXIT_ON_ERR(parser_new(&parser, inputFileName));
    EXIT_ON_ERR(codeWriter_new(&codeWriter, outputFileName));

    while (parser_hasMoreCommands(&parser)) {
        EXIT_ON_ERR(parser_advance(&parser));
        EXIT_ON_ERR(codeWriter_translateCmd(&codeWriter, &parser.currCmd));
    }
    
    EXIT_ON_ERR(parser_close(&parser));
    EXIT_ON_ERR(codeWriter_close(&codeWriter));
    return 0;
}

static ErrorCode changeFileExtension(char** dst, const char* name)
{
    *dst = malloc(strlen(name) + 1);
    if (!(*dst)) {
        logError(ERR_PROG_OUT_OF_MEMORY, NULL);
        return ERR_PROG_OUT_OF_MEMORY;
    }

    strncpy(*dst, name, strlen(name) - strlen(".vm"));
    strcpy(*dst + strlen(*dst), ".asm");
    return OK;
}

static void attemptCleanup(void)
{
    free(outputFileName);
}

