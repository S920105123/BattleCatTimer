#ifndef CELL_H
#define CELL_H

#include "header.h"
#include "debug.h"
#include "logger.h"
#include "file_reader.h"
#include "pin.h"

class CellLib;
class Cell{

public:
    Cell(CellLib *_cell_lib){
        cell_lib = _cell_lib;
    }

    ~Cell(){
        for(auto it:pins) delete it.second;
    }

    void read(File_Reader &in);

    void set_name(const string& name);
    void add_pin(const string& name, Pin* pin);

    void print(const string &tab="");

    string get_type_name(){ return type_name; }

    float get_pin_capacitance(const string& pin_name);
    Direction_type get_pin_direction(const string& pin_name);
    bool get_pin_is_clock(const string& pin_name);
    vector<TimingArc*>* get_pin_TimingArc(const string& pin_name, const string& src);
    vector<TimingArc*>* get_pin_total_TimingArc(const string& pin_name);
    Pin* get_pin_ptr(const string& pin_name);

private:
    string type_name;
    unordered_map<string, Pin*> pins;
    CellLib* cell_lib;
};

#endif /* end CELL_H */
