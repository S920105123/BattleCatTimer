#include "bct_writer.h"

Writer::Writer(const string& filename) {
	fp = fopen(filename.c_str(), "w");
	len = 0;
}

Writer::~Writer() {
}

void Writer::addchar(const char c) {
	buf[len++] = c;

	if(len == WRITER_BUFFER_MAX_LEN) {
		fwrite(buf, sizeof(char), len, fp);
		len = 0;
	}

}

void Writer::addstring(const char* x) {
	for(int i=0; x[i]; i++) {
		buf[len++] = x[i];
		if(len == WRITER_BUFFER_MAX_LEN) {
			fwrite(buf, sizeof(char), len, fp);
			len = 0;
		}
	}
}

void Writer::close() {
	fwrite(buf, sizeof(char), len, fp);
	fclose(fp);
}
