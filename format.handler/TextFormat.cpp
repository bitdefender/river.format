#include "TextFormat.h"
#include "revtracer/river.h"
#include "CommonCrossPlatform/Common.h" //MAX_PATH

#include <string.h>

bool TextFormat::WriteTestName(const char *testName) {
	char line[MAX_PATH + 10];
	int sz = sprintf(line, "## %s\n", testName);

	log->WriteBytes((unsigned char *)line, sz);
	sz = sprintf(line, "%-30s + %-8s %-6s %-6s %-6s %-10s %-6s %-30s - %-8s %-30s - %-8s\n",
			"moduleName", "offset", "cost", "jmp", "instr", "esp", "ninstr", "taken", "offset",
			"nottaken", "offset");
	log->WriteBytes((unsigned char *)line, sz);
	return true;
}

bool TextFormat::WriteBasicBlock(struct BasicBlockMeta bbm) {
	char line[MAX_PATH * (bbm.bbpNextSize + 1) + 50];
	int sz = sprintf(line, "%-30s + %08X (%4d) (%4d) (%4d) (%08X) (%4d)",
		bbm.bbp.modName,
		bbm.bbp.offset,
		bbm.cost,
		bbm.jumpType,
		bbm.jumpInstruction,
		bbm.esp,
		bbm.nInstructions
	);

	for (unsigned int i = 0; i < bbm.bbpNextSize; ++i) {
		sz += sprintf(line + sz, " %-30s + %08X", bbm.bbpNext[i].modName,
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
		unsigned int source, unsigned int size) {
	char line[20];
	int sz = sprintf(line, "I[%u] <= p[%u](%u)\n", dest, source, size);
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

bool TextFormat::WriteTaintedIndexConst(unsigned int dest,
			unsigned int value, unsigned int size) {
	char line[50];
	int sz = sprintf(line, "I[%u] <= const 0x", dest);

	switch(size) {
		case 8:
			sz += sprintf(line + sz, "%02X", (uint8_t)value);
			break;
		case 16:
			sz += sprintf(line + sz, "%04X", (uint16_t)value);
			break;
		case 32:
			sz += sprintf(line + sz, "%08X", (uint32_t)value);
			break;
		default:
			DEBUG_BREAK;

	}
	sz += sprintf(line + sz, "(%u)\n", size);
	log->WriteBytes((unsigned char *)line, sz);
	return true;
}

static const char flagNames[FLAG_LEN][3] = {
	"CF", "PF", "AF", "ZF", "SF", "OF", "DF"
};

static const unsigned char flagList[FLAG_LEN] = {
	RIVER_SPEC_FLAG_CF,
	RIVER_SPEC_FLAG_PF,
	RIVER_SPEC_FLAG_AF,
	RIVER_SPEC_FLAG_ZF,
	RIVER_SPEC_FLAG_SF,
	RIVER_SPEC_FLAG_OF,
	RIVER_SPEC_FLAG_DF
};

static const int flagCount = sizeof(flagNames) / sizeof(flagNames[0]);

bool TextFormat::WriteTaintedIndexExecute(unsigned int dest, BasicBlockPointer bbp,
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
	sz += sprintf(line + sz, " ( %-15s + %08X )", bbp.modName,
			bbp.offset);
	sz += sprintf(line + sz, "\n");
	log->WriteBytes((unsigned char *)line, sz);
	return true;
}

void TextFormat::WriteAst(SymbolicAst ast) {
	char before[] = "'''\n";
	char after[] = "\n'''\n";
	log->WriteBytes((unsigned char *)before, sizeof(before) - 1);
	log->WriteBytes((unsigned char *)ast.address, ast.size);
	log->WriteBytes((unsigned char *)after, sizeof(after) - 1);
}

bool TextFormat::WriteZ3SymbolicAddress(unsigned int dest, SymbolicAddress symbolicAddress, SymbolicAst ast) {
	size_t sz = 0;
	char line[MAX_LINE_SIZE];

	sz = sprintf(line, "0x%08X <= 0x%08X + 0x%02X x 0x%08X",
			symbolicAddress.composedSymbolicAddress,
			symbolicAddress.symbolicBase,
			(unsigned char)symbolicAddress.scale,
			symbolicAddress.symbolicIndex);

	if (symbolicAddress.dispType & DISP8) {
		sz += sprintf(line + sz, " + 0x%02X", (unsigned char)symbolicAddress.displacement);
	} else if (symbolicAddress.dispType & DISP) {
		sz += sprintf(line + sz, " + 0x%08X", symbolicAddress.displacement);
	}

	if (symbolicAddress.inputOutput & INPUT_ADDR) {
		sz += sprintf(line + sz, " | IN");
	}
	if (symbolicAddress.inputOutput & OUTPUT_ADDR) {
		sz += sprintf(line + sz, " | OUT");
	}

	sz += sprintf(line + sz, " ( %-15s + %08X )",
			symbolicAddress.bbp.modName,
			symbolicAddress.bbp.offset);

	sz += sprintf(line + sz, "\n");
	log->WriteBytes((unsigned char *)line, sz);

	WriteAst(ast);

	return true;
}

bool TextFormat::WriteZ3SymbolicJumpCC(unsigned int dest, SymbolicFlag symbolicFlag, SymbolicAst ast) {
	size_t sz;
	char line[MAX_LINE_SIZE];

	sz = sprintf(line, "jcc 0x%08X <=", symbolicFlag.symbolicCond);

	for (int i = 0; i < flagCount; ++i) {
		if (flagList[i] & symbolicFlag.testFlags) {
			sz += sprintf(line + sz, " %s[%08X]",
					flagNames[i], symbolicFlag.symbolicFlags[i]);
		}
	}
	sz += sprintf(line + sz, "\n");
	log->WriteBytes((unsigned char *)line, sz);

	WriteAst(ast);

	return true;
}

