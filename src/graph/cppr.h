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
    void build_tree();
    float cppr_credit(Mode mode, int u,Transition_Type type1,int v, Transition_Type type2);

private:
    void dfs_build(int root,int dep,int neg);
    void build_sparse();
    void add_node(int node_id, int dep, int neg);      // add node to clock tree
    int get_lca(int u,int v);
    int root;                // primary clock input

    int num;                 // total nodes

    vector<int> to_node_id;      // tree node id to graph id
    unordered_map<int, int> to_tree_id;  // graph node id to tree node id
    vector<int> negation;        // negation times
    vector<int> level,who;       // dfs depth, who's depth
    vector<int> in;              // in dfs time
    vector<vector<int>> T;       // sparse table

    Graph* graph;
};
#endif /* CPPR_H */
