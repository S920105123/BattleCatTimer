#ifndef BCT_WRITER
#define BCT_WRITER

#include "header.h"

#define WRITER_BUFFER_MAX_LEN 100000
class Writer{

public:
	Writer(const string& filename);
	~Writer();
	void addchar(const char c);
	void addstring(const char* x);
	void close();

private:
	FILE* fp;
	char buf[WRITER_BUFFER_MAX_LEN + 10];
	int len;
};

#endif
