#ifndef TIMINGARC_H
#define TIMINGARC_H

#include "header.h"
#include "logger.h"
#include "timing_table.h"
#include "file_reader.h"

class CellLib;

class TimingArc{

public:
    TimingArc(CellLib *_cell_lib){
        cell_lib = _cell_lib;

        timing_sense = UNDEFINED_TIMING_SENSE;
        timing_type  = COMBINATINAL;

        cell_fall_table = NULL;
        cell_rise_table = NULL;
        fall_transition_table = NULL;
        rise_transition_table = NULL;
        rise_constraint_table = NULL;
        fall_constraint_table = NULL;
    }

    ~TimingArc(){
        delete cell_fall_table;
        delete cell_rise_table;
        delete fall_transition_table;
        delete rise_transition_table;
        delete rise_constraint_table;
        delete fall_constraint_table;
    }

    void read(File_Reader &in);
    void print(const string &tab="");

    string get_related_pin();
    string get_timing_type_string();
    string get_timing_sense_string();

    void set_timing_sense(const string& val);
    void set_timing_type(const string& val);

    TimingTable* get_cell_fall_table(){ return cell_fall_table; }
    TimingTable* get_cell_rise_table(){ return cell_rise_table; }
    TimingTable* get_fall_transition_table(){ return fall_transition_table; }
    TimingTable* get_rise_transition_table(){ return rise_transition_table; }
    TimingTable* get_rise_constraint_table(){ return rise_constraint_table; }
    TimingTable* get_fall_constraint_table(){ return fall_constraint_table; }

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

    CellLib *cell_lib;
};

#endif /* end TIMINGARC_H */
