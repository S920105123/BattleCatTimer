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

public:
	Cache(BC_map*);

	void clear();
	void set_disable(const vector<int>&);
	void add_cache_node(CacheNode* node);             // the level of cacheNode is increased 
	void update_cacheNode();                          // build all cacheNode
	void kth(vector<Path*>& ans, int k);
	void output_shortest_path();

	void print();

private:
	void build_SPT();
	void recover_path(PrefixNode* pfx, int dest, Path* path);
	void super(PrefixNode* pfx);
	void push_to_queue(PrefixNode* pfx, int where, float delta);

	bool is_disable(int x);
	const vector<Cache_Edge*> get_edges(int x);        // get edges from CacheNode
	const vector<Cache_Edge*> get_edges_reverse(int x);

	vector<CacheNode*> nodes;
	vector<int> level_to_nodes;

	unordered_map<int, bool> disable;

	unordered_map<int, int> trans_id; // bc_map id to shortest path tree id
	vector<int> all_vec;              // all valid nodes
	vector<float> dist_to_dest;       // distance to destination
	vector<int> sptTree;              // parent of each node in shortest path tree rooted as destination
	int src_tree, dest_tree;          // source and destination of s-t shortest path

	priority_queue<PrefixNode*, vector<PrefixNode*>, PrefixNode::Compare> kth_Q;
	BC_map* bc_map;
};
#endif
