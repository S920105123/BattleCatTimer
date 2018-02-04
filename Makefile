
INCLUDEPATH = "-L${CURDIR}"

GCCINCLUDE += -Isrc/header/
GCCINCLUDE += -Isrc/file_reader/
GCCINCLUDE += -Isrc/debug/
GCCINCLUDE += -Isrc/spef/
GCCINCLUDE += -Isrc/verilog/
GCCINCLUDE += -Isrc/liberty/
GCCINCLUDE += -Isrc/graph/

GCCFLAG = g++ $(INCLUDEPATH) $(GCCINCLUDE) -std=c++14 -Wall

HEADER_SRC      = src/header/func.cpp
HEADER_SRC      += src/debug/logger.cpp
HEADER_SRC      += src/debug/debug.cpp
FILE_READER_SRC = src/file_reader/file_reader.cpp
SPEF_SRC        = src/spef/spef.cpp
VERILOG_SRC     = src/verilog/verilog.cpp
LIBERTY_SRC     = src/liberty/cell_lib.cpp
LIBERTY_SRC    += src/liberty/lu_table_template.cpp
LIBERTY_SRC    += src/liberty/cell.cpp
LIBERTY_SRC    += src/liberty/pin.cpp
LIBERTY_SRC    += src/liberty/timing_arc.cpp
LIBERTY_SRC    += src/liberty/timing_table.cpp
DATA_SRC        = $(SPEF_SRC) $(VERILOG_SRC) $(LIBERTY_SRC)
RCTREE_SRC      = src/graph/rc_tree.cpp
GRAPH_SRC       = src/graph/graph.cpp

HEADER_OBJECT   = logger.o debug.o func.o
LIBERTY_OBJECT  = cell_lib.o lu_table_template.o pin.o timing_arc.o timing_table.o cell.o
DATA_OBEJCT     = verilog.o spef.o $(LIBERTY_OBJECT)

TEST_LOGGER:
	$(GCCFLAG) $(LOGGER_SRC) -DTEST_LOGGER -o logger
	logger.exe

TEST_DEBUG :
	$(GCCFLAG) $(HEADER_SRC) -DTEST_DEBUG -o debug
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

TEST_LU_TABLE_TEMPLATE: file_reader.o src/liberty/lu_table_template.cpp $(HEADER_OBJECT)
	$(GCCFLAG) file_reader.o $(HEADER_OBJECT) src/liberty/lu_table_template.cpp -DTEST_LU_TABLE_TEMPLAT -o lu_table_template
	lu_table_template.exe

TEST_TIMING_TABLE: file_reader.o src/liberty/timing_table.cpp $(HEADER_OBJECT)
	$(GCCFLAG) file_reader.o $(HEADER_OBJECT) $(LIBERTY_SRC) -DTEST_TIMING_TABLE -o timing_table
	timing_table.exe

TEST_TIMING_ARC: $(HEADER_OBJECT) file_reader.o src/liberty/timing_arc.cpp
	$(GCCFLAG) file_reader.o $(HEADER_OBJECT) $(LIBERTY_SRC) -DTEST_TIMING_ARC -o timing_arc
	timing_arc.exe

TEST_PIN: file_reader.o $(HEADER_OBJECT)
	$(GCCFLAG) file_reader.o $(HEADER_OBJECT) $(LIBERTY_SRC) -DTEST_PIN -o pin
	pin.exe

TEST_CELLLIB: file_reader.o $(HEADER_OBJECT)
	$(GCCFLAG) file_reader.o $(HEADER_OBJECT) $(LIBERTY_SRC) -o cell_lib -DTEST_CELLLIB
	cell_lib.exe

TEST_RCTREE: file_reader.o $(HEADER_OBJECT) $(DATA_OBEJCT)
	$(GCCFLAG) file_reader.o $(HEADER_OBJECT) $(DATA_OBEJCT) $(RCTREE_SRC) -o rc_tree2 -DTEST_RCTREE
	rc_tree2.exe

TEST_GRAPH: file_reader.o $(HEADER_OBJECT) $(DATA_OBEJCT)
	$(GCCFLAG) file_reader.o $(HEADER_OBJECT) $(DATA_OBEJCT) $(GRAPH_SRC) -o graph -DTEST_GRAPH
	graph.exe

$(HEADER_OBJECT): $(HEADER_SRC)
	$(GCCFLAG) -c $(HEADER_SRC)

$(DATA_OBEJCT): $(DATA_SRC)
	$(GCCFLAG) -c $(DATA_SRC)

# $(LIBERTY_OBJECT): $(LIBERTY_SRC)
# 	$(GCCFLAG) -c $(LIBERTY_SRC)

# header.o:  $(HEADER_SRC)
# 	$(GCCFLAG) -c $(HEADER_SRC) -o header.o

file_reader.o: $(FILE_READER_SRC)
	$(GCCFLAG) -c $(FILE_READER_SRC) -o file_reader.o

# spef.o: $(SPEF_SRC)
# 	$(GCCFLAG) -c $(SPEF_SRC) -o spef.o
#
# verilog.o: $(VERILOG_SRC)
# 	$(GCCFLAG) -c $(VERILOG_SRC) -o verilog.o

rc_tree.o: $(RCTREE_SRC)
	$(GCCFLAG) -c $(RCTREE_SRC) -o rc_tree.o
main: file_reader.o $(HEADER_OBJECT) $(DATA_OBJECT) rc_tree.o
	$(GCCFLAG) $(HEADER_OBJECT) $(DATA_OBEJCT) rc_tree.o file_reader.o test_main.cpp  -o main
	main.exe

clean:
	del *.o
	del *.exe
	del *.log

rm_clean:
	rm *.o
	rm *.exe
	rm *.log
