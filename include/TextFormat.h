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

	virtual bool WriteBasicBlock(struct BasicBlockMeta bbm);

	virtual bool WriteInputUsage(unsigned int offset);
	unsigned int WriteVariables(char * line, unsigned int sz);

	virtual bool WriteTaintedIndexPayload(unsigned int dest,
			unsigned int source);
	virtual bool WriteTaintedIndexExtract(unsigned int dest,
			unsigned int source, unsigned int lsb, unsigned int size);
	virtual bool WriteTaintedIndexConcat(unsigned int dest,
			unsigned int operands[2]);
	virtual bool WriteTaintedIndexExecute(unsigned int dest,
			unsigned int flags, unsigned int depsSize,
			unsigned int *deps);
};

#endif
