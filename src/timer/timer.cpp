#include "timer.h"


Timer::~Timer(){

    clear_Timer();

    Logger::create()->~Logger();
    output.close();
}

void Timer::gen_test(const string& type, const string& tau, const string& output){
    open_tau(tau);
    // cout << "read file ok\n";
    graph = new Graph();
    graph->build(*verilog, *spef, *lib[EARLY], *lib[LATE]);
    // cout << "graph build ok\n";
    graph->gen_test(output, type);
}

void Timer::run(const string& tau, const string& timing, const string& ops, const string&output_file){
    Logger::add_timestamp("start");

    omp_set_num_threads(8);
    /* important */
    omp_set_nested(4);
   #pragma omp parallel sections
    {
       #pragma omp section
        {
            open_tau(tau);
            Logger::add_timestamp("open_tau ok");
            graph = new Graph();
            graph->build(*verilog, *spef, *lib[EARLY], *lib[LATE]);

            Logger::add_timestamp("graph build ok");
            open_timing(timing);

            // init graph
            init_timer();
        }

       #pragma omp section
        {
            LOG(CERR)<< "tid: " << omp_get_thread_num() << " open ops\n";
            open_ops(ops);
        }
    }
    vector< vector<Path>* > ans;
    ans.resize((int)_from.size());
    int i;
    output.open(output_file);

    #pragma omp parallel for schedule(dynamic) private(i)
    for(i=0; i<(int)_from.size(); i++){
        ans[i] = graph->report_timing(*_from[i], *_through[i], *_to[i], _max_paths[i], _nworst[i]);
    }
    Logger::add_timestamp("report_timing ok");
    for(int i=0; i<(int)ans.size(); i++){
        for(auto p:*ans[i])
            p.output(output, graph);
    }
    output.close();
    Logger::add_timestamp("writing ok");

    #pragma omp parallel for schedule(dynamic) private(i)
    for(int i=0; i<(int)ans.size(); i++){
        delete ans[i];
    }
    Logger::add_timestamp("free ok");

    // for(i=0; i<(int)_from.size(); i++){
    //     auto a = graph->report_timing(*_from[i], *_through[i], *_to[i], _max_paths[i], _nworst[i]);
    //     for(auto p:*a)
    //         p.output(output, graph);
    // }
    // output.close();
    // Logger::add_timestamp("report_timing ok");
}

void Timer::clear_Timer(){
    if(verilog) delete verilog;
    if(spef) delete spef;
    if(lib[0]) delete lib[0];
    if(lib[1]) delete lib[1];
    if(graph) delete graph;
}

void Timer::init_timer(){
    graph->init_graph();
    // graph->print_graph();
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

   #pragma omp parallel sections
    {
       #pragma omp section
        {
            LOG(CERR) << "tid : " << omp_get_thread_num() << " open verilog\n";
            verilog->open( file_verilog );
        }
       // #pragma omp section
       //  {
       //      LOG(CERR) << "tid : " << omp_get_thread_num() << " open spef \n";
       //      spef->open( file_spef);
       //  }
       #pragma omp section
        {
            LOG(CERR) << "tid : " << omp_get_thread_num() << " open lib \n";
            lib[EARLY]->open( file_lib_early);
        }
       #pragma omp section
        {
            LOG(CERR) << "tid : " << omp_get_thread_num() << " open lib \n";
            lib[LATE]->open( file_lib_late );
        }
    }
}

void Timer::open_timing(const string& timing){
    File_Reader in;
    in.open(timing);

    string cmd, pin;
    float early[2], late[2];
    while((cmd = in.next_token())!=""){
        if(cmd=="clock"){
            pin = in.next_token();
            float period = stof(in.next_token());
            float low = stof(in.next_token());

            graph->set_clock(pin, period, low);
        }
        else if(cmd=="at"){
            pin = in.next_token();
            string tmp = in.next_token();
            if(tmp==":") LOG(ERROR) << "[Timer][open_timing] at pin is not primary " << pin << endl;
            early[RISE] = stof(tmp);
            early[FALL] = stof(in.next_token());
            late[RISE]  = stof(in.next_token());
            late[FALL]  = stof(in.next_token());

            graph->set_at(pin, early, late);
        }
        else if(cmd=="slew"){
            pin = in.next_token();
            string tmp = in.next_token();
            if(tmp==":") LOG(ERROR) << "[Timer][open_timing] slew pin is not primary " << pin << endl;
            early[RISE] = stof(tmp);
            early[FALL] = stof(in.next_token());
            late[RISE]  = stof(in.next_token());
            late[FALL]  = stof(in.next_token());

            graph->set_slew(pin, early, late);
        }
        else if(cmd=="rat"){
            pin = in.next_token();
            string tmp = in.next_token();
            if(tmp==":") LOG(ERROR) << "[Timer][open_timing] slew pin is not primary " << pin << endl;
            early[RISE] = stof(tmp);
            early[FALL] = stof(in.next_token());
            late[RISE]  = stof(in.next_token());
            late[FALL]  = stof(in.next_token());

            graph->set_rat(pin, early, late);
        }
        else if(cmd=="load"){
            pin = in.next_token();
            string tmp = in.next_token();
            if(tmp==":") LOG(ERROR) << "[Timer][open_timing] load pin is not primary out" << pin << endl;
            float cap = stof(tmp);

            graph->set_load(pin, cap);
        }
        else{
            LOG(ERROR) << "[Timer][open_timing] unknown keyword. " << cmd << endl;
        }
    }
}

