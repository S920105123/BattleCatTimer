
INCLUDEPATH = "-L${CURDIR}"

GCCINCLUDE += -Isrc/header/
GCCINCLUDE += -Isrc/file_reader/
GCCINCLUDE += -Isrc/debug/
GCCINCLUDE += -Isrc/spef/

GCCFLAG = g++ $(INCLUDEPATH) $(GCCINCLUDE) -std=c++14 -Wall

HEADER_SRC      = src/header/func.cpp
FILE_READER_SRC = src/file_reader/file_reader.cpp
LOGGER_SRC      = src/debug/logger.cpp
SPEF_SRC        = src/spef/spef.cpp
DEBUG_SRC       = src/debug/debug.cpp

HEADER_OBJECT   = log.o header.o

TEST_FILE_READER:
	$(GCCFLAG) $(FILE_READER_SRC) $(HEADER_OBJECT) -DTEST_FILE_READER -o file_reader
	file_reader.exe

TEST_LOGGER: log.o
	$(GCCFLAG) $(LOGGER_SRC) -DTEST_LOGGER -o logger
	logger.exe

TEST_SPEF:  file_reader.o $(HEADER_OBJECT)
	$(GCCFLAG) $(HEADER_OBJECT) file_reader.o $(SPEF_SRC) -DTEST_SPEF -o spef
	spef.exe

TEST_DEBUG : log.o
	$(GCCFLAG) $(DEBUG_SRC) log.o -DTEST_DEBUG -o debug
	debug.exe

header.o: $(HEADER_SRC)
	$(GCCFLAG) -c $(HEADER_SRC) -o header.o

log.o:  $(LOGGER_SRC)
	$(GCCFLAG) -c $(LOGGER_SRC) -o log.o

file_reader.o: $(FILE_READER_SRC)
	$(GCCFLAG) -c $(FILE_READER_SRC) -o file_reader.o

spef.o: $(SPEF_SRC)
	$(GCCFLAG) -c $(SPEF_SRC) -o spef.o

main: file_reader.o $(HEADER_OBJECT) spef.o
	$(GCCFLAG) $(HEADER_OBJECT) spef.o file_reader.o test_main.cpp  -o main
	main.exe

clean:
	del *.o
	del *.exe
	del *.log
