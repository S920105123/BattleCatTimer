#ifndef CACHE_NODE
#define CACHE_NODE

#include "header.h"
#include "bc_map.h"
#include "bct_bitset.h"

class BC_map;

// record the fin of source
class CacheNode {

public:

	CacheNode(BC_map* map, int src, CacheNode_Type type);
	~CacheNode();

	void set_target_level(int target_level);
	void update();
	void init(int src, CacheNode_Type type);
	void clear();

	const vector<int>& get_kth_src();
	const vector<int>& get_kth_dest();
	bool is_valid_point(int x) { return is_valid[x]; }

	void print();

	/*
	 * CACHE_FIN:
	 * 		search fin of source to searched_level
	 *
	 * CACHE_FOUT:
	 * 		search fout of source to FF:D or POUT
	 *
	 * */
	CacheNode_Type cache_type;

	int source;
	int source_level;
	int searched_level;
	bool need_update;

	int last_used;
	int used_cnt;
private:
	// use bc's reverse graph to search all ff:clk or PIO
	void search_source(int);
	bool search_dest(int);

	vector<bool> is_valid;
	vector<bool> vis; 
	vector<int> visited_points; 
	vector<int> valid_points;

	vector<int> endpoints; // for CACHE_FIN bfs
	vector<int> next_endpoints;
	vector<int> kth_src, kth_dest;

	BC_map* bc_map;
};
#endif
