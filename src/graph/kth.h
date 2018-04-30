#ifndef KTH_H
#define KTH_H

#include "bc_map.h"
#include "cppr.h"
#include "graph.h"

class Graph;
class CPPR;
class BC_map;
struct Edge;

enum {NOT_VISITED = -1, VISITED = -2};

struct Path {
    // Explicit representation of a path
    float dist;
    vector<int> path;
    vector<float> delay;

    void print();
    void output(ostream &fout, Graph *graph) const;
    void print_name(ostream &fout, const string &name) const;
};

class Kth {

public:

    struct Sidetrack_Edge {
        /* For compile purpose. */
        int from, to;
        float delay;
    };

	struct Prefix_node {
		/* Implicit representation of a path */
		Prefix_node *parent; // Parent in prefix path tree.
		Sidetrack_Edge last; // Last sidetrack edge.
		float delta;         // Cumulative delta.

		struct Compare {
			bool operator()(const Prefix_node *n1, const Prefix_node *n2);
		};
		Prefix_node(Kth *father, Prefix_node *p, int from, int to, float delay);
		Prefix_node();
	};

    // Kth algorithm interface
    void clear();
    Kth(BC_map *_map, CPPR *_cppr, Graph *_graph);
    void KSP_to_destination(int dest, vector<Path> &result_container);
    void KSP_from_source(int src, vector<Path> &result_container);

private:
    BC_map *bc_map;
    CPPR *cppr;
    Graph *graph;

    /* Lookup table that maps bc map id to [0, N]. */
    unordered_map<int, int> LUT;

    /* For SDSP tree */
    int dest;
    vector<float> dist;
    vector<int> successor;
    vector<int> topo_order;

    /* pseudo source and edges connect to them. */
    int pseudo_src;
    vector<Edge> pseudo_edge;

    /* For KSP algorithm */
    std::priority_queue<Prefix_node*, vector<Prefix_node*>, Prefix_node::Compare> pq;
    vector<Prefix_node*> trash_can;
    int set_id(int v);
    void KSP(int k, vector<Path> &container, const vector<vector<Edge>> &adj, const vector<vector<Edge>> &radj);
    void get_explicit_path_helper(Path *exp_path, const Prefix_node *imp_path, int dest);
    void get_explicit_path(Path *exp_path, const Prefix_node *imp_path);
    bool build_SDSP_tree(int dest, const vector<vector<Edge>> &radj);
    void get_topological_order(int v, const vector<vector<Edge>> &radj);
    void extend(Prefix_node *path, const vector<vector<Edge>> &adj);

    inline float get_delta(const Sidetrack_Edge &edg) {
        return edg.delay + dist[ LUT[edg.to] ] - dist[ LUT[edg.from] ];
    }
};
#endif /* end KTH_H */
