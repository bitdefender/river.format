#include "BinFormat.h"

#include <string.h>

#include <cstdlib>

#include "CommonCrossPlatform/Common.h" //MAX_PATH

#define MAX_AST 4096

BinFormat::BinFormat(AbstractLog *l, bool shouldBufferEntries)
: AbstractFormat(l)
{
	lastModule[0] 		= '\0';
	lastNextModule[0] 	= '\0';
	bufferingEntries 	= shouldBufferEntries;
	bufferEntries 		= shouldBufferEntries ? new unsigned char[MAX_ENTRIES_BUFFER_SIZE] : nullptr;
	bufferHeaderPos 	= 0;
}

BinFormat::~BinFormat()
{
	if (bufferEntries)
	{
		delete [] bufferEntries;
		bufferEntries = nullptr;
	}
}

void BinFormat::OnExecutionBegin(const char* testName)
{
	bufferHeaderPos = 0;
	lastModule[0] = '\0';
	lastNextModule[0] = '\0';
	WriteTestName(testName);
}

void BinFormat::OnExecutionEnd()
{
	if (!bufferEntries) {
		log->Flush();
	}
	else {
		// Write the total number of bytes of the output buffer, then the buffered data
		const size_t totalSizeToWrite = sizeof(bufferEntries[0]) * bufferHeaderPos;
		log->WriteBytes((unsigned char*)&totalSizeToWrite, sizeof(totalSizeToWrite));
		log->WriteBytes(bufferEntries, sizeof(bufferEntries[0]) * bufferHeaderPos);
		log->Flush();
	}
}

bool BinFormat::WriteBBModule(const char *moduleName, unsigned short type) {
	char *refModule = lastModule;
	if (type == ENTRY_TYPE_BB_NEXT_MODULE) {
		refModule = lastNextModule;
	}

	if (strcmp(refModule, moduleName)) {
		unsigned char buff[MAX_PATH + sizeof(BinLogEntry)];
		BinLogEntry *blem = (BinLogEntry *)buff;
		char *name = (char *)&blem->data;
		blem->header.entryType = type;
		blem->header.entryLength = (unsigned short)strlen(moduleName) + 1;

		if (strlen(moduleName) >= MAX_PATH) {
			return false;
		}

		memcpy(name, moduleName, blem->header.entryLength);
		memcpy(refModule, moduleName, blem->header.entryLength);

		WriteData(buff, sizeof(blem->header) + blem->header.entryLength);
	}
	return true;
}

bool BinFormat::WriteAst(SymbolicAst ast) {
	if (ast.size > MAX_AST + sizeof(BinLogEntry)) return false;

	unsigned char buff[MAX_AST + sizeof(BinLogEntry)];
	BinLogEntry *blem = (BinLogEntry *)buff;
	char *dest_ast = (char *)&blem->data;
	blem->header.entryType = ENTRY_TYPE_Z3_AST;
	blem->header.entryLength = ast.size + 1;

	dest_ast[ast.size] = 0;
	memcpy(dest_ast, ast.address, ast.size);

	WriteData(buff, sizeof(blem->header) + blem->header.entryLength);

	return true;
}

bool BinFormat::WriteBasicBlock(struct BasicBlockMeta bbm) {
	WriteBBModule(bbm.bbp.modName, ENTRY_TYPE_BB_MODULE);

	BinLogEntry bleo;
	bleo.header.entryType = ENTRY_TYPE_BB_OFFSET;
	bleo.header.entryLength = sizeof(bleo.data.asBBOffset);
	bleo.data.asBBOffset.offset = bbm.bbp.offset;
	bleo.data.asBBOffset.cost = bbm.cost;
	bleo.data.asBBOffset.jumpType = bbm.jumpType;
	bleo.data.asBBOffset.jumpInstruction = bbm.jumpInstruction;
	bleo.data.asBBOffset.esp = bbm.esp;
	bleo.data.asBBOffset.nInstructions = bbm.nInstructions;

	WriteData((unsigned char *)&bleo,
			sizeof(bleo.header) + bleo.header.entryLength);

	for (unsigned int i = 0; i < bbm.bbpNextSize; ++i) {
		WriteBBModule(bbm.bbpNext[i].modName, ENTRY_TYPE_BB_NEXT_MODULE);
		BinLogEntry nobleo;
		nobleo.header.entryType = ENTRY_TYPE_BB_NEXT_OFFSET;
		nobleo.header.entryLength = sizeof(nobleo.data.asBBNextOffset);
		nobleo.data.asBBNextOffset.offset = bbm.bbpNext[i].offset;
		WriteData((unsigned char *)&nobleo, sizeof(nobleo.header) +
				sizeof(nobleo.data.asBBNextOffset));
	}

	return true;
}

