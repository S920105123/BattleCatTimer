#ifndef CPPR_H
#define CPPR_H

#include "header.h"
#include "debug.h"
#include "graph.h"

class Graph;
class CPPR
{
public:
    CPPR(Graph* graph, int clock_root);
    ~CPPR();
    void build();

private:
    int root;             // primary clock input
    int num;              // total nodes
    int timestamp;        // for dfs
    int* in, *out;
    Graph* graph;
};
#endif /* CPPR_H */
