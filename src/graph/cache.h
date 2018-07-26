#ifndef CACHE_H
#define CACHE_H

#include "header.h"
#include "bc_map.h"
#include "cache_node.h"
#include "bct_bitset.h"
#include "bct_writer.h"

class CacheNode;
class Cache_Edge;
class Graph;
class BC_map;

class Path {
public:
    float dist;
    vector<int> path;
    vector<float> delay;

    Path() {};
    void output(ostream &fout, Graph *graph);
	void fast_output(Writer&, Graph* graph);
};

// merge CacheNode and do kth
class Cache {

public:
	struct PrefixNode {
		PrefixNode* parent;
		int from, to; 
		float delay, delta; 

		PrefixNode(PrefixNode* par, int from, int to, float delay, float delta) {
			this->parent = par;
			this->from = from;
			this->to = to;
			this->delay = delay;
			this->delta = delta;
			if(par != nullptr) this->delta += par->delta;
		}

		struct Compare {
			bool operator()(PrefixNode* pfx1, PrefixNode* pfx2) {
				return pfx1->delta > pfx2->delta;
			}
		};
	};

	struct Edge{
		int from, to;
		float delay;
		Edge(int f,int t, float d):from(f), to(t), delay(d){

		}
	};

	Cache(BC_map*);

	void clear();
	void set_disable(const vector<int>&);
	void add_cache_node(CacheNode* node, int ,int );             // the level of cacheNode is increased 
	void kth(vector<Path*>& ans, int k);
	void update_cacheNode();                          // build all cacheNode
	void output_shortest_path();
	bool is_valid_edge(int from, int to);
	const vector<int>& get_kth_src();
	const vector<int>& get_kth_dest();

	void print();

private:
	bool is_disable(int x);

	void build_graph(int x);
	void add_edge(int, int, float);
	void connect_pseudo_edge_source();
	void connect_pseudo_edge_dest();

	void build_SPT();
	void super(PrefixNode* pfx);
	void recover_path(PrefixNode* pfx, int dest, Path* path);

	vector<CacheNode*> nodes;
	vector<int> level_to_nodes;
	vector<bool> marked_level;
	vector<int> next_level;
	unordered_map<int, bool> disable;
	vector<bool> vis;
	vector<int> visited_points;

	// kth
	vector<vector<Edge>> G;
	vector<vector<Edge>> Gr;
	vector<int> trans_id;
	vector<int> all_vec;			  // all valid nodes
	vector<float> dist_to_dest;       // distance to destination
	vector<int> sptTree;              // parent of each node in shortest path tree rooted as destination

	int src_tree, dest_tree;          // source and destination of s-t shortest path
	int pseudo_src, pseudo_dest;
	priority_queue<PrefixNode*, vector<PrefixNode*>, PrefixNode::Compare> kth_Q;
	BC_map* bc_map;
};
#endif