void BinFormat::WriteData(unsigned char* data, const unsigned int size, const bool ignoreInBufferedMode)
{
	if (!bufferingEntries)
	{
		log->WriteBytes(data, size);
	}
	else
	{
		if (!ignoreInBufferedMode)
		{
			if (bufferHeaderPos + size >= MAX_ENTRIES_BUFFER_SIZE)
			{
				exit(1);
			}

			memcpy(&bufferEntries[bufferHeaderPos], data, size);
			bufferHeaderPos += size;
		}
	}
}

bool BinFormat::WriteTestName(
	const char *testName
) {
	if (testName == nullptr)
		return false;

	unsigned char buff[MAX_PATH + sizeof(BinLogEntryHeader)];
	BinLogEntryHeader *bleh = (BinLogEntryHeader *)buff;
	char *name = (char *)&bleh[1];
	bleh->entryType = ENTRY_TYPE_TEST_NAME;
	bleh->entryLength = (unsigned short)strlen(testName) + 1;
	strcpy(name, testName);

	WriteData(buff, sizeof(*bleh) + bleh->entryLength, true);

	// also reset current module
	lastModule[0] = '\0';
	return true;
}

bool BinFormat::WriteInputUsage(unsigned int offset) {
	BinLogEntry bleo;
	bleo.header.entryType = ENTRY_TYPE_INPUT_USAGE;
	bleo.header.entryLength = sizeof(bleo.data.asInputUsage);
	bleo.data.asInputUsage.offset = offset;

	log->WriteBytes((unsigned char *)&bleo, sizeof(bleo.header) +
			bleo.header.entryLength);
	return true;
}

bool BinFormat::WriteTaintedIndexPayload(unsigned int dest,
		unsigned int source) {
	BinLogEntry bleo;
	bleo.header.entryType = ENTRY_TYPE_TAINTED_INDEX;
	bleo.data.asTaintedIndex.header.destIndex = dest;
	bleo.data.asTaintedIndex.header.entryType = TAINTED_INDEX_TYPE_PAYLOAD;
	bleo.data.asTaintedIndex.source.taintedIndexPayload.payloadIndex = source;

	bleo.data.asTaintedIndex.header.entryLength =
		sizeof(bleo.data.asTaintedIndex.source.taintedIndexPayload);
	bleo.header.entryLength = sizeof(TaintedIndexHeader) +
		bleo.data.asTaintedIndex.header.entryLength;
	log->WriteBytes((unsigned char *)&bleo,
			sizeof(bleo.header) + bleo.header.entryLength);

	return true;
}
bool BinFormat::WriteTaintedIndexExtract(unsigned int dest,
		unsigned int source, unsigned int lsb, unsigned int size) {
	BinLogEntry bleo;
	bleo.header.entryType = ENTRY_TYPE_TAINTED_INDEX;
	bleo.data.asTaintedIndex.header.destIndex = dest;
	bleo.data.asTaintedIndex.header.entryType = TAINTED_INDEX_TYPE_EXTRACT;
	bleo.data.asTaintedIndex.source.taintedIndexExtract.index = source;
	bleo.data.asTaintedIndex.source.taintedIndexExtract.lsb = lsb;
	bleo.data.asTaintedIndex.source.taintedIndexExtract.size = size;

	bleo.data.asTaintedIndex.header.entryLength =
		sizeof(bleo.data.asTaintedIndex.source.taintedIndexExtract);

	bleo.header.entryLength = sizeof(TaintedIndexHeader) +
		bleo.data.asTaintedIndex.header.entryLength;
	log->WriteBytes((unsigned char *)&bleo,
			sizeof(bleo.header) + bleo.header.entryLength);
	return true;
}
bool BinFormat::WriteTaintedIndexConcat(unsigned int dest,
		unsigned int operands[2]) {
	BinLogEntry bleo;
	bleo.header.entryType = ENTRY_TYPE_TAINTED_INDEX;
	bleo.data.asTaintedIndex.header.destIndex = dest;
	bleo.data.asTaintedIndex.header.entryType = TAINTED_INDEX_TYPE_CONCAT;

	for (int i = 0; i < 2; ++i) {
		bleo.data.asTaintedIndex.source.taintedIndexConcat.operands[i] = operands[i];
	}

	bleo.data.asTaintedIndex.header.entryLength =
		sizeof(bleo.data.asTaintedIndex.source.taintedIndexConcat);

	bleo.header.entryLength = sizeof(TaintedIndexHeader) +
		bleo.data.asTaintedIndex.header.entryLength;
	log->WriteBytes((unsigned char *)&bleo,
			sizeof(bleo.header) + bleo.header.entryLength);

	return true;
}

