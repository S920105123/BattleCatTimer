#ifndef BATTLE_CAT_MAP
#define BATTLE_CAT_MAP

#include "header.h"
#include "debug.h"
//#include "kth.h"
#include "graph.h"
#include "cppr.h"
#include "cache_node.h"
#include "cache.h"

class Graph;
class Kth;
class CacheNode;
class Cache;
class CPPR;


class BC_map {

	struct Edge{
		int from, to;
		float delay;
		int id;
		Edge():from(0), to(0), delay(0){}
		Edge(int f,int t,float d):from(f),to(t),delay(d) {}
	};

public:
    BC_map(Graph* graph, CPPR* cppr);
	~BC_map();

    void build();
    int get_index(Mode mode,Transition_Type type, int node_id);
    int get_graph_id(int map_id);      // map id to graph node id
    Mode get_graph_id_mode(int map_id);
    Transition_Type get_graph_id_type(int map_id);
    string get_node_name(int map_id);

	void k_shortest_path(vector<int>& _through,
						 vector<int>& _disable,
						 int k,
						 vector<Path*>& ans);

	//std::atomic<float> threshold;

private:
    void add_edge(int from, int to, float delay);
    void build_map(int root);

	CacheNode* get_cache_node(int from, int to);
	CacheNode* add_cache_node(int from, int to);
	void erase_cache_node(CacheNode*);

    Graph* graph;
	CPPR* cppr;
	Cache* cache;

    int num_node, num_edge;
    int superSource;

    vector<vector<Edge*>> G;
    vector<vector<Edge*>> Gr;

    vector<int> to_map_id[2][2];       // graph node id to bc map id
    vector<int> level, in_degree, vis;
	int max_level;

	vector<unordered_map<int,CacheNode*>> cache_nodes;
	vector<CacheNode*> cache_node_collector;
	int query_cnt;


    friend class Kth; 
	friend class CacheNode;
	friend class Cache;
};
#endif /* BATTLE_CAT_MAP */
