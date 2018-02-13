#ifndef KTH_H
#define KTH_H

#include "bc_map.h"

class Kth {

public:

    struct Edge{
        int from, to;
        float delay, at_delay, delta, at_delta;
        Edge(){}
        Edge(int f,int t,float d):from(f), to(t), delay(d){}
    };

	struct Prefix_node {
		Prefix_node *parent;        // Parent in prefix path tree
		Edge *eptr;                 // Last sidetrack in the path
		float delta_dist, delta_at; // Cumulative delta
		static bool compare(Prefix_node *n1, Prefix_node *n2);
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

    vector<float> dist;         // Shortest distance to dest
    vector<float> at;           // For tie breaking, not real at, it means "how bad the at is", smaller value is worse
    vector<int> successor;      // Keep the shortest paht tree rooted to dest
    int source_kth, dest_kth;   // Source and dest;
    
    void build_single_dest_tree(int dest);  // Build a single destination tree rooted at "dest" (destination)

};
#endif /* end KTH_H */
