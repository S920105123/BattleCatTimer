#ifndef TIMER_H
#define TIMER_H

#include "header.h"
#include "file_reader.h"
#include "debug.h"
#include "verilog.h"
#include "spef.h"
#include "liberty.h"
#include "rc_tree.h"
#include "graph.h"

class Timer{
public:
    ~Timer();
    void run(const string& tau, const string& timing, const string& ops, const string&output);

private:
    void clear_Timer();
    void init_timer();

    void open_tau(const string& tau);
    void open_timing(const string& timing);
    void open_ops(const string& ops);

    void read_timing_assertion_option(File_Reader& in, string &name, Mode &mode, Transition_Type &transition, float &val);
    void read_pin_name(File_Reader& in, string &name);

    ofstream output;
    Verilog *verilog;
    Spef* spef;
    CellLib* lib[2];
    Graph* graph;
};
#endif /* end TIMER_H */
