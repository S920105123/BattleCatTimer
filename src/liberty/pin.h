#ifndef PIN_H
#define PIN_H

#include "header.h"
#include "logger.h"
#include "debug.h"
#include "timing_arc.h"
#include "file_reader.h"

class Cell;

class Pin{

	/*
	Expected format:
		{
			keword : value
			...
		}
	*/

public:
    void read(File_Reader &in);
    void print(const string &tab = "");
    void set_name(const string &new_name);
    void set_parent(Cell *p);

private:
    string name, direction;
    float capacitance;
    float max_capacitance;
    float min_capacitance;
    bool is_clock;
    unordered_map<string, TimingArc*> timing;
    Cell* parent;
};

#endif /* end PIN_H */
