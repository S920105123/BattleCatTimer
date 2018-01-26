#ifndef TIMINGTABLE_H
#define TIMINGTABLE_H

#include "header.h"
#include "logger.h"
#include "file_reader.h"


class TimingTable{

public:
    void read(File_Reader &in);

private:
    string label_name;
    float *index1, *index2;
    float *values;
};

#endif /* end TIMINGTABLE_H */
