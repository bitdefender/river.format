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

	virtual bool WriteBasicBlock(
		struct BasicBlockPointer bbp,
		unsigned int cost,
		unsigned int jumpType,
		unsigned int jumpInstruction,
		unsigned int bbpNextSize,
		struct BasicBlockPointer *bbpNext
	);

	virtual bool WriteInputUsage(unsigned int offset);
	unsigned int WriteVariables(char * line, unsigned int sz);
};

#endif
