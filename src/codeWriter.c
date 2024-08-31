#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "errorHandler.h"
#include "keywords.h"
#include "parser.h"
#include "codeWriter.h"

///////////////////////////////////////////////////////////
// Macros and defines
///////////////////////////////////////////////////////////
#define NUM_ULONG_MAX_RANGE_CHARS               (11) // 4'294'967'295 has 11 chars
#define GET_RETURN_ADDR_LABEL_SIZE(funcName)    \
    (sizeof(funcName) + sizeof("_retAddr") + sizeof("_") + NUM_ULONG_MAX_RANGE_CHARS)

// The following are very common operations throughout the program, so macros were defined
#define GENERATE_PUSH_CONSTANT_CODE(file, constant)    \
    fprintf(file, "    @%s\n    D=A\n    @SP\n    A=M\n    M=D\n    @SP\n    M=M+1\n", (constant))

#define GENERATE_LABEL_DECLARATION_CODE(file, labelName)    fprintf(file, "(%s)\n", (labelName))

#define GENERATE_GOTO_CODE(file, labelName)                 fprintf(file, "    @%s\n    0; JMP\n", (labelName))

///////////////////////////////////////////////////////////
// Local file-scoped functions and variables
///////////////////////////////////////////////////////////
static ErrorCode codeWriter_writeArithmetic(CodeWriter* cw, const Command* cmd);
static ErrorCode codeWriter_writePush(CodeWriter* cw, const Command* cmd);
static ErrorCode codeWriter_writePop(CodeWriter* cw, const Command* cmd);
static ErrorCode codeWriter_writeLabel(CodeWriter* cw, const Command* cmd);
static ErrorCode codeWriter_writeGoto(CodeWriter* cw, const Command* cmd);
static ErrorCode codeWriter_writeIfGoto(CodeWriter* cw, const Command* cmd);
static ErrorCode codeWriter_writeFunction(CodeWriter* cw, const Command* cmd);
static ErrorCode codeWriter_writeFunctionCall(CodeWriter* cw, const Command* cmd);
static ErrorCode codeWriter_writeReturn(CodeWriter* cw, const Command* cmd);
static char* codeWriter_getBaseFileName(CodeWriter* cw);

// The generated label for the return address of a function will be:
// <functionName>_retAddr_<returnAddressCounter>
// The counter was added at the end so that it serves as a unique identifier
// when the same function is called twice. Without it, the same label would be
// generated at different addresses, and the last one would overwrite all of the
// previous ones
static unsigned long returnAddressCounter = 0;

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////
ErrorCode codeWriter_new(CodeWriter *cw, const char* fileOrDirName, FileType fileType)
{
    char* fileExtension = "";

    // Initialize the current processed file name to NULL
    // This should be later set with codeWriter_setCurrentFileName()
    cw->currentVMfile = NULL;
    cw->currentVMfile = 0;

    //  Only generate code for .vm files
    if (FILE_REGULAR == fileType) {
        if (strcmp(&fileOrDirName[strlen(fileOrDirName) - 3], ".vm") != 0) {
            logError(ERR_FILENAME_NOT_VM, NULL);
            return (ERR_FILENAME_NOT_VM);
        }

        fileExtension = "vm";
    }

    // Allocate memory for the file name
    cw->outFileNameLen = strlen(fileOrDirName) - strlen(fileExtension) + strlen("asm");
    cw->outFileName = malloc(cw->outFileNameLen);
    if (!cw->outFileName) {
        logError(ERR_PROG_OUT_OF_MEMORY, NULL);
        return ERR_PROG_OUT_OF_MEMORY;
    }

    // Replace the filename extension
    if (FILE_REGULAR == fileType) {
        strncpy(cw->outFileName, fileOrDirName, strlen(fileOrDirName) - strlen(".vm"));
    }
    else if (FILE_DIR == fileType) {
        strncpy(cw->outFileName, fileOrDirName, strlen(fileOrDirName));
    }
    strcpy(&cw->outFileName[strlen(cw->outFileName)], ".asm");

    cw->outputFile = fopen(cw->outFileName, "w");
    if (!cw->outputFile) {
        logError(ERR_CANT_OPEN_OUTFILE, NULL);
        return ERR_CANT_OPEN_OUTFILE;
    }
    return OK;
}

void codeWriter_close(CodeWriter *cw)
{
    assert(cw->outFileName != NULL);
    free(cw->outFileName);
    cw->outFileName = NULL;
    if (cw->outputFile) {
        fclose(cw->outputFile);
        cw->outputFile = NULL;
    }

    if (cw->currentVMfile == NULL) {
        free(cw->currentVMfile);
    }
}

