#include "cppr.h"

CPPR::CPPR(Graph* _graph, int clock_root){
    graph = _graph;
    root = clock_root;
    num = 0;
}

CPPR::~CPPR(){
}

void CPPR::build_tree(){
    if(root==-1) return;
    dfs_build(root, 0, 0);
    build_sparse();
}

void CPPR::add_node(int node_id,int dep, int neg){
    to_node_id.push_back(node_id);
    to_tree_id[ node_id ] = to_node_id.size()-1;
    // graph->nodes[node_id].in_cppr = true;
    negation.push_back(neg);
    level.push_back(dep);
    who.push_back(to_node_id.size()-1);
    in.push_back(level.size()-1);       // node in time
    // cout << graph->nodes[node_id].name << " tree id = " << num << '\n';
    num++;
}

void CPPR::dfs_build(int root, int dep, int neg){
    add_node(root, dep, neg);
    bool is_neg = 0;
    if(graph->nodes[ root ].type == CLOCK) return;

    for(const auto& adj_pair:graph->adj[root]){
        int to = adj_pair.first;
        const auto& e = adj_pair.second;
        for(const auto& arc:e->arcs[EARLY]){
            if(arc->get_timing_sense()==Timing_Sense::NEGATIVE_UNATE)
                is_neg = 1;
        }
        dfs_build(to, dep+1, neg+is_neg );
        level.push_back(dep);
        who.push_back(to_tree_id[root]);
    }
}

void CPPR::build_sparse(){
    // cout << "num = " << num << '\n';
    // cout << "time:  "; for(size_t t=0; t<level.size(); t++) cout << t << '\n';
    // cout << "level: "; for(auto x:level) cout << x << " " ; cout << '\n';
    // cout << "who:   "; for(auto x:who) cout << x << " " ; cout << '\n';
    //
    // for(int i=0; i<num; i++) cout << graph->nodes[ to_node_id[i] ].name << ": " << in[i] << "\n"; cout << '\n';

    int len = 0;
    for(int i=0; (1<<i)<=(int)level.size(); i++) len++;

    T.resize(level.size()+2);
    for(size_t i=0; i<level.size(); i++) T[i].resize(len+2);
    for(size_t i=0; i<level.size(); i++) T[i][0] = i;

    for(int i=1; i<=len; i++){
        for(int j=0; j+(1<<i)-1<(int)level.size(); j++){
            if(level[ T[j][i-1] ] > level[ T[j+(1<<(i-1))][i-1] ])
                T[j][i] = T[j+(1<<(i-1))][i-1];
            else T[j][i] = T[j][i-1];
        }
    }
}

float CPPR::cppr_credit(Mode mode, int u, Transition_Type type_u, int v, Transition_Type type_v){

	return 0;
    if(root==-1) return 0;
    // cout << graph->nodes[u].name << " " << graph->nodes[v].name << '\n';
    int tree_u = to_tree_id[u], tree_v = to_tree_id[v];
    int lca = get_lca(in[tree_u], in[tree_v]);
    int num_neg_u = negation[lca] - negation[ tree_u ];
    int num_neg_v = negation[lca] - negation[ tree_v ];

    if(num_neg_u & 1) type_u = type_u==RISE? FALL:RISE;
    if(num_neg_v & 1) type_v = type_v==RISE? FALL:RISE;
    if(type_u != type_v){
        // LOG(CERR) << " lca type is different " << graph->nodes[u].name << " " << graph->nodes[v].name << '\n';
        return 0;
    }
    Transition_Type root_type = type_u;
    if(negation[tree_u]&1) root_type = root_type==RISE? FALL:RISE;

    lca = to_node_id[lca];
    // cout << "lca = " << graph->nodes[lca].name << '\n';
    // cout << "root = " << get_transition_string( root_type ) << '\n';
    // cout << "lca = " << get_transition_string( type_u ) << '\n';
    // cout << graph->nodes[root].at[LATE][RISE]  << " " ;
    // cout << graph->nodes[root].at[LATE][FALL]  << " " ;
    // cout << graph->nodes[root].at[EARLY][RISE] << " " ;
    // cout << graph->nodes[root].at[EARLY][FALL] << " "  << '\n';
    //
    // cout << graph->nodes[lca].at[LATE][RISE]  << " " ;
    // cout << graph->nodes[lca].at[LATE][FALL]  << " " ;
    // cout << graph->nodes[lca].at[EARLY][RISE] << " " ;
    // cout << graph->nodes[lca].at[EARLY][FALL] << " "  << '\n';
    if(mode==EARLY){ // hold
        return graph->nodes[lca].at[LATE][type_u] - graph->nodes[lca].at[EARLY][type_u];
    }
    else{
        float root_credit = graph->nodes[root].at[LATE][root_type] - graph->nodes[root].at[EARLY][root_type];
        // cout << "root credit = " << root_credit << '\n';
        return graph->nodes[lca].at[LATE][type_u] - graph->nodes[lca].at[EARLY][type_u]
            - (root_credit);
    }
}

int CPPR::get_lca(int u,int v){
    if(u>v) swap(u, v);
    int gap = v-u;

    // cout << gap << '\n';
    if(u==v) return who[u];
    int k = 0;
    while((1<<(k+1)) <= gap) k++;
    if( level[T[u][k]] > level[T[ v-(1<<k)+1 ][k]] ) return who[ T[ v-(1<<k)+1 ][k] ];
    else return who[ T[u][k] ];
}

/*
../testcase_v1.2/s1196/s1196.tau2015 ../testcase_v1.2/s1196/s1196.timing ../testcase_v1.2/s1196/s1196.ops ../testcase_v1.2/s1196/s1196.ui_output
*/
