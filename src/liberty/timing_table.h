#ifndef TIMINGTABLE_H
#define TIMINGTABLE_H

#include "header.h"
#include "debug.h"
#include "logger.h"
#include "file_reader.h"

class CellLib;

class TimingTable{

public:
    TimingTable(CellLib *_cell_lib, const string& _table_tpye){
        cell_lib = _cell_lib;
        table_type = _table_tpye;
        constant_values = 0;
    }

    void read(File_Reader &in);
    void print(const string &tab="");

    void set_name(const string& name);

    // look-up table use linear polation
    // input_val: input_slew, output_val: cload/output_slew
    float get_value(float input_val, float output_val);
    // float get_value_constant(int precision=1);                           // if liberty is constant

private:
    // we use 1d array to store 2d data, so use this function to get 1d position
    int getValuesIndex(int i,int j);

    /*
            indx1   x    indx2
              v1   ans  v2
    */
    float liner_polation(float v1, float v2, float indx1, float indx2, float x);
    string label_name, table_type;
    vector<float> index1, index2;
    vector<float> values;

    CellLib *cell_lib;
};

#endif /* end TIMINGTABLE_H */
