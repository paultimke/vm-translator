#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "codeWriter.h"
#include "errorHandler.h"
#include "keywords.h"
#include "parser.h"

// Local file-scoped functions and variables
ErrorCode codeWriter_writeArithmeticCmd(CodeWriter* cw, const Command* cmd);

ErrorCode codeWriter_new(CodeWriter *cw, const char* fileName)
{
    cw->outputFile = fopen(fileName, "w");
    if (!cw->outputFile) {
        logError(ERR_CANT_OPEN_OUTFILE, NULL);
        return ERR_CANT_OPEN_OUTFILE;
    }
    return OK;
}

ErrorCode codeWriter_close(CodeWriter *cw)
{
    if (cw->outputFile) {
        fclose(cw->outputFile);
        return OK;
    }
    return ERR_UNKNOWN;
}

ErrorCode codeWriter_translateCmd(CodeWriter *cw, const Command *cmd)
{
    ErrorCode err = ERR_UNKNOWN;
    switch (cmd->type) {
        case CMD_ARITHMETIC:
        {
            err = codeWriter_writeArithmeticCmd(cw, cmd);
            if (err != OK) return err;
            break;
        }
        case CMD_PUSH:
        {
            break;
        }
        case CMD_POP:
        {
            break;
        }
        case CMD_LABEL:
        {
            break;
        }
        case CMD_GOTO:
        {
            break;
        }
        case CMD_IF:
        {
            break;
        }
        case CMD_FUNCTION:
        {
            break;
        }
        case CMD_CALL:
        {
            break;
        }
        case CMD_RETURN:
        {
            break;
        }
        default:
            break;
    }
    return OK;
}

ErrorCode codeWriter_writeArithmeticCmd(CodeWriter* cw, const Command* cmd)
{
    if (strcmp(cmd->Arg1, "add") == 0) {
        char* str = "// add\n@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nM=M+D\n@SP\nM=M+1";
        fprintf(cw->outputFile, "%s\n", str);
    }
    
    return OK;
}
