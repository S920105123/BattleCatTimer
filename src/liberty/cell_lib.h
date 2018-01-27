#ifndef CELLLIB_H
#define CELLLIB_H

#include "header.h"
#include "logger.h"
#include "debug.h"
#include "file_reader.h"
#include "lu_table_template.h"
#include "cell.h"

class CellLib{

public:
    void open(string filename);

    void add_table_template(string name, LuTableTemplate* table);
    void add_cell(string name, Cell* cell);

    int cells_size();
    int table_template_size();

    void print_cell(string name);
    void print_template();

private:
    string name;
    unordered_map<string, LuTableTemplate*> table_template;
    unordered_map<string, Cell*> cells;
};

#endif /* end CELLLIB_H */
