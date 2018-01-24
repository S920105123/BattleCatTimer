
INCLUDEPATH = "-L${CURDIR}"

GCCINCLUDE += -Isrc/header/
GCCINCLUDE += -Isrc/parser/file_reader

GCCFLAG = g++ $(INCLUDEPATH) $(GCCINCLUDE) -std=c++14 -Wall

TEST_FILE_READER :
	$(GCCFLAG) src/parser/file_reader/file_reader.cpp -DTEST_FILE_READER -o file_reader
	file_reader.exe
