#ifndef RC_TREE_H
#define RC_TREE_H

#include "header.h"
#include "debug.h"
#include "verilog.h"
#include "spef.h"
#include "cell_lib.h"

class RCTree{
    struct RCNode{
        float downstream[2];
        float cap[2];
        float delay[2];
        float impluse[2];                  // delay*cap
        float beta[2];
        RCNode(){
            beta[EARLY] = beta[LATE] = 0;
            impluse[EARLY] = impluse[LATE] = 0;
        }
    };

    struct RCEdge{
        int to;
        float res;
        RCEdge(){}
        RCEdge(int _to,float _res):to(_to), res(_res){}
    };

public:

    RCTree(SpefNet* _spefnet, Verilog* _verilog, CellLib* _cell_lib[2]);
    float get_slew(Mode mode, const string& name, float input_slew);
    float get_delay(Mode mode, const string& name);
    float get_downstream(Mode mode, const string& name);

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
    CellLib *cell_lib[2];
};

#endif /* end RC_TREE_H */
