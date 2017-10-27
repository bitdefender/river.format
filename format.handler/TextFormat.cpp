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

	for (int i = 0; i < bbm.bbpNextSize; ++i) {
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

/*void TextLog::FlushLog() {
	fflush(fLog);
}*/
