
#include "rc_tree.h"

RCTree::RCTree(SpefNet* _spefnet, Verilog* _verilog, CellLib* _cell_lib[2]){
    spefnet  = _spefnet;
    verilog  = _verilog;
    cell_lib[EARLY] = _cell_lib[EARLY];
    cell_lib[LATE] = _cell_lib[LATE];

}

void RCTree::build_tree(){

    num_nodes = spefnet->get_num_pins();
    /* build edge */
    G = new vector<RCEdge>[num_nodes+5];
    for(auto it:spefnet->get_pin_res()){
        int from, to;
        float res;
        tie(from, to, res) = it;
        G[from].emplace_back(to, res);
        G[to].emplace_back(from, res);
    }

    /* build nodes */
    nodes.resize( num_nodes + 5);
    for(auto& it:spefnet->get_total_pins()){ // <string, int>
        nodes[it.second].cap[EARLY] = nodes[it.second].cap[LATE] = spefnet->get_pin_cap(it.second);
        nodes[it.second].downstream[EARLY] = nodes[it.second].downstream[LATE] = 0;
        nodes[it.second].delay[EARLY] = nodes[it.second].delay[LATE] = 0;
    }

        /* get root and add tap capaitance from liberty*/
    root = -1;
    for(auto& it:spefnet->get_total_conn()){
        string name, type, dir;
        tie(name, type, dir) = it;
        if( (dir=="O" or type=="P" )&& (!(dir=="O" and type=="P"))){
            if(root!=-1) LOG(ERROR) << "[RC_Tree][build_tree] net = "
                            << spefnet->get_name() << " can't get root" << '\n';
            root = spefnet->get_pin_id(name);
            // don't add cap of root pin
            continue;
        }
        if(type=="P") continue;

        string inst, pin_name;
        for(size_t i=0, j = 0; i<name.size(); i++){
            if(name[i]==':') j = 1;
            else if(j==0) inst += name[i];
            else pin_name += name[i];
        }
        if(inst.size()==0 or pin_name.size()==0)
            LOG(ERROR) << "[RC_Tree][build_grapgh] net: " << spefnet->get_name()
                << " : " << name <<" inst is empty or pin_name is empty\n";

        float tap_cap_early = cell_lib[EARLY]->get_pin_capacitance(verilog->get_cell_type(inst), pin_name);
        nodes[ spefnet->get_pin_id(name) ].cap[EARLY] += tap_cap_early;

        float tap_cap_late = cell_lib[LATE]->get_pin_capacitance(verilog->get_cell_type(inst), pin_name);
        nodes[ spefnet->get_pin_id(name) ].cap[LATE] += tap_cap_late;
        if(tap_cap_late != tap_cap_early)
            LOG(WARNING) << "[RC_Tree][build_grapgh] net: " << spefnet->get_name() << " early pin cap != late pin cap" << name << '\n';
    }

    if(root==-1) LOG(ERROR) << "[RC_Tree][build_tree] net = "
                    << spefnet->get_name() << " can't get root" << '\n';

}

void RCTree::cal(){
    ASSERT(root != -1);
    for(size_t i=0; i<nodes.size(); i++){
        nodes[i].delay[EARLY] = nodes[i].delay[LATE] = 0;
        nodes[i].downstream[EARLY] = nodes[i].downstream[LATE] = 0;
        nodes[i].impluse[EARLY] = nodes[i].impluse[LATE] = 0;
    }
    cal_downstream(root, -1);
    cal_delay(root, -1);
    // cal_impluse(root, -1);
    cal_beta(root, -1);
}

void RCTree::cal_downstream(int x, int pa){
    nodes[x].downstream[EARLY] = nodes[x].cap[EARLY];
    nodes[x].downstream[LATE]  = nodes[x].cap[LATE];
    for(auto e:G[x]) if(e.to != pa){
        cal_downstream(e.to, x);
        nodes[x].downstream[EARLY] += nodes[e.to].downstream[EARLY];
        nodes[x].downstream[LATE]  += nodes[e.to].downstream[LATE];
    }
}

void RCTree::cal_delay(int x,int pa){
    nodes[x].impluse[EARLY] = nodes[x].cap[EARLY]*nodes[x].delay[EARLY];
    nodes[x].impluse[LATE]  = nodes[x].cap[LATE]*nodes[x].delay[LATE];
    for(auto e:G[x]) if(e.to != pa){
        nodes[e.to].delay[EARLY] += nodes[x].delay[EARLY] + nodes[e.to].downstream[EARLY]*e.res;
        nodes[e.to].delay[LATE] += nodes[x].delay[LATE] + nodes[e.to].downstream[LATE]*e.res;
        cal_delay(e.to, x);
        nodes[x].impluse[EARLY] += nodes[e.to].impluse[EARLY];
        nodes[x].impluse[LATE] += nodes[e.to].impluse[LATE];
    }
}

