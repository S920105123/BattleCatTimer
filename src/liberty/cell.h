#ifndef CELL_H
#define CELL_H

#include "header.h"
#include "debug.h"
#include "logger.h"
#include "file_reader.h"
#include "pin.h"

class Cell{

public:
    void read(File_Reader &in);

    void set_name(string name);
    void add_pin(string name, Pin* pin);

    void print(const string &tab="");

private:
    string type_name;
    unordered_map<string, Pin*> pins;
};

#endif /* end CELL_H */
