#ifndef TIMINGTABLE_H
#define TIMINGTABLE_H

#include "header.h"
#include "debug.h"
#include "logger.h"
#include "file_reader.h"


class TimingTable{

public:
    void read(File_Reader &in);
    void print(const string &tab="");

    void set_name(string name);

private:
    int getValuesIndex(int i,int j);
    string label_name;
    vector<float> index1, index2;
    vector<float> values;
};

#endif /* end TIMINGTABLE_H */