// void RCTree::cal_impluse(int x, int pa){
//     nodes[x].impluse = nodes[x].cap*nodes[x].delay;
//     for(auto e:G[x]) if(e.to != pa){
//         cal_impluse(e.to, x);
//         nodes[x].impluse += nodes[e.to].impluse;
//     }
// }

void RCTree::cal_beta(int x,int pa){
    for(auto e:G[x]) if(e.to != pa){
        nodes[e.to].beta[EARLY] += nodes[x].beta[EARLY] + nodes[e.to].impluse[EARLY]*e.res;
        nodes[e.to].beta[LATE] += nodes[x].beta[LATE] + nodes[e.to].impluse[LATE]*e.res;
        cal_beta(e.to, x);
    }
}

float RCTree::get_slew(Mode mode, const string& name,float input_slew){
    // return 0;
    int id = spefnet->get_pin_id(name);
    float tmp = std::sqrt(2*nodes[id].beta[mode] - nodes[id].delay[mode]*nodes[id].delay[mode]);
    return std::sqrt( tmp*tmp + input_slew*input_slew);
}

float RCTree::get_delay(Mode mode, const string& name){
    // return 0;
    int id = spefnet->get_pin_id(name);
    return nodes[id].delay[mode];
}

float RCTree::get_downstream(Mode mode, const string& name){
    // return 0;
    int id = spefnet->get_pin_id(name);
    return nodes[id].downstream[mode];
}

void RCTree::add_pin_cap(const string& name, float cap){
    int id = spefnet->get_pin_id(name);
    nodes[id].cap[EARLY] += cap;
    nodes[id].cap[LATE]  += cap;
}

void RCTree::print(){
    cout << "net: " << spefnet->get_name() << '\n';
    cout << "   root = " << spefnet->get_pin_name(root) << '\n';
    for(int i=0; i<num_nodes; i++){
            cout << "   " << spefnet->get_pin_name(i)  << '\n';
            cout << "   delal: EARLY = " << nodes[i].delay[EARLY] << ", LATE = " << nodes[i].delay[LATE] << '\n';
            cout << "   downstream: EARLY = " << nodes[i].downstream[EARLY] << ", LATE = " << nodes[i].downstream[LATE] << '\n';
            cout << "   beta: EARLY = " << nodes[i].beta[EARLY] << ", LATE = " << nodes[i].beta[LATE] << '\n';
            // use simple:inp2 input slew
            cout << "   slew(input_slew=30) EARLY = " << get_slew(EARLY, spefnet->get_pin_name(i), 30)  << '\n';
            cout << "   slew(input_slew=30) LATE = " << get_slew(LATE, spefnet->get_pin_name(i), 30)  << '\n';
            cout << '\n';
    }
}

#ifdef TEST_RCTREE

int main()
{

    Spef* spef = new Spef();
    Verilog* verilog = new Verilog();
    CellLib* lib[2];
    lib[EARLY] = new CellLib(EARLY);
    lib[LATE] = new CellLib(LATE);

    // string testcase[5] ={ "s1196", "systemcdes", "usb_funct", "vga_lcd"};
    // for(int i=0; i<4; i++){
    //     spef = new Spef();
    //     verilog = new Verilog();
    //     lib[EARLY] = new CellLib();
    //     lib[LATE] = new CellLib();
    //     string tmp = testcase[i] + "/" + testcase[i];
    //     spef->open("testcase_v1.2/" + tmp  + ".spef");
    //     verilog->open("testcase_v1.2/" + tmp + ".v");
    //     lib[EARLY]->open("testcase_v1.2/" + tmp + "_Early.lib");
    //     lib[LATE]->open("testcase_v1.2/" + tmp + "_Late.lib");
    //
    //     cout << "open " << testcase[i] << "ok\n";
    //     cout << "try to buil all rc tree...\n";
    //     for(auto& it:spef->get_total_nets()){
    //         RCTree rc(it.second, verilog, lib);
    //     }
    //     cout << "buil ok!\n";
    //
    // }

    string testcase ;
    cout << "enter testcase: ";
    cin >> testcase;
    spef->open("testcase_v1.2/" + testcase + "/" + testcase + ".spef");
    verilog->open("testcase_v1.2/" + testcase + "/" + testcase + ".v");
    lib[EARLY]->open("testcase_v1.2/" + testcase + "/" + testcase + "_Early.lib");
    lib[LATE]->open("testcase_v1.2/" + testcase + "/" + testcase + "_Late.lib");
    string name;
    cout << "enter net name or exit: ";
    while(cin>>name){
        if(name=="exit") break;
        SpefNet* spefnet = spef->get_spefnet_ptr(name);
        if(spefnet==NULL){
            cout << "no such spefnet: " << name << '\n';
            continue;
        }
        spef->print_net(name);
        RCTree rc(spefnet, verilog, lib);
        rc.build_tree();
        rc.cal();
        rc.print();
    }
    Logger::create()->~Logger();
}

#endif /* end TEST_RC_TREE */
