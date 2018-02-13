#ifndef KTH_H
#define KTH_H

#include "bc_map.h"
#include "cppr.h"

class Kth {

public:

    struct Edge{
        int from, to;
<<<<<<< HEAD
        float delay;
        float clock_delay;
=======
        float delay, at_delay, delta, at_delta;
>>>>>>> 78ba3bf14bc849d60379d19d80bf72810007448c
        Edge(){}
        Edge(int f,int t,float d):from(f), to(t), delay(d), clock_delay(d){}
    };

<<<<<<< HEAD
    Kth(BC_map *_map, CPPR *_cppr);
=======
	struct Prefix_node {
		Prefix_node *parent;        // Parent in prefix path tree
		Edge *eptr;                 // Last sidetrack in the path
		float delta_dist, delta_at; // Cumulative delta
		static bool compare(Prefix_node *n1, Prefix_node *n2);
	};
>>>>>>> 78ba3bf14bc849d60379d19d80bf72810007448c

    void build_from_src(const vector<pair<Transition_Type,int>>&, int src, bool only_src);
    void build_from_dest(const vector<pair<Transition_Type,int>>&, int dest, bool only_dest);

<<<<<<< HEAD
    int get_kth_id(int map_id);
=======
private:
	
    void mark_through(BC_map* map, const vector<pair<Transition_Type,int>>&);
>>>>>>> 78ba3bf14bc849d60379d19d80bf72810007448c

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

<<<<<<< HEAD
    vector<int> dist;           // shortest distance to dest
    vector<int> successor;      // keep the shortest path tree rooted to dest
    int source_kth, dest_kth;   // source and dest;
=======
    vector<float> dist;         // Shortest distance to dest
    vector<float> at;           // For tie breaking, not real at, it means "how bad the at is", smaller value is worse
    vector<int> successor;      // Keep the shortest paht tree rooted to dest
    int source_kth, dest_kth;   // Source and dest;
    
    void build_single_dest_tree(int dest);  // Build a single destination tree rooted at "dest" (destination)

>>>>>>> 78ba3bf14bc849d60379d19d80bf72810007448c
};
#endif /* end KTH_H */
