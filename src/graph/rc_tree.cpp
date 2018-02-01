
#include "rc_tree.h"

RcTree::RcTree(SpefNet* _spefnet, Verilog* _verilog, CellLib* _cell_lib){
    spefnet  = _spefnet;
    verilog  = _verilog;
    cell_lib = _cell_lib;

    build_tree();
    cal_downstream(root, -1);
    cal_delay(root, -1);
}

void RcTree::build_tree(){

    num_nodes = spefnet->get_num_pins();
    /* build edge */
    G = new vector<RcEdge>[num_nodes+5];
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

        /* judge root and add tap capaitance from liberty*/
    root = -1;
    for(auto& it:spefnet->get_total_conn()){
        string name, type, dir;
        tie(name, type, dir) = it;
        if(dir=="O"){
            if(root!=-1) LOG(ERROR) << "[Rc_Tree][build_tree] net = "
                            << spefnet->get_name() << " can't judge root" << endl;
            root = spefnet->get_pin_id(name);
        }
        if(type=="P") continue;

        string inst, pin_name;
        for(int i=0, j = 0; i<name.size(); i++){
            if(name[i]==':') j = 1;
            else if(j==0) inst += name[i];
            else pin_name += name[i];
        }
        if(inst.size()==0 or pin_name.size()==0)
            LOG(ERROR) << "[Rc_Tree][build_grapgh] net: " << spefnet->get_name()
                << " : " << name <<" inst is empty or pin_name is empty\n";

        float tap_cap = cell_lib->get_pin_capacitance(verilog->get_cell_type(inst), pin_name);
        nodes[ spefnet->get_pin_id(name) ].cap += tap_cap;
    }

    if(root==-1) LOG(ERROR) << "[Rc_Tree][build_tree] net = "
                    << spefnet->get_name() << " can't judge root" << endl;
}

void RcTree::cal_downstream(int x, int pa){
    nodes[x].downstream = nodes[x].cap;
    for(auto e:G[root]) if(e.to != pa){
        cal_downstream(e.to, x);
        nodes[x].downstream += nodes[e.to].downstream;
    }
}

void RcTree::cal_delay(int x,int pa){
    for(auto e:G[root]) if(e.to != pa){
        cal_delay(e.to, x);
        nodes[e.to].delay += nodes[x].delay + nodes[e.to].downstream*e.res;
    }
}

#ifdef TEST_RC_TREE

int main()
{
    Spef* spef = new spef();
    Verilog* verilog = new verilog();
    CellLib* lib = new CellLib();
    spef->open("simple/simple.spef");
    verilog->open("simple/simple.v");
    lib->open("simple/simple_Early.lib");

    string name;
    while(cin>>name){
        SpefNet* spefnet = spef->get_spefnet_ptr(name);
        if(spefnet==NULL){
            cout << "no such spefnet: " << name << endl;
            continue;
        }

        RcTree rc(spefnet, verilog, lib);
    }
    Logger::create()~Logger();
}

#endif /* end TEST_RC_TREE */
