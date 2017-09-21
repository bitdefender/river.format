#include "TextFormat.h"
#include <limits.h>

bool TextFormat::WriteTestName(const char *testName) {
	char line[PATH_MAX + 10];
	int sz = sprintf(line, "## %s\n", testName);

	log->WriteBytes((unsigned char *)line, sz);
	return true;
}

bool TextFormat::WriteBasicBlock(
		struct BasicBlockPointer bbp,
		unsigned int cost,
		unsigned int jumpType,
		unsigned int jumpInstruction,
		unsigned int bbpNextSize,
		struct BasicBlockPointer *bbpNext) {
	char line[PATH_MAX * (bbpNextSize + 1) + 50];
	int sz = sprintf(line, "%-15s + %08X (%4d) (%4d) (%4d)",
		bbp.modName,
		bbp.offset,
		cost,
		jumpType,
		jumpInstruction
	);

	for (int i = 0; i < bbpNextSize; ++i) {
		sz += sprintf(line + sz, " %-15s + %08X", bbpNext[i].modName,
				bbpNext[i].offset);
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

/*void TextLog::FlushLog() {
	fflush(fLog);
}*/
