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

struct Edge{
	int from, to;
	float delay;
	int id;
	int nodes_on_jump_id;
	//unordered_map<int,bool> nodes_jump_edge;
	Edge():from(0), to(0), delay(0){}
	Edge(int f,int t,float d):from(f),to(t),delay(d){}
};

class BC_map {

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
	void add_jump_edge(int from, int to, float delay);
    void build_map(int root);
	void condense(int x, int root, float delay);
	void condense(int x);
	bool is_jump_tail(int x);
	void check_condense(); // for debugging


	CacheNode* get_cache_node(int source, CacheNode_Type type);
	CacheNode* add_cache_node(int source, CacheNode_Type type);
	void erase_cache_node(CacheNode*);

    Graph* graph;
	CPPR* cppr;
	Cache* cache;

    int num_node, num_edge;

    vector<vector<Edge*>> G;			// graph
    vector<vector<Edge*>> Gr;
	vector<vector<Edge*>> J;			// jump
	vector<vector<Edge*>> Jr;
	vector<unordered_map<int,bool>> nodes_on_jump;

    vector<int> to_map_id[2][2];       // graph node id to bc map id
    vector<int> level, in_degree, vis;
	vector<int> topological_order;
	vector<int> condensed_by;
	vector<int> nodes_jump_edge;
	int max_level;

	queue<CacheNode*> free_cache_node;
	vector<CacheNode*> cache_nodes[2]; // two type
	vector<CacheNode*> cache_node_collector;
	int query_cnt;


    friend class Kth; 
	friend class CacheNode;
	friend class Cache;
	friend class Path;
};
#endif /* BATTLE_CAT_MAP */
