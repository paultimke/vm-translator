#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "errorHandler.h"
#include "parser.h"
#include <stdio.h>

typedef struct CodeWriter {
    size_t fileNameLen;    // strlen(fileName)
    char* fileName;        // File name without extension
    FILE* outputFile;      // File handle to write
} CodeWriter;


ErrorCode codeWriter_new(CodeWriter *cw, const char* fileName);
void codeWriter_close(CodeWriter *cw);
ErrorCode codeWriter_translateCmd(CodeWriter* cw, const Command* cmd);

#ifdef __cplusplus
}
#endif

#endif // CODE_WRITER_H