ErrorCode codeWriter_setCurrentFileName(CodeWriter* cw, const char* fileName)
{
    if (cw->currentVMfile != NULL) {
        free(cw->currentVMfile);
    }
    cw->currentVMfileLen = strlen(fileName);
    cw->currentVMfile = strdup(fileName);
    if (cw->currentVMfile == NULL) {
        return ERR_PROG_OUT_OF_MEMORY;
    }
    return OK;
}

ErrorCode codeWriter_writeStartupCode(CodeWriter *cw)
{
    fprintf(cw->outputFile, "// **** Bootstrap code ****\n");
    fprintf(cw->outputFile, "// Set Stack pointer to start at address 256\n");
    fprintf(cw->outputFile, "    @256\n    D=A\n    @SP\n    M=D\n");
    Command cmd = {
        .type = CMD_CALL,
        .Arg1 = "Sys.init",
        .Arg2 = "0"
    };
    codeWriter_writeFunctionCall(cw, &cmd);
    return OK;
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
            err = codeWriter_writeLabel(cw, cmd);
            if (err != OK) return err;
            break;
        }
        case CMD_GOTO:
        {
            err = codeWriter_writeGoto(cw, cmd);
            if (err != OK) return err;
            break;
        }
        case CMD_IF:
        {
            err = codeWriter_writeIfGoto(cw, cmd);
            if (err != OK) return err;
            break;
        }
        case CMD_FUNCTION:
        {
            err = codeWriter_writeFunction(cw, cmd);
            if (err != OK) return err;
            break;
        }
        case CMD_CALL:
        {
            err = codeWriter_writeFunctionCall(cw, cmd);
            if (err != OK) return err;
            break;
        }
        case CMD_RETURN:
        {
            err = codeWriter_writeReturn(cw, cmd);
            if (err != OK) return err;
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
        str = "// add\n    @SP\n    AM=M-1\n    D=M\n    @SP\n    M=M-1\n    A=M\n    M=M+D\n    @SP\n    M=M+1";
        fprintf(cw->outputFile, "%s\n", str);
    }
    else if (strcmp(cmd->Arg1, "sub") == 0) {
        str = "// sub\n    @SP\n    AM=M-1\n    D=M\n    @SP\n    M=M-1\n    A=M\n    M=M-D\n    @SP\n    M=M+1";
        fprintf(cw->outputFile, "%s\n", str);
    }
    else if (strcmp(cmd->Arg1, "neg") == 0) {
        str = "// neg\n    @SP\n    A=M-1\n    M=-M";
        fprintf(cw->outputFile, "%s\n", str);
    }
    else if (strcmp(cmd->Arg1, "eq") == 0) {
        str = "// eq\n    @SP\n    AM=M-1\n    D=M\n    A=A-1\n    D=D-M\n    M=D";
        fprintf(cw->outputFile, "%s\n", str);
    }
    else if (strcmp(cmd->Arg1, "gt") == 0) {
        static size_t gt_jump_count = 0;
        fprintf(cw->outputFile, "// gt\n    @SP\n    AM=M-1\n    D=M\n    A=A-1\n    D=D-M\n");
        fprintf(cw->outputFile, "    @__GT_%lu\n    D; JLE\n", gt_jump_count);
        fprintf(cw->outputFile, "    @SP\n    A=M-1\n    M=0\n    @__GT_END_%lu\n", gt_jump_count);
        fprintf(cw->outputFile, "    0; JMP\n(__GT_%lu)\n    @SP\n A=M-1\n", gt_jump_count);
        fprintf(cw->outputFile, "    M=1\n(__GT_END_%lu)\n", gt_jump_count);
        gt_jump_count++;
    }
    else if (strcmp(cmd->Arg1, "lt") == 0) {
        static size_t lt_jump_count = 0;
        fprintf(cw->outputFile, "// lt\n    @SP\n    AM=M-1\n    D=M\n    A=A-1\n    D=D-M\n");
        fprintf(cw->outputFile, "    @__LT_%lu\n    D; JGT\n", lt_jump_count);
        fprintf(cw->outputFile, "    @SP\n    A=M-1\n    M=0\n    @__LT_END_%lu\n", lt_jump_count);
        fprintf(cw->outputFile, "    0; JMP\n(__LT_%lu)\n    @SP\n A=M-1\n", lt_jump_count);
        fprintf(cw->outputFile, "    M=1\n(__LT_END_%lu)\n", lt_jump_count);
        lt_jump_count++;
    }
    else if (strcmp(cmd->Arg1, "and") == 0) {
        str = "//   and\n    @SP\n    AM=M-1\n    D=M\n    A=A-1\n    D=D&M\n    M=D";
        fprintf(cw->outputFile, "%s\n", str);
    }
    else if (strcmp(cmd->Arg1, "or") == 0) {
        str = "//   or\n    @SP\n    AM=M-1\n    D=M\n    A=A-1\n    D=D|M\n    M=D";
        fprintf(cw->outputFile, "%s\n", str);
    }
    else if (strcmp(cmd->Arg1, "not") == 0) {
        str = "//   not\n    @SP\n    A=M-1\n    M=!M";
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
        GENERATE_PUSH_CONSTANT_CODE(cw->outputFile, cmd->Arg2);
        return OK;
    }
    else if (strcmp(cmd->Arg1, "pointer") == 0) {
        ErrorCode err = OK;
        if (strcmp(cmd->Arg2, "0") == 0) {
            fprintf(cw->outputFile, "    @THIS\n    D=M\n    @SP\n    A=M\n    M=D\n    @SP\n    M=M+1\n");
        }
        else if (strcmp(cmd->Arg2, "1") == 0) {
            fprintf(cw->outputFile, "    @THAT\n    D=M\n    @SP\n    A=M\n    M=D\n    @SP\n    M=M+1\n");
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
        fprintf(cw->outputFile, "    @LCL\n    D=M\n");
    }
    else if (strcmp(cmd->Arg1, "argument") == 0) {
        fprintf(cw->outputFile, "    @ARG\n    D=M\n");
    }
    else if (strcmp(cmd->Arg1, "this") == 0) {
        fprintf(cw->outputFile, "    @THIS\n    D=M\n");
    }
    else if (strcmp(cmd->Arg1, "that") == 0) {
        fprintf(cw->outputFile, "    @THAT\n    D=M\n");
    }
    else if (strcmp(cmd->Arg1, "temp") == 0) {
        fprintf(cw->outputFile, "    @5\n    D=A\n");
    }
    else if (strcmp(cmd->Arg1, "static") == 0) {
        char* baseFileName = codeWriter_getBaseFileName(cw);
        fprintf(cw->outputFile, "    @%s.%s\n    D=M\n", baseFileName, cmd->Arg2);
        fprintf(cw->outputFile, "    @SP\n    A=M\n    M=D\n    @SP\n    M=M+1\n");
        free(baseFileName);
        return OK;
    }

    // The rest of the code is the same
    fprintf(cw->outputFile, "    @%s\n", cmd->Arg2);
    fprintf(cw->outputFile, "    A=D+A\n    D=M\n    @SP\n    A=M\n    M=D\n    @SP\n    M=M+1\n");

    return OK;
}

