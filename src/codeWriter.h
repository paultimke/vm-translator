#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "errorHandler.h"
#include "parser.h"
#include <stdio.h>

typedef struct CodeWriter {
    size_t outFileNameLen;   // strlen(outFileName)
    char* outFileName;       // File name without extension
    char* currentVMfile;     // Currently processed VM file, when input is not
                             // a directory, this is the same as outFileName
    size_t currentVMfileLen; // strlen(currentVMfile) 
    FILE* outputFile;        // File handle to write
} CodeWriter;


ErrorCode codeWriter_new(CodeWriter *cw, const char* fileName, FileType fileType);
void codeWriter_close(CodeWriter *cw);
ErrorCode codeWriter_writeStartupCode(CodeWriter *cw);
ErrorCode codeWriter_translateCmd(CodeWriter* cw, const Command* cmd);
ErrorCode codeWriter_setCurrentFileName(CodeWriter* cw, const char* fileName);

#ifdef __cplusplus
}
#endif

#endif // CODE_WRITER_H