bool BinFormat::WriteTaintedIndexExecute(unsigned int dest,
		BasicBlockPointer bbp,
		unsigned int flags, unsigned int depsSize,
		unsigned int *deps) {
	// Write module name for the instruction that propagates the taint
	WriteBBModule(bbp.modName, TAINTED_INDEX_TYPE_MODULE);

	BinLogEntry bleo;
	bleo.header.entryType = ENTRY_TYPE_TAINTED_INDEX;
	bleo.data.asTaintedIndex.header.destIndex = dest;
	bleo.data.asTaintedIndex.header.entryType = TAINTED_INDEX_TYPE_EXECUTE;
	bleo.data.asTaintedIndex.source.taintedIndexExecute.offset = bbp.offset;
	bleo.data.asTaintedIndex.source.taintedIndexExecute.flags = flags;
	bleo.data.asTaintedIndex.source.taintedIndexExecute.depsSize = depsSize;

	for (int i = 0; i < depsSize; ++i) {
		bleo.data.asTaintedIndex.source.taintedIndexExecute.deps[i] =
			deps[i];
	}

	bleo.data.asTaintedIndex.header.entryLength =
		sizeof(bleo.data.asTaintedIndex.source.taintedIndexExecute.flags) +
		sizeof(bleo.data.asTaintedIndex.source.taintedIndexExecute.depsSize) +
		depsSize * sizeof(bleo.data.asTaintedIndex.source.taintedIndexExecute.deps[0]);

	bleo.header.entryLength = sizeof(TaintedIndexHeader) +
		bleo.data.asTaintedIndex.header.entryLength;
	log->WriteBytes((unsigned char *)&bleo,
			sizeof(bleo.header) + bleo.header.entryLength);
	return true;
}

bool BinFormat::WriteZ3SymbolicAddress(unsigned int dest,
		SymbolicAddress symbolicAddress, SymbolicAst ast) {

	// Write module for instruction that references current symbolic address
	WriteBBModule(symbolicAddress.bbp.modName, ENTRY_TYPE_Z3_MODULE);

	BinLogEntry bleo;
	bleo.header.entryType = ENTRY_TYPE_Z3_SYMBOLIC;
	bleo.data.asZ3Symbolic.header.entryType = Z3_SYMBOLIC_TYPE_ADDRESS;

	bleo.data.asZ3Symbolic.source.z3SymbolicAddress.offset = symbolicAddress.bbp.offset;
	bleo.data.asZ3Symbolic.source.z3SymbolicAddress.symbolicBase = symbolicAddress.symbolicBase;
	bleo.data.asZ3Symbolic.source.z3SymbolicAddress.scale = symbolicAddress.scale;
	bleo.data.asZ3Symbolic.source.z3SymbolicAddress.symbolicIndex = symbolicAddress.symbolicIndex;
	bleo.data.asZ3Symbolic.source.z3SymbolicAddress.displacement = symbolicAddress.displacement;
	bleo.data.asZ3Symbolic.source.z3SymbolicAddress.input = symbolicAddress.inputOutput & INPUT_ADDR;
	bleo.data.asZ3Symbolic.source.z3SymbolicAddress.output = symbolicAddress.inputOutput & OUTPUT_ADDR;

	bleo.data.asZ3Symbolic.header.entryLength =
		sizeof(bleo.data.asZ3Symbolic.source.z3SymbolicAddress);

	bleo.header.entryLength = sizeof(Z3SymbolicHeader) +
		bleo.data.asZ3Symbolic.header.entryLength;

	log->WriteBytes((unsigned char *)&bleo, sizeof(bleo.header) +
			bleo.header.entryLength);

	WriteAst(ast);

	return true;
}

