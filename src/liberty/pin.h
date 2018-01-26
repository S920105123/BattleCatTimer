#ifndef PIN_H
#define PIN_H

#include "header.h"
#include "logger.h"
#include "timing_arc.h"
#include "file_reader.h"

class Cell;

class Pin{

public:
    void read(File_Reader &in);

private:
    string direction;
    float capacitance;
    float max_capacitance;
    float min_capacitance;
    bool is_clock;
    unordered_map<string, TimingArc*> timing;

    Cell* parent;
};

#endif /* end PIN_H */