static ErrorCode codeWriter_writePop(CodeWriter* cw, const Command* cmd)
{
    fprintf(cw->outputFile, "// pop %s %s\n", cmd->Arg1, cmd->Arg2);

    // Implementation for constant and pointer is different, so we return
    // early on either of them
    if (strcmp(cmd->Arg1, "constant") == 0) {
        fprintf(cw->outputFile, "    @SP\n    M=M-1\n    D=M\n    @%s\n    M=D\n", cmd->Arg2);
        return OK;
    }
    else if (strcmp(cmd->Arg1, "pointer") == 0) {
        ErrorCode err = OK;
        if (strcmp(cmd->Arg2, "0") == 0) {
            fprintf(cw->outputFile, "    @SP\n    AM=M-1\n    D=M\n    @THIS\n    M=D\n");
        }
        else if (strcmp(cmd->Arg2, "1") == 0) {
            fprintf(cw->outputFile, "    @SP\n    AM=M-1\n    D=M\n    @THAT\n    M=D\n");
        }
        else {
            logError(ERR_PUSHPOP_PTR_NOT_0_OR_1, cmd->Arg1);
            err = ERR_PUSHPOP_PTR_NOT_0_OR_1;
        }
        return err;
    }

    // For all other segments:
    // First four lines are the same
    fprintf(cw->outputFile, "    @SP\n    M=M-1\n    A=M\n    D=M\n");

    // Fifth line depends on the segment
    if (strcmp(cmd->Arg1, "local") == 0) {
        fprintf(cw->outputFile, "    @LCL\n    D=D+M\n");
    }
    else if (strcmp(cmd->Arg1, "argument") == 0) {
        fprintf(cw->outputFile, "    @ARG\n    D=D+M\n");
    }
    else if (strcmp(cmd->Arg1, "this") == 0) {
        fprintf(cw->outputFile, "    @THIS\n    D=D+M\n");
    }
    else if (strcmp(cmd->Arg1, "that") == 0) {
        fprintf(cw->outputFile, "    @THAT\n    D=D+M\n");
    }
    else if (strcmp(cmd->Arg1, "temp") == 0) {
        fprintf(cw->outputFile, "    @5\n    D=D+A\n");
    }
    else if (strcmp(cmd->Arg1, "static") == 0) {
        char* baseFileName = codeWriter_getBaseFileName(cw);
        fprintf(cw->outputFile, "    @%s.%s\n    M=D\n", baseFileName, cmd->Arg2);
        free(baseFileName);
        return OK;
    }
    else {
        logError(ERR_UNKNOWN_SEGMENT, cmd->Arg1);
        return ERR_UNKNOWN_SEGMENT;
    }

    // The rest of the code is the same
    fprintf(cw->outputFile, "    @%s\n", cmd->Arg2);
    fprintf(cw->outputFile, "    D=D+A\n    @SP\n    A=M\n    A=M\n    A=D-A\n    M=D-A\n");

    return OK;
}

