#ifndef CELLLIB_H
#define CELLLIB_H

#include "header.h"
#include "logger.h"
#include "file_reader.h"
#include "lu_table_template.h"
#include "cell.h"

class CellLib{

public:
    void open(string filename);

private:
    string name;
    unordered_map<string, LuTableTemplate*> table_template;
    unordered_map<string, Cell*> cells;
};

#endif /* end CELLLIB_H */
