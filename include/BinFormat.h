#ifndef _BINFORMAT_H_
#define _BINFORMAT_H_

#include <stdio.h>

#include "AbstractLog.h"
#include "CommonCrossPlatform/Common.h" //MAX_PATH

#define ENTRY_TYPE_TEST_NAME		0x0010
#define ENTRY_TYPE_BB_MODULE		0x00B0
#define ENTRY_TYPE_BB_NEXT_MODULE	0x00C0
#define ENTRY_TYPE_BB_OFFSET		0x00BB
#define ENTRY_TYPE_INPUT_USAGE		0x00AA
#define ENTRY_TYPE_BB_NEXT_OFFSET	0x00A0
#define ENTRY_TYPE_TAINTED_INDEX	0x00D0

#define TAINTED_INDEX_TYPE_CONCAT 0x0001
#define TAINTED_INDEX_TYPE_EXTRACT 0x0010
#define TAINTED_INDEX_TYPE_PAYLOAD 0x0100
#define TAINTED_INDEX_TYPE_EXECUTE 0x1000

#define MAX_DEPS 20

struct BinLogEntryHeader {
	unsigned short entryType;
	unsigned short entryLength;
};

struct TaintedIndexHeader {
	unsigned short entryType;
	unsigned short entryLength;
	unsigned int destIndex;
};

struct BinLogEntry {
	BinLogEntryHeader header;

	union Data {
		struct AsBBOffset {
			// encodes the module name length (including trailing \0)
			// a value of 0 means the current basic block resides in the same module as the previous
			unsigned int offset;
			unsigned short cost;
			unsigned short jumpType;
			unsigned short jumpInstruction;
			unsigned short nInstructions;
		} asBBOffset;

		struct AsBBNextOffset {
			unsigned int offset;
		} asBBNextOffset;

		struct AsInputUsage {
			unsigned int offset;
		} asInputUsage;

		struct AsTaintedIndex {
			TaintedIndexHeader header;
			union Source {
				// source is original input index
				struct TaintedIndexPayload {
					unsigned int payloadIndex;
				} taintedIndexPayload;

				// operation implies bits extraction
				struct TaintedIndexExtract {
					unsigned int index;
					unsigned int lsb;
					unsigned int size;
				} taintedIndexExtract;

				// operation implies data concatenation
				struct TaintedIndexConcat {
					unsigned int operands[2];
				} taintedIndexConcat;

				// index changes after instruction execution
				// flag indices are first stored in deps
				// each flag bit is set in flags if it is tainted
				// after flags, deps contains data indices
				struct TaintedIndexExecute {
					unsigned int instrAddress;
					unsigned int flags;
					unsigned int depsSize;
					unsigned int deps[MAX_DEPS];
				} taintedIndexExecute;
			} source;
		} asTaintedIndex;

	} data;
};

/*class BinFormat : public AbstractFormat {
private:
	FILE *fLog;
	char lastModule[MAX_PATH];
protected :
	bool _OpenLog();
	bool _CloseLog();
	bool _WriteTestName(const char *testName);
	bool _WriteBasicBlock(const char *module, unsigned int offset, unsigned int cost, unsigned int jumpType);
public :
	virtual void FlushLog();
};*/

class BinFormat : public AbstractFormat {
private :
	char lastModule[MAX_PATH];
	char lastNextModule[MAX_PATH];

  // Variables below are used for buffering mode. 
	// The reason i need buffering is that communication to the tracer.simple process are done by pipes and we can't seek in a pipe.
	// What I do is to write all entries in the buffer at runtime, then when executon ends write data to pipe(stdout) [number of bytes used + buffer]
	bool bufferingEntries;										// True if buffering entries
	unsigned char* bufferEntries;								// If this is created with shouldBufferEntries = true => we'll buffer all entries and send them at once
	static const int MAX_ENTRIES_BUFFER_SIZE = 1024*1024*2;   	// Preallocated buffer used when buffering entries. If exceeded an exception occurs. TODO: recreate buffer when exceeded max size ?
	int bufferHeaderPos;

  // Writes data either to the internal buffer or to the log file depending on the type
	void WriteData(unsigned char* data, const unsigned int size, const bool ignoreInBufferedMode = false);
	bool WriteBBModule(const char *moduleName, unsigned short type);
public :
	BinFormat(AbstractLog *l, bool shouldBufferEntries=false);
	~BinFormat();

	virtual bool WriteTestName(
		const char *testName
	);

	virtual bool WriteBasicBlock(struct BasicBlockMeta bbm);

	virtual bool WriteInputUsage(unsigned int offset);

	virtual bool WriteTaintedIndexPayload(unsigned int dest,
			unsigned int source);
	virtual bool WriteTaintedIndexExtract(unsigned int dest,
			unsigned int source, unsigned int lsb, unsigned int size);
	virtual bool WriteTaintedIndexConcat(unsigned int dest,
			unsigned int operands[2]);
	virtual bool WriteTaintedIndexExecute(unsigned int dest, DWORD address,
			unsigned int flags, unsigned int depsSize,
			unsigned int *deps);

	// Callbacks to know about execution status and update internal data structures	
	void OnExecutionEnd() override;
	void OnExecutionBegin(const char* testName) override; // testName optional when running in buffered / flow mode (you can set it as nullptr)
};

#endif

