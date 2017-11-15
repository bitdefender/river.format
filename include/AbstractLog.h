#ifndef _ABSTRACT_LOG_H_
#define _ABSTRACT_LOG_H_

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

#define MAX_VARCOUNT 1024

struct BasicBlockPointer {
	unsigned int offset;
	char modName[MAX_PATH];
};

struct BasicBlockMeta {
	struct BasicBlockPointer bbp;
	unsigned int cost;
	unsigned int jumpType;
	unsigned int jumpInstruction;
	unsigned int nInstructions;
	unsigned int bbpNextSize;
	struct BasicBlockPointer *bbpNext;
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

	virtual bool WriteBasicBlock(struct BasicBlockMeta bbm) = 0;

	// Maybe these need a better name ?
	virtual void OnExecutionEnd() {}
	virtual void OnExecutionBegin(const char* testName) { WriteTestName(testName); }

	virtual bool WriteInputUsage(unsigned int offset) = 0;
	virtual bool WriteTaintedIndexPayload(unsigned int dest,
			unsigned int source) = 0;
	virtual bool WriteTaintedIndexExtract(unsigned int dest,
			unsigned int source, unsigned int lsb, unsigned int size) = 0;
	virtual bool WriteTaintedIndexConcat(unsigned int dest,
			unsigned int operands[2]) = 0;
	virtual bool WriteTaintedIndexExecute(unsigned int dest,
			unsigned int flags, unsigned int depsSize,
			unsigned int *deps) = 0;

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
