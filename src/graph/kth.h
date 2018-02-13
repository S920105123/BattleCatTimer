#ifndef KTH_H
#define KTH_H

#include "bc_map.h"

class Kth {

public:

    struct Edge{
        int from, to;
        float delay;
        Edge(){}
        Edge(int f,int t,float d):from(f), to(t), delay(d){}
    };


    void build_from_src(BC_map* map, const vector<pair<Transition_Type,int>>&, int src);
    void build_from_dest(BC_map* map, const vector<pair<Transition_Type,int>>&, int dest);

private:
    void mark_through(BC_map* map, const vector<pair<Transition_Type,int>>&);

    vector<int> to_bc_id;       // kth node id to bc node id
    vector<int> to_tk_id;       // bc node id tot kth node id
    vector<vector<Edge>> G;     // kth graph

    vector<bool> mark;          // mark node in graph
    vector<int> object;         // the search order to build good map for kth

    vector<int> dist;           // shortest distance to dest
    vector<int> successor;      // keep the shortest paht tree rooted to dest
    int source, dest;           // source and dest;
};
#endif /* end KTH_H */
