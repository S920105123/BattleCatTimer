#ifndef KTH_H
#define KTH_H

#include "bc_map.h"
#include "cppr.h"

class CPPR;
class BC_map;
class Kth {

public:

    struct Edge{
        int from, to;
        float delay, at_delay, delta, at_delta;
        Edge(){}
        Edge(int f,int t,float d):from(f), to(t), delay(d), at_delay(d){}
    };

	struct Prefix_node {
		Prefix_node *parent;        // Parent in prefix path tree
		Edge *eptr;                 // Last sidetrack in the path
		float delta_dist, delta_at; // Cumulative delta
		static bool compare(Prefix_node *n1, Prefix_node *n2);
	};

    Kth(BC_map *_map, CPPR *_cppr);
    void build_from_src(const vector<pair<Transition_Type,int>>&, int src, bool only_src);
    void build_from_dest(const vector<pair<Transition_Type,int>>&, int dest, bool only_dest);

    int get_kth_id(int map_id);
    void print();
    string get_node_name(int kth_id);

private:
    void mark_through(const vector<pair<Transition_Type,int>>&);
    bool forward_build(int now, int next_object);
    bool backward_build(int now, int next_object);
    int add_node(int bc_node_id);
    void add_edge(int from, int to, float delay);                    // from , to in bc map
    void add_edge(int from, int to, float delay, float clock_delay); // from , to in bc map

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

    vector<float> dist;         // Shortest distance to dest
    vector<float> at;           // For tie breaking, not real at, it means "how bad the at is", smaller value is worse
    vector<int> successor;      // Keep the shortest paht tree rooted to dest
    int source_kth, dest_kth;   // Source and dest;

    void build_single_dest_tree(int dest);  // Build a single destination tree rooted at "dest" (destination)
    void single_dest_dfs(int v);
};
#endif /* end KTH_H */
