#ifndef TIMINGARC_H
#define TIMINGARC_H

#include "header.h"
#include "logger.h"
#include "timing_table.h"
#include "file_reader.h"

class TimingArc{

public:
    void read(File_Reader &in);

private:
    string related_pin;
    Timing_Sense sense;
    Timing_Type type;
    TimingTable *cell_fall, *fall_transition, *cell_rise, *cell_transition;
};

#endif /* end TIMINGARC_H */
