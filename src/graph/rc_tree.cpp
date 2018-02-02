
#include "rc_tree.h"

RCTree::RCTree(SpefNet* _spefnet, Verilog* _verilog, CellLib* _cell_lib){
    spefnet  = _spefnet;
    verilog  = _verilog;
    cell_lib = _cell_lib;

    build_tree();
    if(root==-1) return;
    cal();
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
        nodes[it.second].cap = spefnet->get_pin_cap(it.second);
        nodes[it.second].downstream = 0;
        nodes[it.second].delay = 0;
    }

        /* get root and add tap capaitance from liberty*/
    root = -1;
    for(auto& it:spefnet->get_total_conn()){
        string name, type, dir;
        tie(name, type, dir) = it;
        if( (dir=="O" or type=="P" )&& (!(dir=="O" and type=="P"))){
            if(root!=-1) LOG(ERROR) << "[RC_Tree][build_tree] net = "
                            << spefnet->get_name() << " can't get root" << endl;
            root = spefnet->get_pin_id(name);
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

        float tap_cap = cell_lib->get_pin_capacitance(verilog->get_cell_type(inst), pin_name);
        nodes[ spefnet->get_pin_id(name) ].cap += tap_cap;
    }

    if(root==-1) LOG(ERROR) << "[RC_Tree][build_tree] net = "
                    << spefnet->get_name() << " can't get root" << endl;

}

void RCTree::cal(){
    ASSERT(root != -1);
    cal_downstream(root, -1);
    cal_delay(root, -1);
    // cal_impluse(root, -1);
    cal_beta(root, -1);
}

void RCTree::cal_downstream(int x, int pa){
    nodes[x].downstream = nodes[x].cap;
    for(auto e:G[x]) if(e.to != pa){
        cal_downstream(e.to, x);
        nodes[x].downstream += nodes[e.to].downstream;
    }
}

void RCTree::cal_delay(int x,int pa){
    nodes[x].impluse = nodes[x].cap*nodes[x].delay;
    for(auto e:G[x]) if(e.to != pa){
        nodes[e.to].delay += nodes[x].delay + nodes[e.to].downstream*e.res;
        cal_delay(e.to, x);
        nodes[x].impluse += nodes[e.to].impluse;
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
        nodes[e.to].beta += nodes[x].beta + nodes[e.to].impluse*e.res;
        cal_beta(e.to, x);
    }
}

float RCTree::get_slew(float input_slew,string name){
    int id = spefnet->get_pin_id(name);
    float tmp = std::sqrt(2*nodes[id].beta - nodes[id].delay*nodes[id].delay);
    return std::sqrt( tmp*tmp + input_slew*input_slew);
}

float RCTree::get_delay(string name){
    int id = spefnet->get_pin_id(name);
    return nodes[id].delay;
}

void RCTree::print(){
    cout << "net: " << spefnet->get_name() << endl;
    cout << "   root = " << spefnet->get_pin_name(root) << endl;
    for(int i=0; i<num_nodes; i++){
        cout << "   " << spefnet->get_pin_name(i) << " delay = " << nodes[i].delay
            << " downstream = " << nodes[i].downstream
            << " beta = " << nodes[i].beta
            << " slew(input_slew=30) = " << get_slew(30, spefnet->get_pin_name(i))         // use simple:inp2 input slew
            << endl;
    }
}

#ifdef TEST_RCTREE

int main()
{
    string testcase[5] ={ "s1196", "systemcdes", "usb_funct", "vga_lcd"};

    for(int i=0; i<4; i++){
        Spef* spef = new Spef();
        Verilog* verilog = new Verilog();
        CellLib* lib = new CellLib();

        string tmp = testcase[i] + "/" + testcase[i];
        spef->open("testcase_v1.2/" + tmp  + ".spef");
        verilog->parse("testcase_v1.2/" + tmp + ".v");
        lib->open("testcase_v1.2/" + tmp + "_Early.lib");

        cout << "open " << testcase[i] << "ok\n";
        cout << "try to buil all rc tree...\n";
        for(auto& it:spef->get_total_nets()){
            RCTree rc(it.second, verilog, lib);
        }
        cout << "buil ok!\n";

    }
    Logger::create()->~Logger();

    // spef->open("simple/simple.spef");
    // verilog->parse("simple/simple.v");
    // lib->open("simple/simple_Early.lib");
    // string name;
    // cout << "enter net name or exit: ";
    // while(cin>>name){
    //     SpefNet* spefnet = spef->get_spefnet_ptr(name);
    //     if(name=="exit") break;
    //     if(spefnet==NULL){
    //         cout << "no such spefnet: " << name << endl;
    //         continue;
    //     }
    //     spef->print_net(name);
    //     RCTree rc(spefnet, verilog, lib);
    //     // rc.print();
    // }
}

#endif /* end TEST_RC_TREE */
