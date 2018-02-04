#ifndef RC_TREE_H
#define RC_TREE_H

#include "header.h"
#include "debug.h"
#include "verilog.h"
#include "spef.h"
#include "cell_lib.h"

class RCTree{
    struct RCNode{
        float downstream;
        float cap;
        float delay;
        float impluse;                  // delay*cap
        float beta;
        RCNode(){}
        RCNode(float _downstream, float _cap, float _delay):
        downstream(_downstream), cap(_cap), delay(_delay){
            beta = 0;
            impluse = 0;
        }
    };

    struct RCEdge{
        int to;
        float res;
        RCEdge(){}
        RCEdge(int _to,float _res):to(_to), res(_res){}
    };

public:

    RCTree(SpefNet* _spefnet, Verilog* _verilog, CellLib* _cell_lib);
    float get_slew(const string& name, float input_slew);
    float get_delay(const string& name);
    float get_downstream(const string& name);

    void print();

private:
    void build_tree();
    void cal();
    void cal_downstream(int x, int pa);
    void cal_delay(int x, int pa);          // cal delay and impluse
    // void cal_impluse(int x, int pa);
    void cal_beta(int x, int pa);

    int root, num_nodes;

    vector<RCEdge>* G;
    vector<RCNode> nodes;

    SpefNet *spefnet;
    Verilog *verilog;
    CellLib *cell_lib;
};

#endif /* end RC_TREE_H */
