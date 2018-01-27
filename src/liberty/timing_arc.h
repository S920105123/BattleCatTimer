#ifndef TIMINGARC_H
#define TIMINGARC_H

#include "header.h"
#include "logger.h"
#include "timing_table.h"
#include "file_reader.h"

class TimingArc{

public:
    TimingArc(){
        timing_sense = UNDEFINED_TIMING_SENSE;
        timing_type  = COMBINATINAL;

        cell_fall_table = NULL;
        cell_rise_table = NULL;
        fall_transition_table = NULL;
        rise_transition_table = NULL;
        rise_constraint_table = NULL;
        fall_constraint_table = NULL;
    }

    void read(File_Reader &in);
    void print(const string &tab="");

    string get_related_pin();
    string get_timing_type_string();
    string get_timing_sense_string();

    void set_timing_sense(string val);
    void set_timing_type(string val);

private:
    string related_pin;
    Timing_Sense timing_sense;
    Timing_Type timing_type;
    TimingTable *cell_fall_table;
    TimingTable *cell_rise_table;
    TimingTable *fall_transition_table;
    TimingTable *rise_transition_table;
    TimingTable *rise_constraint_table;
    TimingTable *fall_constraint_table;

};

#endif /* end TIMINGARC_H */
