
INCLUDEPATH = "-L${CURDIR}"

GCCINCLUDE += -Isrc/header/
GCCINCLUDE += -Isrc/file_reader/
GCCINCLUDE += -Isrc/debug/
GCCINCLUDE += -Isrc/spef/

GCCFLAG = g++ $(INCLUDEPATH) $(GCCINCLUDE) -std=c++14 -Wall

HEADER_SRC      = src/header/func.cpp
LOGGER_SRC      = src/debug/logger.cpp
DEBUG_SRC       = src/debug/debug.cpp
FILE_READER_SRC = src/file_reader/file_reader.cpp
SPEF_SRC        = src/spef/spef.cpp
VERILOG_SRC     = src/verilog/verilog.cpp

HEADER_OBJECT   = logger.o header.o debug.o

TEST_LOGGER: logger.o
	$(GCCFLAG) $(LOGGER_SRC) -DTEST_LOGGER -o logger
	logger.exe
	
TEST_DEBUG : logger.o
	$(GCCFLAG) $(DEBUG_SRC) logger.o -DTEST_DEBUG -o debug
	debug.exe

TEST_SPEF:  file_reader.o $(HEADER_OBJECT)
	$(GCCFLAG) $(HEADER_OBJECT) file_reader.o $(SPEF_SRC) -DTEST_SPEF -o spef
	spef.exe

TEST_VERILOG: file_reader.o $(HEADER_OBJECT)
	$(GCCFLAG) $(HEADER_OBJECT) file_reader.o $(VERILOG_SRC) -DTEST_VERILOG -o verilog
	verilog.exe

TEST_FILE_READER:
	$(GCCFLAG) $(FILE_READER_SRC) $(HEADER_OBJECT) -DTEST_FILE_READER -o file_reader
	file_reader.exe

header.o: $(HEADER_SRC)
	$(GCCFLAG) -c $(HEADER_SRC) -o header.o

logger.o:  $(LOGGER_SRC)
	$(GCCFLAG) -c $(LOGGER_SRC) -o logger.o

debug.o:  $(DEBUG_SRC)
	$(GCCFLAG) -c $(DEBUG_SRC) -o debug.o
	
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

clean_linux:
	rm *.o
	rm *.exe
	rm *.log
