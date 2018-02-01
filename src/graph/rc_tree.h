#ifndef RC_TREE_H
#define RC_TREE_H

#include "header.h"
#include "debug.h"
#include "verilog.h"
#include "spef.h"
#include "cell_lib.h"

class RcTree{

public:
    struct RcNode{
        float downstream;
        float delay;
        float cap;
    };

    struct RcEdge{
        int to;
        float res;
    };

    RcTree(SpefNet* _spefnet, Verilog* _verilog, CellLib* _cell_lib);

private:
    void build_tree();
    void cal_downstream(int x, int pa);
    void cal_delay(int x, int pa);
    void cal_slew(int x, int pa);

    int root, num_nodes;

    vector<RcEdge>* G;
    vector<RcNode> nodes;

    SpefNet *spefnet;
    Verilog *verilog;
    CellLib *cell_lib;
};

#endif /* end RC_TREE_H */
