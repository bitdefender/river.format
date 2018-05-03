#ifndef _ABSTRACT_LOG_H_
#define _ABSTRACT_LOG_H_

#include "revtracer/revtracer.h"		//ExecutionRegs
#include "CommonCrossPlatform/Common.h" //MAX_PATH

class AbstractLog {
private :
	bool isLogOpen;

protected :
	virtual bool _OpenLog() = 0;
	virtual bool _CloseLog() = 0;

	bool OpenLog();
	bool CloseLog();
	bool IsLogOpen();

public :
	AbstractLog();
	virtual bool WriteBytes(unsigned char *buffer, unsigned int size) = 0;
	virtual bool Flush() = 0;
	virtual ~AbstractLog() {}
};

#define MAX_VARCOUNT	1024
#define FLAG_LEN	7
#define INPUT_ADDR	0x01
#define OUTPUT_ADDR		0x02
#define DISP8	0x01
#define DISP	0x04

#define MAX_LINE_SIZE 512

struct BasicBlockPointer {
	unsigned int offset;
	char modName[MAX_PATH];
};

struct BasicBlockMeta {
	struct BasicBlockPointer bbp;
	unsigned int cost;
	unsigned int jumpType;
	unsigned int jumpInstruction;
	unsigned int esp;
	unsigned int nInstructions;
	unsigned int bbpNextSize;
	struct BasicBlockPointer *bbpNext;
};

struct SymbolicAddress {
	unsigned int symbolicBase;
	unsigned int scale;
	unsigned int symbolicIndex;
	unsigned int composedSymbolicAddress;
	unsigned char dispType;
	unsigned displacement;
	unsigned char inputOutput;
	struct BasicBlockPointer bbp;
};

struct SymbolicFlag {
	unsigned int testFlags;
	unsigned int symbolicCond;
	unsigned int symbolicFlags[FLAG_LEN];
};

struct SymbolicAst {
	const char *address;
	size_t size;
};

class AbstractFormat {
protected :
	AbstractLog *log;

public :
	AbstractFormat(AbstractLog *l) {
		log = l;
	}
	virtual ~AbstractFormat() {}

	virtual bool WriteTestName(
		const char *testName
	) = 0;

	virtual bool WriteRegisters(rev::ExecutionRegs &regs) = 0;

	virtual bool WriteBasicBlock(struct BasicBlockMeta bbm) = 0;

	// Maybe these need a better name ?
	virtual void OnExecutionEnd() {}
	virtual void OnExecutionBegin(const char* testName) { WriteTestName(testName); }

	virtual bool WriteInputUsage(unsigned int offset) = 0;
	virtual bool WriteTaintedIndexPayload(unsigned int dest,
			unsigned int source, unsigned int size) = 0;
	virtual bool WriteTaintedIndexExtract(unsigned int dest,
			unsigned int source, unsigned int lsb, unsigned int size) = 0;
	virtual bool WriteTaintedIndexConcat(unsigned int dest,
			unsigned int operands[2]) = 0;
	virtual bool WriteTaintedIndexExecute(unsigned int dest, BasicBlockPointer bbp,
			unsigned int flags, unsigned int depsSize,
			unsigned int *deps) = 0;
	virtual bool WriteTaintedIndexConst(unsigned int dest,
			unsigned int value, unsigned int size) = 0;
	virtual bool WriteZ3SymbolicAddress(unsigned int dest,
			SymbolicAddress symbolicAddress, SymbolicAst ast) = 0;
	virtual bool WriteZ3SymbolicJumpCC(unsigned int dest,
			SymbolicFlag symbolicFlag, SymbolicAst ast) = 0;

};

/*class AbstractLog {
private :
	bool isLogOpen;
protected :
	char logName[PATH_LEN];
	virtual bool _OpenLog() = 0;
	virtual bool _CloseLog() = 0;

	bool OpenLog();
	bool CloseLog();
	
	virtual bool _WriteTestName(
		const char *testName
	) = 0;

	virtual bool _WriteBasicBlock(
		const char *module,
		unsigned int offset,
		unsigned int cost,
		unsigned int jumpType
	) = 0;
public :
	virtual bool SetLogFile(
		const char *log
	);

	virtual void FlushLog() = 0;

	bool WriteTestName(
		const char *testName
	);

	bool WriteBasicBlock(
		const char *module,
		unsigned int offset,
		unsigned int cost,
		unsigned int jumpType
	);

	virtual bool WriteInputUsage(
		unsigned int offset
	) = 0;

	virtual bool WriteTestResult(
	) = 0;
};*/

#endif
