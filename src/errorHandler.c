#include <stdio.h>
#include "errorHandler.h"
#include "parser.h"

#define RESET  "\x1B[0m"
#define RED    "\x1B[31m"

void logError(ErrorCode err, const char* msg)
{
#ifndef THIS_IS_TEST
    switch (err) {
        case ERR_CANT_OPEN_INPUT_FILE:
        {
            printf("%sERROR. Could not open input file %s%s\n",
                    RED,
                    msg,
                    RESET);
            break;
        }
        case ERR_FILENAME_NOT_VM:
        {
            printf("%sERROR. Please provide a file with .vm extension%s\n",
                    RED,
                    RESET);
            break;
        }
        case ERR_UNEXPEC_TOKEN:
        {
            printf("%sERROR. Unexpected token '%s'%s\n",
                    RED,
                    msg,
                    RESET);
            break;
        }
        default:
            break;
    }
#endif // ifndef THIS_IS_TEST
}

void parser_logError(const Parser *p, ErrorCode err)
{
#ifndef THIS_IS_TEST
    switch (err) {
        case ERR_MAX_IDENTIFIER_LEN:
        {
            printf("%sERROR. Maximum length for an identifier Line %llu%s\n",
                    RED,
                    p->lineNumber,
                    RESET);
        }
        case ERR_UNEXPEC_TOKEN:
        {
            printf("%sERROR. Unexpected token '%c' on line %llu%s\n",
                    RED,
                    p->content[p->cursor],
                    p->lineNumber,
                    RESET);
            break;
        }
        default:
            break;
    }
#endif // ifndef THIS_IS_TEST
}
