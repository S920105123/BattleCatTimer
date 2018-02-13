#ifndef KTH_H
#define KTH_H

#include "bc_map.h"
#include "cppr.h"

class Kth {

public:

    struct Edge{
        int from, to;
        float delay;
        float clock_delay;
        Edge(){}
        Edge(int f,int t,float d):from(f), to(t), delay(d), clock_delay(d){}
    };

    Kth(BC_map *_map, CPPR *_cppr);

    void build_from_src(const vector<pair<Transition_Type,int>>&, int src, bool only_src);
    void build_from_dest(const vector<pair<Transition_Type,int>>&, int dest, bool only_dest);

    int get_kth_id(int map_id);

private:
    void mark_through(const vector<pair<Transition_Type,int>>&);
    bool forward_build(int now, int next_object);
    int add_node(int bc_node_id);
    int add_edge(int from, int to, float delay);                    // from , to in bc map
    int add_edge(int from, int to, float delay, float clock_delay); // from , to in bc map

    BC_map* map;
    CPPR* cppr;
    vector<int> to_bc_id;       // kth node id to bc node id
    vector<int> to_kth_id;      // bc node id to kth node id
    vector<bool> is_good;       // if the node is good then it can go through all obect under it's level
    vector<bool> has_kth_id;    // if bc map id has mapped to kth id
    vector<bool> vis;           // for dfs
    vector<vector<Edge>> G;     // kth graph
    int num_node;

    vector<bool> mark;          // mark node in graph
    vector<int> object;         // the search order to build good map for kth
    vector<int> all_leave;      //

    vector<int> dist;           // shortest distance to dest
    vector<int> successor;      // keep the shortest path tree rooted to dest
    int source_kth, dest_kth;   // source and dest;
};
#endif /* end KTH_H */
