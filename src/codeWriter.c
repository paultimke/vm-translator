#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "codeWriter.h"
#include "errorHandler.h"
#include "keywords.h"
#include "parser.h"

// Local file-scoped functions and variables
static ErrorCode codeWriter_writeArithmetic(CodeWriter* cw, const Command* cmd);
static ErrorCode codeWriter_writePush(CodeWriter* cw, const Command* cmd);
static ErrorCode codeWriter_writePop(CodeWriter* cw, const Command* cmd);
static char* codeWriter_getBaseFileName(CodeWriter* cw);

// -------------------------- PUBLIC FUNCTIONS ----------------------------- //
ErrorCode codeWriter_new(CodeWriter *cw, const char* fileName)
{
    // Assert the file has a .vm extension
    if (strcmp(&fileName[strlen(fileName) - 3], ".vm") != 0) {
        logError(ERR_FILENAME_NOT_VM, NULL);
        exit(ERR_FILENAME_NOT_VM);
    }

    cw->fileNameLen = strlen(fileName) - strlen("vm") + strlen("asm");
    cw->fileName = malloc(cw->fileNameLen);
    if (!cw->fileName) {
        logError(ERR_PROG_OUT_OF_MEMORY, NULL);
        return ERR_PROG_OUT_OF_MEMORY;
    }

    // Replace the filename extension
    strncpy(cw->fileName, fileName, strlen(fileName) - strlen(".vm"));
    strcpy(&cw->fileName[strlen(cw->fileName)], ".asm");

    cw->outputFile = fopen(cw->fileName, "w");
    if (!cw->outputFile) {
        logError(ERR_CANT_OPEN_OUTFILE, NULL);
        return ERR_CANT_OPEN_OUTFILE;
    }
    return OK;
}

void codeWriter_close(CodeWriter *cw)
{
    assert(cw->fileName != NULL);
    free(cw->fileName);
    if (cw->outputFile) {
        fclose(cw->outputFile);
    }
}