static ErrorCode codeWriter_writeLabel(CodeWriter* cw, const Command* cmd)
{
    fprintf(cw->outputFile, "// label %s\n", cmd->Arg1);
    GENERATE_LABEL_DECLARATION_CODE(cw->outputFile, cmd->Arg1);
    return OK;
}

static ErrorCode codeWriter_writeGoto(CodeWriter* cw, const Command* cmd)
{
    fprintf(cw->outputFile, "// goto %s\n", cmd->Arg1);
    GENERATE_GOTO_CODE(cw->outputFile, cmd->Arg1);
    return OK;
}

static ErrorCode codeWriter_writeIfGoto(CodeWriter* cw, const Command* cmd)
{
    // For some reason, the If-Goto command has the side effect of decrementing
    // the stack pointer. So it doesn't just checks the top of the stack to
    // know if it should jump or not but it actually pops the value
    fprintf(cw->outputFile, "// if-goto %s\n", cmd->Arg1);
    fprintf(cw->outputFile, "    @SP\n    AM=M-1\n    D=M\n");
    fprintf(cw->outputFile, "    @%s\n    D; JGT\n", cmd->Arg1);
    return OK;
}

static ErrorCode codeWriter_writeFunction(CodeWriter* cw, const Command* cmd)
{
    fprintf(cw->outputFile, "\n// function %s %s\n", cmd->Arg1, cmd->Arg2);
    GENERATE_LABEL_DECLARATION_CODE(cw->outputFile, cmd->Arg1);

    // Convert nVars argument from string to integer
    int nVars = atoi(cmd->Arg2);
    for (int i = 0; i < nVars; i++) {
        // Push 0 nVars times into the stack
        GENERATE_PUSH_CONSTANT_CODE(cw->outputFile, "0");
    }

    return OK;
}

