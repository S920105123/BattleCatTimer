
INCLUDEPATH = "-L${CURDIR}"

GCCINCLUDE += -Isrc/header/
GCCINCLUDE += -Isrc/parser/file_reader

GCCFLAG = g++ $(INCLUDEPATH) $(GCCINCLUDE) -std=c++14 -Wall

FILE_READER_SRC = src/parser/file_reader/file_reader.cpp

TEST_FILE_READER :
	$(GCCFLAG) $(FILE_READER_SRC) -DTEST_FILE_READER -o file_reader
	file_reader.exe
