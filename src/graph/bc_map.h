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
	bool valid;
	Edge():from(0), to(0), delay(0){}
	Edge(int f,int t,float d):from(f),to(t),delay(d) {}
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

	std::atomic<float> threshold;

private:
    void add_edge(int from, int to, float delay);
    void build_map(int root);

	// iterate the all condidates with calling function fun
    //void do_kth(const vector<int>& condidate, size_t k, std::function<void(Kth*,int,int,vector<Path*>&)> fun, vector<Path*>& ans);
	//void search_fin(int x);
	//bool search_fout(int x, int next_level_id);
	//void search(vector<int>& through);

	/* kth */
	//vector<bool> is_valid;
	//vector<bool> is_disable, is_through;
	//vector<int> next_level;
	//vector<int> kth_start, kth_dest;
	//vector<Edge*> valid_edge;   // To clean the mark of the edge of each query, we store the valid edge and clean it after we get the k shortest_path.

	//vector<Path*> path_kth[NUM_THREAD];
	//Kth* kths[NUM_THREAD];
	CacheNode* get_cache_node(int source, CacheNode_Type type);
	CacheNode* add_cache_node(int source, CacheNode_Type type);
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

	//vector<unordered_map<int,CacheNode*>> cache_nodes;
	vector<CacheNode*> cache_nodes[2]; // two type
	vector<CacheNode*> cache_node_collector;
	int query_cnt;


    friend class Kth; 
	friend class CacheNode;
	friend class Cache;
};
#endif /* BATTLE_CAT_MAP */
