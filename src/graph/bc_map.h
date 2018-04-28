#ifndef BATTLE_CAT_MAP
#define BATTLE_CAT_MAP

#include "header.h"
#include "debug.h"
#include "kth.h"
#include "graph.h"

class Graph;
class Kth;
class BC_map {

public:
    BC_map(Graph* graph);

    void build();
    int get_index(Mode mode,Transition_Type type, int node_id);
    int get_graph_id(int map_id);      // map id to graph node id
    Mode get_graph_id_mode(int map_id);
    Transition_Type get_graph_id_type(int map_id);
    string get_node_name(int map_id);
	void k_shortest_path(vector<int>& _through, 
						 const vector<int>& _disable, 
						 int nworst,
						 vector<Path>& ans);

    struct Edge{
        int from, to;
        float delay;
        Edge(){}
        Edge(int f,int t,float d):from(f),to(t),delay(d){}
    };

private:
    void add_edge(int from, int to, float delay);
    void build_map(int root);

	void mark_point(const vector<int>& through, const vector<int>& disable);
	void search_fin(int x);
	bool search_fout(int x, int next_level_id);

    vector<int> to_map_id[2][2];       // graph node id to bc map id
    vector<int> level, in_degree, vis;

	/* kth */
	vector<bool> is_valid;			   
	vector<bool> is_disable, is_through;
	vector<int> next_level;		  
	vector<int> kth_start, kth_dest;  

    Graph* graph;
    int num_node;
    int superSource;

    vector<vector<Edge>> G;
    vector<vector<Edge>> Gr;

    friend class Kth;
};
#endif /* BATTLE_CAT_MAP */
