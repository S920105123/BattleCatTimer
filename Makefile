
INCLUDEPATH = "-L${CURDIR}"

GCCINCLUDE += -Isrc/header/
GCCINCLUDE += -Isrc/file_reader/
GCCINCLUDE += -Isrc/debug/
GCCINCLUDE += -Isrc/spef/
GCCINCLUDE += -Isrc/liberty/

GCCFLAG = g++ $(INCLUDEPATH) $(GCCINCLUDE) -std=c++14 -Wall

HEADER_SRC      = src/header/func.cpp
LOGGER_SRC      = src/debug/logger.cpp
DEBUG_SRC       = src/debug/debug.cpp
FILE_READER_SRC = src/file_reader/file_reader.cpp
SPEF_SRC        = src/spef/spef.cpp
VERILOG_SRC     = src/verilog/verilog.cpp
LIBERTY_SRC     = src/liberty/cell_lib.cpp
LIBERTY_SRC    += src/liberty/lu_table_template.cpp
LIBERTY_SRC    += src/liberty/cell.cpp
LIBERTY_SRC    += src/liberty/pin.cpp
LIBERTY_SRC    += src/liberty/timing_arc.cpp
LIBERTY_SRC    += src/liberty/timing_table.cpp

HEADER_OBJECT   = logger.o debug.o
LIBERTY_OBJECT  = cell_lib.o lu_table_template.o pin.o timing_arc.o timing_table.o

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

$(HEADER_OBJECT): $(HEADER_SRC)
	$(GCCFLAG) -c $(HEADER_SRC) -o $(HEADER_OBJECT)

$(LIBERTY_OBJECT): $(LIBERTY_SRC)
	$(GCCFLAG) -c $(LIBERTY_SRC)

logger.o:  $(LOGGER_SRC)
	$(GCCFLAG) -c $(LOGGER_SRC) -o logger.o

debug.o:  $(DEBUG_SRC)
	$(GCCFLAG) -c $(DEBUG_SRC) -o debug.o

file_reader.o: $(FILE_READER_SRC)
	$(GCCFLAG) -c $(FILE_READER_SRC) -o file_reader.o

spef.o: $(SPEF_SRC)
	$(GCCFLAG) -c $(SPEF_SRC) -o spef.o

main: file_reader.o $(HEADER_OBJECT) $(LIBERTY_OBJECT)
	$(GCCFLAG) $(HEADER_OBJECT) $(LIBERTY_OBJECT) file_reader.o test_main.cpp  -o main
	main.exe

clean:
	del *.o
	del *.exe
	del *.log

rm_clean:
	rm *.o
	rm *.exe
	rm *.log