void Timer::read_pin_name(File_Reader& in, string &name){
    name = in.next_token();
    string tmp = in.next_token();
    if(tmp==":" or tmp=="/"){
        name += ":";
        name += in.next_token();
    }
    else in.put_back(tmp);
}

void Timer::read_timing_assertion_option(File_Reader& in, string &name,
    Mode &mode, Transition_Type &transition, float &val){

        mode = EARLY;
        transition = RISE;
        name = "";
        string op;
        while(true){
            op = in.next_token();
            if(op=="-pin") read_pin_name(in, name);
            else if(op=="-fall")  transition = FALL;
            else if(op=="-rise")  transition = RISE;
            else if(op=="-late")  mode = LATE;
            else if(op=="-early") mode = EARLY;
            else if(isfloat(op)) val = stof(op);
            else break;
        }
        if(op.size()) in.put_back(op);
}

void Timer::open_ops(const string& ops){
    File_Reader in;
    in.open(ops);

    string cmd, op, name, inst_name, cell_type, net_name, pin_name;
    // Transition_Type transition;
    // Mode mode;
    // float val;
    // int num_path;

    do{
        cmd = in.next_token();
        if(cmd=="") break;
        /* tau 2018 */
        if(cmd=="report_timing"){
            string op, pin;
            vector<pair<Transition_Type, string>> *through, *from , *to;
            through = new vector<pair<Transition_Type,string>>();
            from = new vector<pair<Transition_Type,string>>();
            to = new vector<pair<Transition_Type,string>>();
            int max_pahts = 1, nworst = 1;
            do{
                op = in.next_token();
                if(op=="") break;
                if(op=="-from"){
                    read_pin_name(in, pin);
                    from->emplace_back(RISE, pin);
                    from->emplace_back(FALL, pin);
                }
                else if(op=="-rise_from"){
                    read_pin_name(in, pin);
                    from->emplace_back(RISE, pin);
                }
                else if(op=="-fall_from"){
                    read_pin_name(in, pin);
                    from->emplace_back(FALL, pin);
                }
                else if(op=="-to"){
                    read_pin_name(in, pin);
                    to->emplace_back(RISE, pin);
                    to->emplace_back(FALL, pin);
                }
                else if(op=="-rise_to"){
                    read_pin_name(in, pin);
                    to->emplace_back(RISE, pin);
                }
                else if(op=="-fall_to"){
                    read_pin_name(in, pin);
                    to->emplace_back(FALL, pin);
                }
                else if(op=="-through"){
                    read_pin_name(in, pin);
                    through->emplace_back(RISE, pin);
                    through->emplace_back(FALL, pin);
                }
                else if(op=="-rise_through"){
                    read_pin_name(in, pin);
                    through->emplace_back(RISE, pin);
                }
                else if(op=="-fall_through"){
                    read_pin_name(in, pin);
                    through->emplace_back(FALL, pin);
                }
                else if(op=="-max_paths") max_pahts = (int)stof(in.next_token());
                else if(op=="-nworst")   nworst = (int)stof(in.next_token());
                else if(op[0]=='-'){
                    LOG(CERR) << "[Timer][open_ops] unknown parameter in report_timing: " << op << endl;
                    output.close();
                    my_exit();
                }
                else break;
            }while(true);
            if(op.size()) in.put_back(op);
            _from.emplace_back(from);
            _to.emplace_back(to);
            _through.emplace_back(through);
            _nworst.emplace_back(nworst);
            _max_paths.emplace_back(max_pahts);
            // graph->report_timing(output, from, through, to, max_pahts, nworst);
        }
        // else if(cmd=="report_cppr_credit"){
        //     string pin1, pin2;
        //     Transition_Type type1, type2;
        //     mode = EARLY;
        //     type1 = type2 = RISE;
        //     do{
        //         op = in.next_token();
        //         if(op=="") break;
        //         else if(op=="-pin1") read_pin_name(in, pin1);
        //         else if(op=="-pin2") read_pin_name(in, pin2);
        //         else if(op=="-fall1") type1 = FALL;
        //         else if(op=="-fall2") type2 = FALL;
        //         else if(op=="-late") mode = LATE;
        //         else break;
        //     }while(true);
        //     if(op.size()) in.put_back(op);
        //     output << std::fixed << std::setprecision(OUTPUT_PRECISION) << graph->get_cppr_credit(pin1, pin2, type1, type2, mode) << endl;
        // }
        // /* Timing assertions */
        // else if(cmd=="set_at"){
        //     read_timing_assertion_option(in, name, mode, transition, val);
        //     graph->set_at(name, mode, transition, val);
        // }
        // else if(cmd=="set_slew"){
        //     read_timing_assertion_option(in, name, mode, transition, val);
        //     graph->set_slew(name, mode, transition, val);
        // }
        // else if(cmd=="set_rat"){
        //     read_timing_assertion_option(in, name, mode, transition, val);
        //     graph->set_rat(name, mode, transition, val);
        // }
        // else if(cmd=="set_load"){
        //     name = "";
        //     while(true){
        //         op = in.next_token();
        //         if(op=="-pin") read_pin_name(in, name);
        //         else if(isfloat(op)) val = stof(in.next_token());
        //         else break;
        //     }
        //     if(op.size()) in.put_back(op);
        // }
        // /* Timing queryies */
        // else if(cmd=="report_at"){
        //     read_timing_assertion_option(in, name, mode, transition, val);
        //     output << std::fixed << std::setprecision(OUTPUT_PRECISION) << graph->get_at(name, mode, transition) << endl;
        // }
        // else if(cmd=="report_rat"){
        //     read_timing_assertion_option(in, name, mode, transition, val);
        //     output << std::fixed << std::setprecision(OUTPUT_PRECISION) << graph->get_rat(name, mode, transition) << endl;
        // }
        // else if(cmd=="report_slack"){
        //     read_timing_assertion_option(in, name, mode, transition, val);
        //     output << std::fixed << std::setprecision(OUTPUT_PRECISION) << graph->get_slack(name, mode, transition) << endl;
        // }
        // else if(cmd=="report_slew"){
        //     read_timing_assertion_option(in, name, mode, transition, val);
        //     output << std::fixed << std::setprecision(OUTPUT_PRECISION) << graph->get_slew(name, mode, transition) << endl;
        // }
        // else if(cmd=="report_worst_paths"){
        //     name = "";
        //     num_path = 1; // default
        //     while(true){
        //         op = in.next_token();
        //         if(op=="-pin") read_pin_name(in, name);
        //         else if(op=="-numPaths") num_path = int(stof(in.next_token()));
        //         else break;
        //     }
        //     if(op.size()) in.put_back(op);
        //     graph->report_worst_paths(name, num_path);
        // }
        // /* circuit modification */
        // else if(cmd=="insert_gate"){
        //     inst_name = in.next_token();
        //     cell_type = in.next_token();
        //     graph->insert_gate(inst_name, cell_type);
        // }
        // else if(cmd=="repower_gate"){
        //     inst_name = in.next_token();
        //     cell_type = in.next_token();
        //     graph->repower_gate(inst_name, cell_type);
        // }
        // else if(cmd=="remove_gate"){
        //     inst_name = in.next_token();
        //     graph->remove_gate(inst_name);
        // }
        // /* net-level */
        // else if(cmd=="insert_net"){
        //     net_name = in.next_token();
        //     graph->insert_net(net_name);
        // }
        // else if(cmd=="read_spef"){
        //     name = in.next_token();
        //     graph->update_spef(name);
        // }
        // else if(cmd=="remove_net"){
        //     net_name = in.next_token();
        //     graph->remove_net(net_name);
        // }
        // /* pin-level*/
        // else if(cmd=="connect_pin"){
        //     read_pin_name(in, pin_name);
        //     net_name = in.next_token();
        //     graph->connect_pin(pin_name, net_name);
        // }
        // else if(cmd=="disconnect_pin"){
        //     read_pin_name(in, pin_name);
        //     graph->disconnect_pin(pin_name);
        // }
        else{
            LOG(ERROR) << "[Timer][open_ops] unknown keyword " << cmd << endl;
        }
    }while(true);
}
