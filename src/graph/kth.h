#ifndef KTH_H
#define KTH_H

#include "bc_map.h"
#include "cppr.h"

class CPPR;
class BC_map;
class Kth {

public:

    struct Edge{
        int id, from, to;
        float delay, at_delay, delta, at_delta;
        Edge(){}
        Edge(int id, int f,int t,float d);
    };

	struct Prefix_node {
		// Implicit representation of a path
		Prefix_node *parent;   // Parent in prefix path tree
		Edge *eptr;            // Last sidetrack in the path
		float delta, at_delta; // Cumulative delta

		struct Compare {
			bool operator()(const Prefix_node *n1, const Prefix_node *n2);
		};
		Prefix_node(Prefix_node *p, Edge *e);
		Prefix_node();
	};

	struct Path {
		// Explicit representation of a path
		float dist;
		vector<int> path;
		vector<float> delay;

		void print();
	};

	Kth(BC_map *_map, CPPR *_cppr);
    void build_from_src(const vector<pair<Transition_Type,int>>&, int src, bool only_src);
    void build_from_dest(const vector<pair<Transition_Type,int>>&, int dest, bool only_dest);
    int get_kth_id(int map_id);
    void add_edge(int from, int to, float delay);                    // from , to in bc map
    void add_edge(int from, int to, float delay, float clock_delay); // from , to in bc map

    // Kth algorithm implementation
    void k_shortest_path(int k, vector<Path> &container);
    void get_explicit_path(Path *exp_path, const Prefix_node *imp_path);
    void print();
    void print_path(const Path& path);
    string get_node_name(int kth_id);

    Kth(){} /* Just for unit test */
	vector<vector<Edge>>& getG(); /* Just for unit test */
	void set_st(int s, int t);

private:
    void mark_through(const vector<pair<Transition_Type,int>>&);
    bool forward_build(int now, int next_object);
    bool backward_build(int now, int next_object);
    int add_node(int bc_node_id);

    BC_map* map;
    CPPR* cppr;
    vector<int> to_bc_id;       // kth node id to bc node id
    vector<int> to_kth_id;      // bc node id to kth node id
    vector<bool> is_good;       // if the node is good then it can go through all obect under it's level
    vector<bool> has_kth_id;    // if bc map id has mapped to kth id
    vector<bool> vis;           // for dfs
    vector<vector<Edge>> G;     // kth graph
    int num_node, num_edge;

    vector<bool> mark;          // mark node in graph
    vector<int> object;         // the search order to build good map for kth
    vector<int> all_leave;      //

    vector<float> dist;         // Shortest distance to dest
    vector<float> at_dist;      // For tie breaking, not real at, it means "how bad the at is", smaller value is worse
    vector<int> successor;      // Keep the shortest paht tree rooted to dest
    vector<int> use_edge;       // Keep edge id
    int source_kth, dest_kth;   // Source and dest;

    std::priority_queue<Prefix_node*, vector<Prefix_node*>, Prefix_node::Compare> pq;
    vector<Prefix_node*> trash_can; // Store used prefix node

    bool build_single_dest_tree(int dest);  // Build a single destination tree rooted at "dest" (destination)
    void get_explicit_path_helper(Path *exp_path, const Prefix_node *imp_path, int dest);
    void extend(Prefix_node *path);
    void single_dest_dfs(int v);
};
#endif /* end KTH_H */
