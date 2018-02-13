
#include "kth.h"

void Kth::build_from_src(BC_map* map, const vector<pair<Transition_Type,int>>& through, int src){

    mark_through(map, through);
    // forward_build(src, 0);
}

void Kth::mark_through(BC_map* map, const vector<pair<Transition_Type,int>>& through){

    mark.resize( map->num_node );
    if(through.size()==0) return;

    vector<pair<int,int>> level; // level , node_id

    for(int i=0; i<(int)through.size(); i++){
        int node_id = through[i].second;
        mark[node_id] = 1;
        // level.emplace_back(map->level[node_id], node_id);
    }
    sort(level.begin(), level.end());
    //
    object.emplace_back(level[0].second);
    for(int i=1; i<(int)level.size(); i++){
        if(level[i].second != object.back()){
            object.emplace_back(level[i].second);
        }
    }
}

void Kth::build_from_dest(BC_map* map, const vector<pair<Transition_Type,int>>& through, int dest){

    mark_through(map, through);
    // backward_build(dest, 0);
}
