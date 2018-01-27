#ifndef LU_TABLE_TEMPLATE_H
#define LU_TABLE_TEMPLATE_H

#include "header.h"
#include "debug.h"
#include "logger.h"
#include "file_reader.h"

class LuTableTemplate{

public:
    LuTableTemplate(){
        var1 = UNDEFINED_LU_TABLE_VARIABLE;
        var2 = UNDEFINED_LU_TABLE_VARIABLE;
    }
    void read(File_Reader &in);
    void print(const string &tab="");

    string get_variable_string(Lu_Table_Variable var);

    void set_name(string name);

private:
    Lu_Table_Variable judge_variable(string var);

    string label_name;
    Lu_Table_Variable var1, var2;
    vector<float> index1, index2;
};

#endif /* end LU_TABLE_TEMPLATE_H */
