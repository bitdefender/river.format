#include "TextFormat.h"

bool TextFormat::WriteTestName(const char *testName) {
	char line[PATH_MAX + 10];
	int sz = sprintf(line, "## %s\n", testName);

	log->WriteBytes((unsigned char *)line, sz);
	return true;
}

bool TextFormat::WriteBasicBlock(const char *module,
		unsigned int offset,
		unsigned int cost,
		unsigned int jumpType,
		unsigned int jumpInstruction,
		unsigned int addressBranchTaken,
		unsigned int addressBranchNotTaken) {
	char line[PATH_MAX + 30];
	int sz = sprintf(line, "%-15s + %08X (%4d) (%4d) (%4d) (%p) (%p)\n",
		module,
		offset,
		cost,
		jumpType,
		jumpInstruction,
		addressBranchTaken,
		addressBranchNotTaken
	);

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
