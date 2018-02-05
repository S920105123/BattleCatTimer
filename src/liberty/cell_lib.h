#ifndef CELLLIB_H
#define CELLLIB_H

#include "header.h"
#include "logger.h"
#include "debug.h"
#include "file_reader.h"
#include "lu_table_template.h"
#include "cell.h"

class CellLib{

public:
    ~CellLib(){
        for(auto it:table_template) delete it.second;
        for(auto it:cells) delete it.second;
    }

    void open(const string& filename);

    void add_table_template(const string& name, LuTableTemplate* table);
    void add_cell(const string& name, Cell* cell);

    int cells_size();
    int table_template_size();

    void print_cell(const string& name);
    void print_template();

    float get_pin_capacitance(const string& cell_type, const string& pin_name);
    bool get_pin_is_clock(const string& cell_type, const string& pin_name);
    Direction_type get_pin_direction(const string& cell_type, const string& pin_name);
    vector<TimingArc*>* get_pin_TimingArc(const string& cell_type, const string& pin_name, const string& src);       // TimingArc: from src to this pin
    vector<TimingArc*>* get_pin_total_TimingArc(const string& cell_type, const string& pin_name);             // TimingArc: total src to this pin

    Cell* get_cell_ptr(const string& cell_type);
    Pin* get_pin_ptr(const string& cell_type, const string& pin_name);

    LuTableTemplate* get_table_template(const string& table_label);
private:
    string name;
    unordered_map<string, LuTableTemplate*> table_template;
    unordered_map<string, Cell*> cells;
};

#endif /* end CELLLIB_H */