ErrorCode codeWriter_translateCmd(CodeWriter *cw, const Command *cmd)
{
    ErrorCode err = ERR_UNKNOWN;
    switch (cmd->type) {
        case CMD_ARITHMETIC:
        {
            err = codeWriter_writeArithmetic(cw, cmd);
            if (err != OK) return err;
            break;
        }
        case CMD_PUSH:
        {
            err = codeWriter_writePush(cw, cmd);
            if (err != OK) return err;
            break;
        }
        case CMD_POP:
        {
            err = codeWriter_writePop(cw, cmd);
            if (err != OK) return err;
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

// --------------------------- PRIVATE FUNCTIONS ---------------------------- //
ErrorCode codeWriter_writeArithmetic(CodeWriter* cw, const Command* cmd)
{
    char* str;
    if (strcmp(cmd->Arg1, "add") == 0) {
        str = "// add\n  @SP\n  AM=M-1\n  D=M\n  @SP\n  M=M-1\n  A=M\n  M=M+D\n  @SP\n  M=M+1\n";
        fprintf(cw->outputFile, "%s\n", str);
    }
    else if (strcmp(cmd->Arg1, "sub") == 0) {
        str = "// sub\n  @SP\n  AM=M-1\n  D=M\n  @SP\n  M=M-1\n  A=M\n  M=M-D\n  @SP\n  M=M+1\n";
        fprintf(cw->outputFile, "%s\n", str);
    }
    else if (strcmp(cmd->Arg1, "neg") == 0) {
        str = "// neg\n  @SP\n  A=M-1\n  M=-M\n";
        fprintf(cw->outputFile, "%s\n", str);
    }
    else if (strcmp(cmd->Arg1, "eq") == 0) {
        str = "// eq\n  @SP\n  AM=M-1\n  D=M\n  A=A-1\n  D=D-M\n  M=D\n";
        fprintf(cw->outputFile, "%s\n", str);
    }
    else if (strcmp(cmd->Arg1, "gt") == 0) {
        static size_t gt_jump_count = 0;
        fprintf(cw->outputFile, "// gt\n  @SP\n  AM=M-1\n  D=M\n  A=A-1\n  D=D-M\n");
        fprintf(cw->outputFile, "  @__GT_%lu\n  D; JLE\n", gt_jump_count);
        fprintf(cw->outputFile, "  @SP\n  A=M-1\n  M=0\n  @__GT_END_%lu\n", gt_jump_count);
        fprintf(cw->outputFile, "  0; JMP\n(__GT_%lu)\n  @SP\n A=M-1\n", gt_jump_count);
        fprintf(cw->outputFile, "  M=1\n(__GT_END_%lu)\n", gt_jump_count);
        gt_jump_count++;
    }
    else if (strcmp(cmd->Arg1, "lt") == 0) {
        static size_t lt_jump_count = 0;
        fprintf(cw->outputFile, "// lt\n  @SP\n  AM=M-1\n  D=M\n  A=A-1\n  D=D-M\n");
        fprintf(cw->outputFile, "  @__LT_%lu\n  D; JGT\n", lt_jump_count);
        fprintf(cw->outputFile, "  @SP\n  A=M-1\n  M=0\n  @__LT_END_%lu\n", lt_jump_count);
        fprintf(cw->outputFile, "  0; JMP\n(__LT_%lu)\n  @SP\n A=M-1\n", lt_jump_count);
        fprintf(cw->outputFile, "  M=1\n(__LT_END_%lu)\n", lt_jump_count);
        lt_jump_count++;
    }
    else if (strcmp(cmd->Arg1, "and") == 0) {
        str = "// and\n  @SP\n  AM=M-1\n  D=M\n  A=A-1\n  D=D&M\n  M=D\n";
        fprintf(cw->outputFile, "%s\n", str);
    }
    else if (strcmp(cmd->Arg1, "or") == 0) {
        str = "// or\n  @SP\n  AM=M-1\n  D=M\n  A=A-1\n  D=D|M\n  M=D\n";
        fprintf(cw->outputFile, "%s\n", str);
    }
    else if (strcmp(cmd->Arg1, "not") == 0) {
        str = "// not\n  @SP\n  A=M-1\n  M=!M\n";
        fprintf(cw->outputFile, "%s\n", str);
    }
    
    return OK;
}

ErrorCode codeWriter_writePush(CodeWriter* cw, const Command* cmd)
{
    fprintf(cw->outputFile, "// push %s %s\n", cmd->Arg1, cmd->Arg2);

    // Implementation for constant and pointer is different, so we return
    // early on either of them
    if (strcmp(cmd->Arg1, "constant") == 0) {
        fprintf(cw->outputFile, "@%s\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", cmd->Arg2);
        return OK;
    }
    else if (strcmp(cmd->Arg1, "pointer") == 0) {
        ErrorCode err = OK;
        if (strcmp(cmd->Arg2, "0") == 0) {
            fprintf(cw->outputFile, "@SP\nA=M\nM=THIS\n@SP\nM=M+1\n");
        }
        else if (strcmp(cmd->Arg2, "1") == 0) {
            fprintf(cw->outputFile, "@SP\nA=M\nM=THAT\n@SP\nM=M+1\n");
        }
        else {
            logError(ERR_PUSHPOP_PTR_NOT_0_OR_1, NULL);
            err = ERR_PUSHPOP_PTR_NOT_0_OR_1;
        }
        return err;
    }

    // For all other segments:
    // The first two lines of the assembly depend on the segment being used
    // First line is base address (like LCL or 5), and second line is
    // D=M for segments with indirect addressing (like local and this) and
    // D=A for segments with direct addressing like static and temp
    else if (strcmp(cmd->Arg1, "local") == 0) {
        fprintf(cw->outputFile, "@LCL\nD=M\n");
    }
    else if (strcmp(cmd->Arg1, "argument") == 0) {
        fprintf(cw->outputFile, "@ARG\nD=M\n");
    }
    else if (strcmp(cmd->Arg1, "this") == 0) {
        fprintf(cw->outputFile, "@THIS\nD=M\n");
    }
    else if (strcmp(cmd->Arg1, "that") == 0) {
        fprintf(cw->outputFile, "@THAT\nD=M\n");
    }
    else if (strcmp(cmd->Arg1, "temp") == 0) {
        fprintf(cw->outputFile, "@5\nD=A\n");
    }
    else if (strcmp(cmd->Arg1, "static") == 0) {
        char* baseFileName = codeWriter_getBaseFileName(cw);
        fprintf(cw->outputFile, "@%s.%s\nD=A\n", baseFileName, cmd->Arg2);
        free(baseFileName);
    }

    // The rest of the code is the same
    fprintf(cw->outputFile, "@%s\n", cmd->Arg2);
    fprintf(cw->outputFile, "A=D+A\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");

    return OK;
}

static ErrorCode codeWriter_writePop(CodeWriter* cw, const Command* cmd)
{
    fprintf(cw->outputFile, "// pop %s %s\n", cmd->Arg1, cmd->Arg2);

    // Implementation for constant and pointer is different, so we return
    // early on either of them
    if (strcmp(cmd->Arg1, "constant") == 0) {
        fprintf(cw->outputFile, "@SP\nM=M-1\nD=M\n@%s\nM=D\n", cmd->Arg2);
        return OK;
    }
    else if (strcmp(cmd->Arg1, "pointer") == 0) {
        ErrorCode err = OK;
        if (strcmp(cmd->Arg2, "0") == 0) {
            fprintf(cw->outputFile, "@SP\nM=M-1\nD=M\n@THIS\nM=D\n");
        }
        else if (strcmp(cmd->Arg2, "1") == 0) {
            fprintf(cw->outputFile, "@SP\nM=M-1\nD=M\n@THAT\nM=D\n");
        }
        else {
            logError(ERR_PUSHPOP_PTR_NOT_0_OR_1, cmd->Arg1);
            err = ERR_PUSHPOP_PTR_NOT_0_OR_1;
        }
        return err;
    }

    // For all other segments:
    // First four lines are the same
    fprintf(cw->outputFile, "@SP\nM=M-1\nA=M\nD=M\n");

    // Fifth line depends on the segment
    if (strcmp(cmd->Arg1, "local") == 0) {
        fprintf(cw->outputFile, "@LCL\nD=D+M\n");
    }
    else if (strcmp(cmd->Arg1, "argument") == 0) {
        fprintf(cw->outputFile, "@ARG\nD=D+M\n");
    }
    else if (strcmp(cmd->Arg1, "this") == 0) {
        fprintf(cw->outputFile, "@THIS\nD=D+M\n");
    }
    else if (strcmp(cmd->Arg1, "that") == 0) {
        fprintf(cw->outputFile, "@THAT\nD=D+M\n");
    }
    else if (strcmp(cmd->Arg1, "temp") == 0) {
        fprintf(cw->outputFile, "@5\nD=D+A\n");
    }
    else if (strcmp(cmd->Arg1, "static") == 0) {
        char* baseFileName = codeWriter_getBaseFileName(cw);
        fprintf(cw->outputFile, "@%s.%s\nD=D+A\n", baseFileName, cmd->Arg2);
        free(baseFileName);
    }
    else {
        logError(ERR_UNKNOWN_SEGMENT, cmd->Arg1);
        return ERR_UNKNOWN_SEGMENT;
    }

    // The rest of the code is the same
    fprintf(cw->outputFile, "@%s\n", cmd->Arg2);
    fprintf(cw->outputFile, "D=D+A\n@SP\nA=M\nA=M\nA=D-A\nM=D-A\n");

    return OK;
}

static char* codeWriter_getBaseFileName(CodeWriter* cw)
{
    return strndup(cw->fileName, cw->fileNameLen - strlen(".asm"));
}
