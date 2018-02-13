#include "bc_map.h"


BC_map::BC_map(Graph* graph){
    this->graph = graph;
}

int BC_map::get_index(Mode mode, Transition_Type type, int node_id){
    return to_map_id[mode][type][node_id];
}

int BC_map::get_graph_id(int map_id){
    return map_id>>2;
}

void BC_map::add_edge(int from, int to, float delay){
    G[from].emplace_back(from, to, delay);
    in[to]++;
}

void BC_map::build(){
    // add node
    // every graph node has 4 nodes in map
    for(size_t i=0; i<graph->nodes.size(); i++){
        to_map_id[EARLY][RISE].emplace_back(num_node++);
        to_map_id[EARLY][FALL].emplace_back(num_node++);
        to_map_id[LATE][RISE].emplace_back(num_node++);
        to_map_id[LATE][FALL].emplace_back(num_node++);
    }
    superSource = num_node++;
    G.resize(num_node);
    in.resize(num_node);
    vis.resize(num_node);
    level.resize(num_node);

    // dfs build map
    vector<int> clocks;
    for(size_t i=0; i<graph->nodes.size(); i++){
        Graph::Node &node = graph->nodes[i];
        if(node.is_clock){
            clocks.push_back(i);
        }
        if(node.node_type==PRIMARY_IN or node.is_clock){
            build_map(i, 0);
            add_edge(superSource, get_index(EARLY, RISE, i), 0);
            add_edge(superSource, get_index(EARLY, FALL, i), 0);
            add_edge(superSource, get_index(LATE, RISE, i), 0);
            add_edge(superSource, get_index(LATE, FALL, i), 0);
        }
    }

    //bfs build level
    for(int i=0; i<num_node; i++) if(in[i]==0)
        q.push(i);

    while(!q.empty()){
        int x = q.front(); q.pop();
        for(const auto& e:G[x]){
            in[e.to]--;
            if(in[e.to]==0) q.push(e.to);
            level[e.to] = max(level[e.to], level[x]+1);
        }
    }

    for(size_t i=0; i<graph->nodes.size(); i++){
        int a = level[ get_index(EARLY, RISE, i) ];
        int b = level[ get_index(EARLY, FALL, i) ];
        int c = level[ get_index(LATE, RISE, i) ];
        int d = level[ get_index(LATE, FALL, i) ];
        // cout << a << " " << b << " " << c << " " << d << endl;
        if(a!=b || a!=c || a!=d || b!=c || b!=d || c!=d){
            cout << " level error\n";
        }
    }
}

void BC_map::build_map(int root, int dep){
    // cout << graph->get_name(root) << endl;
    if(vis[root]) return;
    vis[root] = 1;
    for(const auto& adj_pair:graph->adj[root]){
        int to = adj_pair.first;
        const auto& e = adj_pair.second;
        if(e->type == RC_TREE){
            float delay = e->tree? e->tree->get_delay(EARLY, graph->get_name(to)):0;
            add_edge(get_index(EARLY, RISE, root), get_index(EARLY, RISE, to), delay);
            add_edge(get_index(EARLY, FALL, root), get_index(EARLY, FALL, to), delay);

            delay = e->tree? e->tree->get_delay(LATE, graph->get_name(to)):0;
            add_edge(get_index(LATE, RISE, root), get_index(LATE, RISE, to), delay);
            add_edge(get_index(LATE, FALL, root), get_index(LATE, FALL, to), delay);
        }
        else{
            for(int mm=0; mm<2; mm++){
                for(int ii=0; ii<2; ii++){
                    for(int jj=0; jj<2; jj++){
                        Mode mode = MODES[mm];
                        Transition_Type from_type = TYPES[ii];
                        Transition_Type to_type   = TYPES[jj];

                        for(const auto& arc:e->arcs[mode]){
                            if(!arc->is_transition_defined(from_type, to_type)) continue;
                            float input_slew = graph->nodes[root].slew[mode][from_type];
                            float cload = graph->nodes[to].tree->get_downstream(mode, graph->nodes[to].name);
                            // float cload = e->tree->get_downstream(mode, graph->nodes[to].name);
                            float delay = arc->get_delay(from_type, to_type, input_slew, cload);

                            add_edge(get_index(mode, from_type, root), get_index(mode, to_type, to), delay);
                        }
                    }
                }
            }
        }
        build_map(to, dep+1);
    }
}
