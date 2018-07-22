#ifndef CACHE_NODE
#define CACHE_NODE

#include "header.h"
#include "bc_map.h"
#include "bct_bitset.h"

class BC_map;

// save the valid graph from {src} to {dest}
struct Cache_Edge {
	int from, to;
	float delay;

	Cache_Edge() {}
	Cache_Edge(int f, int t, float d):from(f), to(t), delay(d){}
};

class CacheNode {

public:

	CacheNode(BC_map* map, int src, int dest);
	~CacheNode();

	void update();
	void set_src_dest(int, int);

	const vector<int>& get_topological_order() { return topological_order; }
	const vector<Cache_Edge*>& get_valid_edge(int x) { return valid_edges[x]; }
	const vector<Cache_Edge*>& get_valid_edge_reverse(int x) { return valid_edges_reverse[x]; }
	void clear();
	void print();

	int start_level, end_level;
	int last_used; // keep the last used query id
	int used_cnt;
	int source, dest;

private:
	// use bc's reverse graph to search all ff:clk or PIO
	bool search_source(int);
	// search paths from src to dest, if dest == -1, then all ff:d or POUT are destination
	bool search_dest(int);

	void add_edge(int from,int to, float delay);
	void connect_pseudo_edge_source();
	void connect_pseudo_edge_dest();

	unordered_map<int, vector<Cache_Edge*>> valid_edges;
	unordered_map<int, vector<Cache_Edge*>> valid_edges_reverse;
	//vector<vector<Cache_Edge*>> valid_edges;
	//vector<vector<Cache_Edge*>> valid_edges_reverse;
	vector<Cache_Edge*> edge_collector;
	//unordered_map<int, bool> vis;
	vector<bool> vis, is_valid;
	vector<int> visited_points;
	// BitSet vis;
	// BitSet is_valid;

	vector<int> kth_src, kth_dest;
	vector<int> topological_order;

	BC_map* bc_map;

	bool has_built;
};
#endif
