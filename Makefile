
INCLUDEPATH = "-L${CURDIR}"

GCCINCLUDE += -Isrc/header/
GCCINCLUDE += -Isrc/file_reader/
GCCINCLUDE += -Isrc/debug/

GCCFLAG = g++ $(INCLUDEPATH) $(GCCINCLUDE) -std=c++14 -Wall

FILE_READER_SRC = src/file_reader/file_reader.cpp
LOGGER_SRC = src/debug/logger.cpp

TEST_FILE_READER : log.o
	$(GCCFLAG) $(FILE_READER_SRC) log.o -DTEST_FILE_READER -o file_reader
	file_reader.exe

TEST_LOGGER :
	$(GCCFLAG) $(LOGGER_SRC) -DTEST_LOGGER -o logger
	logger.exe

log.o:  $(LOGGER_SRC)
	$(GCCFLAG) -c $(LOGGER_SRC) -o log.o

file_reader.o: $(FILE_READER_SRC) log.o
	$(GCCFLAG) -c $(FILE_READER_SRC) -o file_reader.o

main: log.o
	$(GCCFLAG) log.o test_main.cpp  -o main
	main.exe

clean:
	del *.o
