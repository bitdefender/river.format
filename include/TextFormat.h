#ifndef _TEXT_FORMAT_H_
#define _TEXT_FORMAT_H_

#include <stdio.h>

#include "AbstractLog.h"


class TextFormat : public AbstractFormat {
public :
	TextFormat(AbstractLog *l) : AbstractFormat(l) {}
	~TextFormat() {}

	virtual bool WriteTestName(
		const char *testName
	);

	virtual bool WriteRegisters(rev::ExecutionRegs &regs);

	virtual bool WriteBasicBlock(struct BasicBlockMeta bbm);

	virtual bool WriteInputUsage(unsigned int offset);
	unsigned int WriteVariables(char * line, unsigned int sz);

	virtual bool WriteTaintedIndexPayload(unsigned int dest,
			unsigned int source, unsigned int size);
	virtual bool WriteTaintedIndexExtract(unsigned int dest,
			unsigned int source, unsigned int lsb, unsigned int size);
	virtual bool WriteTaintedIndexConcat(unsigned int dest,
			unsigned int operands[2]);
	virtual bool WriteTaintedIndexExecute(unsigned int dest,
			BasicBlockPointer bbp, unsigned int flags, unsigned int depsSize,
			unsigned int *deps);
	virtual bool WriteTaintedIndexConst(unsigned int dest,
			unsigned int value, unsigned int size);
	virtual bool WriteZ3SymbolicAddress(unsigned int dest,
			SymbolicAddress symbolicAddress, SymbolicAst ast);
	virtual bool WriteZ3SymbolicJumpCC(unsigned int dest,
			SymbolicFlag symbolicFlag, SymbolicAst ast);

private:
	void WriteAst(SymbolicAst ast);
};

#endif
