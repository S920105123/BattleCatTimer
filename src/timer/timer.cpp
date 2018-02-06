#include "timer.h"


Timer::~Timer(){
    delete verilog;
    delete spef;
    delete lib[0];
    delete lib[1];
    delete graph;

    Logger::create()->~Logger();
}

void Timer::run(const string& tau, const string& timing, const string& ops, const string&output){
    open_tau(tau);
    graph = new Graph();
    graph->build(*verilog, *spef, *lib[EARLY], *lib[LATE]);

    open_timing(timing); // init graph
    // graph->cal();
    open_ops(ops);
}

void Timer::open_tau(const string& tau){
    File_Reader in;
    in.open(tau);

    string base = "";
    for(int i=tau.size()-1, j =0; i>=0; i--){
        if(tau[i]=='/') j++;
        if(j) base += tau[i];
    }
    std::reverse(base.begin(), base.end());

    string file_lib_early = base + in.next_token();
    string file_lib_late  = base + in.next_token();
    string file_spef      = base + in.next_token();
    string file_verilog   = base + in.next_token();
    verilog = new Verilog();
    spef = new Spef();
    lib[EARLY] = new CellLib(EARLY);
    lib[LATE]  = new CellLib(LATE);

    verilog->open( file_verilog );
    spef->open( file_spef);
    lib[EARLY]->open( file_lib_early);
    lib[LATE]->open( file_lib_late );
}

void Timer::open_timing(const string& timing){
    File_Reader in;
    in.open(timing);

    string cmd, pin;
    while((cmd = in.next_token())!=""){
        if(cmd=="clock"){
            pin = in.next_token();
            float low = stof(in.next_token());
            // graph set clock(pin, low);
            LOG(CERR) << "clock " << pin << " " << low << endl;
        }
        else if(cmd=="at"){
            pin = in.next_token();
            string tmp = in.next_token();
            if(tmp==":") LOG(ERROR) << "[Timer][open_timing] at pin is not primary " << pin << endl;
            float early_rise = stof(tmp);
            float early_fall = stof(in.next_token());
            float late_rise  = stof(in.next_token());
            float late_fall  = stof(in.next_token());
            // graph set at
            LOG(CERR) << "at " << pin << " " << early_rise << " " << early_fall;
            LOG(CERR) << " " << late_rise << " " << late_fall << endl;
        }
        else if(cmd=="slew"){
            pin = in.next_token();
            string tmp = in.next_token();
            if(tmp==":") LOG(ERROR) << "[Timer][open_timing] slew pin is not primary " << pin << endl;
            float early_rise = stof(tmp);
            float early_fall = stof(in.next_token());
            float late_rise  = stof(in.next_token());
            float late_fall  = stof(in.next_token());
            // graph set slew
            LOG(CERR) << "slew " << pin << " " << early_rise << " " << early_fall;
            LOG(CERR) << " " << late_rise << " " << late_fall << endl;
        }
        else if(cmd=="rat"){
            pin = in.next_token();
            string tmp = in.next_token();
            if(tmp==":") LOG(ERROR) << "[Timer][open_timing] slew pin is not primary " << pin << endl;
            float early_rise = stof(tmp);
            float early_fall = stof(in.next_token());
            float late_rise  = stof(in.next_token());
            float late_fall  = stof(in.next_token());
            // graph set rat
            LOG(CERR) << "rat " << pin << " " << early_rise << " " << early_fall;
            LOG(CERR) << " " << late_rise << " " << late_fall << endl;
        }
        else if(cmd=="load"){
            pin = in.next_token();
            string tmp = in.next_token();
            if(tmp==":") LOG(ERROR) << "[Timer][open_timing] load pin is not primary out" << pin << endl;
            float cap = stof(tmp);
            // graph set cap
            LOG(CERR) << "load " << pin << " " << cap << endl;
        }
        else{
            LOG(ERROR) << "[Timer][open_timing] unknown keyword. " << cmd << endl;
        }
    }
}

void Timer::open_ops(const string& ops){
    File_Reader in;
    in.open(ops);

    string cmd = in.next_token(),name;
    Transition_Type transition;
    Mode mode;
    do{
        if(cmd=="") break;
        /* Timing assertions */
        if(cmd=="set_at"){
            string pin;
            string op = in.next_token();
            do{
                if(op=="-pin"){
                    pin = in.next_token();
                    if((tmp=in.next_token())==":"){
                        pin += tmp;
                        pin += in.next_token();
                    }
                    else op = tmp;
                }
            }while(true);
        }
        else if(cmd=="set_slew"){

        }
        else if(cmd=="set_rat"){

        }
        else if(cmd=="set_load"){

        }
        /* Timing queryies */
        else if(cmd=="report_at"){

        }
        else if(cmd=="report_rat"){

        }
        else if(cmd=="report_slack"){

        }
        else if(cmd=="report_slew"){

        }
        else if(cmd=="report_worst_paths"){

        }
        /* circuit modification */
        else if(cmd=="insert_gate"){

        }
        else if(cmd=="repower_gate"){

        }
        else if(cmd=="remove_gate"){

        }
        /* net-level */
        else if(cmd=="insert_net"){

        }
        else if(cmd=="read_spef"){

        }
        else if(cmd=="remove_net"){

        }
        /* pin-level*/
        else if(cmd=="connect_pin"){

        }
        else if(cmd=="disconnect_pin"){

        }
        else{
            LOG(ERROR) << "[Timer][open_ops] unknown keyword " << cmd << endl;
        }
    }while(true);
}
