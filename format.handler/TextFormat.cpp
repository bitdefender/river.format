#include "TextFormat.h"
#include "CommonCrossPlatform/Common.h" //MAX_PATH

bool TextFormat::WriteTestName(const char *testName) {
	char line[MAX_PATH + 10];
	int sz = sprintf(line, "## %s\n", testName);

	log->WriteBytes((unsigned char *)line, sz);
	sz = sprintf(line, "%-15s + %-12s %-6s %-6s %-6s %-6s %-15s - %-8s %-15s - %-8s\n",
			"moduleName", "offset", "cost", "jmp", "instr", "ninstr", "taken", "offset",
			"nottaken", "offset");
	log->WriteBytes((unsigned char *)line, sz);
	return true;
}

bool TextFormat::WriteBasicBlock(struct BasicBlockMeta bbm) {
	char line[MAX_PATH * (bbm.bbpNextSize + 1) + 50];
	int sz = sprintf(line, "%-15s + %08X (%4d) (%4d) (%4d) (%4d)",
		bbm.bbp.modName,
		bbm.bbp.offset,
		bbm.cost,
		bbm.jumpType,
		bbm.jumpInstruction,
		bbm.nInstructions
	);

	for (unsigned int i = 0; i < bbm.bbpNextSize; ++i) {
		sz += sprintf(line + sz, " %-15s + %08X", bbm.bbpNext[i].modName,
				bbm.bbpNext[i].offset);
	}
	sz += sprintf(line + sz, "\n");

	log->WriteBytes((unsigned char *)line, sz);
	return true;
}

bool TextFormat::WriteInputUsage(unsigned int offset) {
	char line[30];
	int sz = sprintf(line, "Input offsets used: s[%d]\n", offset);
	log->WriteBytes((unsigned char *)line, sz);
	return true;
}

bool TextFormat::WriteTaintedIndexPayload(unsigned int dest,
		unsigned int source) {
	char line[20];
	int sz = sprintf(line, "I[%u] <= p[%u]\n", dest, source);
	log->WriteBytes((unsigned char *)line, sz);
	return true;
}

bool TextFormat::WriteTaintedIndexExtract(unsigned int dest,
		unsigned int source, unsigned int lsb, unsigned int size) {
	char line[50];
	int sz = sprintf(line, "I[%u] <= I[%u][%u:%u]\n",
			dest, source, lsb, size);
	log->WriteBytes((unsigned char *)line, sz);
	return true;
}

bool TextFormat::WriteTaintedIndexConcat(unsigned int dest,
		unsigned int operands[2]) {
	char line [50];
	int sz = sprintf(line, "I[%u] <= I[%u] ++ I[%u]\n",
			dest, operands[0], operands[1]);
	log->WriteBytes((unsigned char *)line, sz);
	return true;
}

static const char flagNames[6][3] = {
	"CF", "PF", "AF", "ZF", "SF", "OF"
};

static const int flagCount = sizeof(flagNames) / sizeof(flagNames[0]);

bool TextFormat::WriteTaintedIndexExecute(unsigned int dest,
		unsigned int flags, unsigned int depsSize, unsigned int *deps) {
	char line[100];
	int sz = sprintf(line, "I[%u] <=", dest);
	unsigned int depFlags = 0;

	// first check flags
	for (int i = 0; i < flagCount; ++i) {
		if (flags & (1 << i)) {
			sz += sprintf(line + sz, " %s:I[%u]", flagNames[i], deps[depFlags]);
			if (depFlags < depsSize - 1) {
				sz += sprintf(line + sz, " |");
			}
			depFlags += 1;
		}
	}
	for (int i = depFlags; i < depsSize; ++i) {
		sz += sprintf(line + sz, " I[%u]", deps[i]);
		if (i < depsSize - 1) {
			sz += sprintf(line + sz, " |");
		}
	}
	sz += sprintf(line + sz, "\n");
	log->WriteBytes((unsigned char *)line, sz);
	return true;
}

/*void TextLog::FlushLog() {
	fflush(fLog);
}*/