static ErrorCode codeWriter_writeFunctionCall(CodeWriter* cw, const Command* cmd)
{
    fprintf(cw->outputFile, "\n// call %s %s\n", cmd->Arg1, cmd->Arg2);

    // Return address label will be the function name appended by _retAddr
    char* retAddrLabel = malloc(sizeof(char) * GET_RETURN_ADDR_LABEL_SIZE(cmd->Arg1));

    if (retAddrLabel == NULL) {
        logError(ERR_PROG_OUT_OF_MEMORY, NULL);
        return ERR_PROG_OUT_OF_MEMORY;
    }
    sprintf(retAddrLabel, "%s_retAddr_%lu", cmd->Arg1, returnAddressCounter);
    returnAddressCounter++;

    // Push the return address onto the stack
    GENERATE_PUSH_CONSTANT_CODE(cw->outputFile, retAddrLabel);

    // Push the caller's segment pointers into the stack
    fprintf(cw->outputFile, "    @LCL\n    D=M\n    @SP\n    A=M\n    M=D\n    @SP\n    M=M+1");
    fprintf(cw->outputFile, " // Push LCL\n");
    fprintf(cw->outputFile, "    @ARG\n    D=M\n    @SP\n    A=M\n    M=D\n    @SP\n    M=M+1");
    fprintf(cw->outputFile, " // Push ARG\n");
    fprintf(cw->outputFile, "    @THIS\n    D=M\n    @SP\n    A=M\n    M=D\n    @SP\n    M=M+1");
    fprintf(cw->outputFile, " // Push THIS\n");
    fprintf(cw->outputFile, "    @THAT\n    D=M\n    @SP\n    A=M\n    M=D\n    @SP\n    M=M+1");
    fprintf(cw->outputFile, " // Push THAT\n");

    // Reposition ARG. Subtract 5 because we just pushed 5 things.
    // We also need to subtract nArgs because that is the address where
    // the arguments start
    // *ARG = *SP - 5 - nArgs, but we can do (5+nArgs) locally and write
    // the result, so it is now *ARG = *SP - (5 + nArgs)
    const int subtractValue = 5 + atoi(cmd->Arg2);
    // @SP
    // D=M    // D = *SP = RAM[0]
    // @subtractValue
    // D=D-A  // D = *SP - 5 - nArgs
    // @ARG
    // M=D    // *ARG = D = RAM[ARG] 
    fprintf(cw->outputFile, "    @SP\n    D=M\n    @%d\n", subtractValue);
    fprintf(cw->outputFile, "    D=D-A\n    @ARG\n    M=D\n");

    // Reposition LCL to the top of the stack
    // *LCL = SP
    // which is:
    // @SP
    // D=M
    // @LCL
    // M=D
    fprintf(cw->outputFile, "    @SP\n    D=M\n    @LCL\n    M=D\n");

    // Transfer control to called function (goto <functionName>)
    GENERATE_GOTO_CODE(cw->outputFile, cmd->Arg1);

    // Write the return label declaration to the file
    GENERATE_LABEL_DECLARATION_CODE(cw->outputFile, retAddrLabel);

    free(retAddrLabel);
    retAddrLabel = NULL;
    return OK;
}

static ErrorCode codeWriter_writeReturn(CodeWriter* cw, const Command* cmd)
{
    fprintf(cw->outputFile, "// return\n");

    // Save return address into a temporary variable, as now LCL points to the
    // end of frame, but LCL will soon be overwriten with its old value
    fprintf(cw->outputFile, "    @LCL\n    D=M\n    @5\n    A=D-A\n    D=M\n    @retAddrVar\n    M=D\n");

    // Move return value into the position of Argument 0 in the stack
    // At this point, the return value must be at the top of the stack
    // @SP
    // A=M-1
    // D=M  // pop()
    // @ARG
    // A=M
    // M=D
    fprintf(cw->outputFile, "    @SP\n    A=M-1\n    D=M\n    @ARG\n    A=M\n    M=D\n");

    // Update the stack pointer right after return value 
    // (1 plus the position of Argument 0)
    fprintf(cw->outputFile, "    @ARG\n    D=M+1\n    @SP\n    M=D\n");

    // Return the caller's frame (pointer variables)
    // required operation is e.g. *THIS = *(*LCL - 2)
    // which we can do as
    // @LCL
    // D=M    // D = RAM[1] = *LCL
    // @2
    // A=D-A  // A = RAM[1] - 2
    // D=M    // D = RAM[ RAM[1] - 2]
    // @THIS
    // M=D
    fprintf(cw->outputFile, "    @LCL\n    D=M\n    @1\n    A=D-A\n    D=M\n    @THAT\n    M=D\n");
    fprintf(cw->outputFile, "    @LCL\n    D=M\n    @2\n    A=D-A\n    D=M\n    @THIS\n    M=D\n");
    fprintf(cw->outputFile, "    @LCL\n    D=M\n    @3\n    A=D-A\n    D=M\n    @ARG\n    M=D\n");
    fprintf(cw->outputFile, "    @LCL\n    D=M\n    @4\n    A=D-A\n    D=M\n    @LCL\n    M=D\n");

    // Retrieve return address from the stack and go to it
    fprintf(cw->outputFile, "    @retAddrVar\n    A=M\n");
    fprintf(cw->outputFile, "    0; JMP\n");
    return OK;
}

static ErrorCode removePathBackslashes(char* fileNameToChange)
{
    int i = 0;
    for (i = 0; i < strlen(fileNameToChange); i++) {
        if (fileNameToChange[i] == '/')
            fileNameToChange[i] = '_';
        else if (fileNameToChange[i] == '.')
            fileNameToChange[i] = 'x';
    }
    fileNameToChange[i] = '\0';
    return OK;
}

static char* codeWriter_getBaseFileName(CodeWriter* cw)
{
    char* str = strndup(cw->currentVMfile, cw->currentVMfileLen - strlen(".vm"));
    removePathBackslashes(str);
    return str;
}
