#include "SmtParser.h"
#include "z3.h"

#include "CommonCrossPlatform/Common.h"

#define MAX_FILENAME 200

unsigned count = 0;

FILE *DumpStringToFile(const char *ast) {
    FILE *tmpFile;
    size_t written = 0, astSize;

    astSize = strlen(ast);
    tmpFile = tmpfile();

    while (written < astSize) {
        size_t ret = fwrite(ast + written, 1, astSize - written, tmpFile);
        if (!ret)
            DEBUG_BREAK;
        written += ret;
    }

	fseek(tmpFile, 0, SEEK_SET);

	return tmpFile;
}

void ParseAst(const char *ast) {

    FILE *f = DumpStringToFile(ast);

    // do stuff

    fclose(f);
}