bool BinFormat::WriteZ3SymbolicJumpCC(unsigned int dest,
		SymbolicFlag symbolicFlag, SymbolicAst ast) {
	BinLogEntry bleo;
	bleo.header.entryType = ENTRY_TYPE_Z3_SYMBOLIC;
	bleo.data.asZ3Symbolic.header.entryType = Z3_SYMBOLIC_TYPE_JCC;

	bleo.data.asZ3Symbolic.source.z3SymbolicJumpCC.symbolicCond = symbolicFlag.symbolicCond;
	bleo.data.asZ3Symbolic.source.z3SymbolicJumpCC.testFlags = symbolicFlag.testFlags;

	for (int i = 0; i < FLAG_LEN; ++i) {
		bleo.data.asZ3Symbolic.source.z3SymbolicJumpCC.symbolicFlags[i] = symbolicFlag.symbolicFlags[i];
	}

	bleo.data.asZ3Symbolic.header.entryLength =
		sizeof(bleo.data.asZ3Symbolic.source.z3SymbolicJumpCC);

	bleo.header.entryLength = sizeof(Z3SymbolicHeader) +
		bleo.data.asZ3Symbolic.header.entryLength;

	log->WriteBytes((unsigned char *)&bleo, sizeof(bleo.header) +
			bleo.header.entryLength);

	WriteAst(ast);

	return true;
}

/*bool BinLog::_OpenLog() {
	fLog = fopen(logName, "wb");

	return fLog != nullptr;
}

bool BinLog::_CloseLog() {
	fclose(fLog);
	fLog = nullptr;
	return true;
}

bool BinLog::_WriteTestName(const char *testName) {
	unsigned char buff[MAX_PATH + sizeof(BinLogEntryHeader)];
	BinLogEntryHeader *bleh = (BinLogEntryHeader *)buff;
	char *name = (char *)&bleh[1];
	bleh->entryType = ENTRY_TYPE_TEST_NAME;
	bleh->entryLength = (unsigned short)strlen(testName) + 1;
	strcpy(name, testName);
	
	fwrite(buff, sizeof(*bleh) + bleh->entryLength, 1, fLog);

	// also reset current module
	lastModule[0] = '\0';
	return true;
}

bool BinLog::_WriteBasicBlock(const char *module, unsigned int offset, unsigned int cost, unsigned int jumpType) {
	if (strcmp(lastModule, module)) {
		unsigned char buff[MAX_PATH + sizeof(BinLogEntry)];
		BinLogEntry *blem = (BinLogEntry *)buff;
		char *name = (char *)&blem->data;
		blem->header.entryType = ENTRY_TYPE_BB_MODULE;
		blem->header.entryLength = (unsigned short)strlen(module) + 1;
		strcpy(name, module);
		strcpy(lastModule, module);

		fwrite(buff, sizeof(blem->header) + blem->header.entryLength, 1, fLog);
	}

	BinLogEntry bleo;
	bleo.header.entryType = ENTRY_TYPE_BB_OFFSET;
	bleo.header.entryLength = sizeof(bleo.data.asBBOffset);
	bleo.data.asBBOffset.offset = offset;
	bleo.data.asBBOffset.cost = cost;
	bleo.data.asBBOffset.jumpType = jumpType;

	fwrite(&bleo, sizeof(bleo), 1, fLog);
	fflush(fLog);
	return true;
}

void BinLog::FlushLog() {
	fflush(fLog);
}*/


