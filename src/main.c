#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "codeWriter.h"
#include "errorHandler.h"
#include "parser.h"
#include "main.h"

#define EXIT_ON_ERR(err)    ({ErrorCode e = err; if (e != OK) exit(e);})
#define RETURN_ON_ERR(err)    ({ErrorCode e = err; if (e != OK) return (e);})

///////////////////////////////////////////////////////////
// Local file-scoped functions and variables
///////////////////////////////////////////////////////////
static Parser parser;
static CodeWriter codeWriter;

static FileType getFileType(const char* path);
static ErrorCode processDirectory(const char* dirName);
static ErrorCode processFile(const char* fileName);
static void attemptCleanup(void);

///////////////////////////////////////////////////////////
// Entry point
///////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    atexit(attemptCleanup);
    if (argc != 2) {
        printf("Use %s <file_path>\n", argv[0]);
        exit(ERR_NO_FILENAME_GIVEN);
    }
    const char* path = argv[1];

    switch (getFileType(path)) {
        case FILE_REGULAR:
            EXIT_ON_ERR(codeWriter_new(&codeWriter, path, FILE_REGULAR));
            EXIT_ON_ERR(processFile(path));
            break;
        case FILE_DIR:
            EXIT_ON_ERR(codeWriter_new(&codeWriter, path, FILE_DIR));
            EXIT_ON_ERR(processDirectory(path));
            break;
        default:
            logError(ERR_CANT_OPEN_INPUT_FILE, "Unknown file type");
            exit(ERR_CANT_OPEN_INPUT_FILE);
            break;
    }

    return 0;
}

///////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////

static FileType getFileType(const char* path)
{
    struct stat s;
    if ( stat(path, &s) == 0 ) {
        if (s.st_mode & S_IFDIR) {
            return FILE_DIR;
        }
        else if (s.st_mode & S_IFREG) {
            return FILE_REGULAR;
        }
    }
    return FILE_ERR;
}

static ErrorCode processDirectory(const char* dirName)
{
    struct dirent *dp = NULL;
    DIR *dfd = NULL;

    dfd = opendir(dirName);
    if (dfd == NULL) {
        logError(ERR_CANT_OPEN_DIR, dirName);
    }

    char fileName[100];

    while ( (dp = readdir(dfd)) != NULL) {
        struct stat stbuf ;
        sprintf( fileName , "%s/%s", dirName, dp->d_name) ;

        if( stat(fileName, &stbuf ) == -1 )
        {
            printf("Unable to stat file: %s\n", fileName) ;
            continue ;
        }

        if ( ( stbuf.st_mode & S_IFMT ) == S_IFDIR )
        {
            continue;
            // Skip directories
        }
        else
        {
            // Files found
            printf("Processing %s\n", fileName);
            RETURN_ON_ERR(processFile(fileName));
        }
    }

    return OK;
}

static ErrorCode processFile(const char* fileName)
{
    RETURN_ON_ERR(parser_new(&parser, fileName));

    while (parser_hasMoreCommands(&parser)) {
        RETURN_ON_ERR(parser_advance(&parser));
        RETURN_ON_ERR(codeWriter_translateCmd(&codeWriter, &parser.currCmd));
    }
    parser_close(&parser);
    return OK;
}

static void attemptCleanup(void)
{
    parser_close(&parser);
    codeWriter_close(&codeWriter);
}
