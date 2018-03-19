#include "SmtParser.h"
#include "smtlib2/smtlib2abstractparser.h"
#include "smtlib2yices.h"

#include "CommonCrossPlatform/Common.h"

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

	return tmpFile;
}

void ParseAst(const char *ast) {
    smtlib2_yices_parser *parser = smtlib2_yices_parser_new();

    FILE *f = DumpStringToFile(ast);
    smtlib2_abstract_parser_parse((smtlib2_abstract_parser *)parser, f);

    // do stuff

    fclose(f);
    smtlib2_yices_parser_delete(parser);
}
