#ifndef TIMINGTABLE_H
#define TIMINGTABLE_H

#include "header.h"
#include "debug.h"
#include "logger.h"
#include "file_reader.h"

class CellLib;

class TimingTable{

public:
    TimingTable(CellLib *_cell_lib){
        cell_lib = _cell_lib;
    }

    void read(File_Reader &in);
    void print(const string &tab="");

    void set_name(const string& name);

private:
    int getValuesIndex(int i,int j);
    string label_name;
    vector<float> index1, index2;
    vector<float> values;

    CellLib *cell_lib;
};

#endif /* end TIMINGTABLE_H */
