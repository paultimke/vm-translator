#ifndef ERR_HANDLER_H
#define ERR_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OK,
    ERR_UNKNOWN,
    ERR_UNEXPEC_TOKEN,
    ERR_CANT_OPEN_INPUT_FILE,
    ERR_CANT_OPEN_OUTFILE,
    ERR_NO_FILENAME_GIVEN,
    ERR_FILENAME_NOT_VM,
    ERR_MAX_IDENTIFIER_LEN,
    ERR_PROG_OUT_OF_MEMORY
} ErrorCode;

typedef struct Parser Parser;
/// @brief: Logs errors related to the parsing process to STDOUT
void parser_logError(const Parser* parserObject, ErrorCode err);

/// @brief: Logs errors to STDOUT
void logError(ErrorCode err, const char* msg);

#ifdef __cplusplus
}
#endif

#endif //ERR_HANDLER_H
