#ifndef LU_TABLE_TEMPLATE_H
#define LU_TABLE_TEMPLATE_H

#include "header.h"
#include "logger.h"
#include "file_reader.h"

class LuTableTemplate{

public:
    void read(File_Reader &in);

private:
    string label_name;
    Lu_Table_Variable var1, var2;
    float *index1, *index2;
};

#endif /* end LU_TABLE_TEMPLATE_H */
