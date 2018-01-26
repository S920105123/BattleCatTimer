#ifndef CELL_H
#define CELL_H

#include "header.h"
#include "logger.h"
#include "file_reader.h"
#include "Pin.h"

class Cell{

public:
    void read(File_Reader &in);

private:
    string type_name;
    unordered_map<string, Pin*> pins;
};

#endif /* end CELL_H */
