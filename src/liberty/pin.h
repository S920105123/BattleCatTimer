#ifndef PIN_H
#define PIN_H

#include "header.h"
#include "logger.h"
#include "debug.h"
#include "timing_arc.h"
#include "file_reader.h"

class Cell;
class CellLib;

class Pin{

	/*
	Expected format:
		{
			keword : value
			...
		}
	*/

public:
    Pin(CellLib* _cell_lib) {
        cell_lib = _cell_lib;
        total_timingArc = new vector<TimingArc*>;
    }

    ~Pin() {
        for (auto &it : timing) delete it.second;
        for (auto &it : *total_timingArc) delete it;
        delete total_timingArc;
    }

    void read(File_Reader &in);
    void print(const string &tab = "");

    void add_arc(const string &src,TimingArc *arc);
    void set_name(const string &new_name);
    void set_parent(Cell *p);

    float get_capacitance();
    bool get_is_clock();
    vector<TimingArc*>* get_TimingArc(const string& src);       // TimingArc: from src to this pin
    vector<TimingArc*>* get_total_TimingArc();           // TimingArc: total src to this pin
    Direction_type get_direction();

private:
    string name, direction;
    float capacitance;
    float max_capacitance;
    float min_capacitance;
    bool is_clock;
    unordered_map<string, vector<TimingArc*>* > timing;
    vector<TimingArc*>* total_timingArc;

    Cell* parent;
    CellLib* cell_lib;
};

#endif /* end PIN_H */
