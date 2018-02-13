
#include "kth.h"

Kth::Kth(BC_map *_map, CPPR *_cppr){
    map = _map;
    cppr = _cppr;
    num_node = 0;
    to_kth_id.resize( map->num_node );
    is_good.resize( map->num_node );
    has_kth_id.resize( map->num_node );
    vis.resize( map->num_node );
}

int Kth::add_node(int bc_node_id){
    to_bc_id.emplace_back(bc_node_id);
    G.emplace_back(vector<Edge>());
    if(bc_node_id != -1) to_kth_id[ bc_node_id ] = num_node, has_kth_id[bc_node_id] = 1;
    return num_node++;
}

int Kth::get_kth_id(int bc_node_id){
    if(has_kth_id[bc_node_id]) return to_kth_id[bc_node_id];
    else return add_node(bc_node_id);
}

int Kth::add_edge(int from, int to, float delay){ // from , to in bc map
    from = get_kth_id(from);
    to = get_kth_id(to);
    G[from].emplace_back(from, to, delay);
}

int Kth::add_edge(int from, int to, float delay, float clock_delay){ // from , to in bc map
    from = get_kth_id(from);
    to = get_kth_id(to);
    G[from].emplace_back(from, to, delay);
    G[from][ G[from].size()-1 ].clock_delay = clock_delay;
}

void Kth::mark_through(const vector<pair<Transition_Type,int>>& through){

    mark.resize( map->num_node );
    if(through.size()==0) return;

    vector<pair<int,int>> level; // level , node_id

    for(int i=0; i<(int)through.size(); i++){
        int node_id = through[i].second;
        mark[node_id] = 1;
        level.emplace_back(map->level[node_id], node_id);
    }
    sort(level.begin(), level.end());

    object.emplace_back(level[0].second);
    for(int i=1; i<(int)level.size(); i++){
        if(level[i].second != object.back()){
            object.emplace_back(level[i].second);
        }
    }
}

// src: in bc_map
void Kth::build_from_src(const vector<pair<Transition_Type,int>>& through, int src, bool only_src){

    // super dest, every ff:d will connect to this node, and the weight is cppr(src, ff:d) + slack(ff:d)
    dest_kth = add_node(-1);

    mark_through(through);

    source_kth = add_node(-1);

    if(!only_src){ // it has 4 src
        int graph_id = map->get_graph_id(src);
        int map_id;
        float clock_at;

        for(int i=0; i<2; i++){
            for(int j=0; j<2; j++){
                Mode mode = MODES[i];
                Transition_Type type = TYPES[j];
                map_id = map->get_index(mode, type, graph_id);
                clock_at = map->graph->nodes[graph_id].at[mode][type];
                add_edge(source_kth, map_id, 0, clock_at);
                forward_build(map_id, 0);
            }
        }
    }
    else{
        Mode mode = map->get_graph_id_mode( src );
        Transition_Type type = map->get_graph_id_type( src );
        float clock_at = map->graph->nodes[map->get_graph_id(src)].at[mode][type];
        add_edge(source_kth, src, clock_at);
        forward_build(src, 0);
    }

    // build all ff:d to desk_kth
    for(auto x:all_leave){
        int graph_id = map->get_graph_id( src );
        Transition_Type clk_mode = map->graph->nodes[graph_id].clk_edge;
        Transition_Type _mode = map->graph->nodes[graph_id].clk_edge;
    }
}


bool Kth::forward_build(int now, int next_object){
    if(vis[now]) return is_good[now];
    vis[now] = 1;

    if(map->G[now].size()==0){ // at primary out or ff:d
        all_leave.emplace_back(now);
        return true;
    }

    for(int i=0; i<(int)map->G[now].size(); i++){
        int to = map->G[now][i].to;
        if(next_object >= object.size() ){
            add_edge(now, to, map->G[now][i].delay);
            forward_build(to, next_object);
        }
        else if(mark[to] and map->level[to]==object[next_object]){
            add_edge(now, to, map->G[now][i].delay);
            forward_build(to, next_object+1);
        }
        else if(map->level[to] < object[next_object]){
            if(forward_build(to, next_object)){
                add_edge(now, to, map->G[now][i].delay);
            }
        }
    }
}

void Kth::build_from_dest(const vector<pair<Transition_Type,int>>& through, int dest, bool only_dest){

    mark_through(through);
    // backward_build(dest, 0);
}
