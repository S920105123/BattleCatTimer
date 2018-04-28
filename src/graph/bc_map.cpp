#include "bc_map.h"


BC_map::BC_map(Graph* graph){
    this->graph = graph;
}

int BC_map::get_index(Mode mode, Transition_Type type, int node_id){
    if(node_id >= (int)to_map_id[mode][type].size()){
        LOG(CERR) << node_id << " " <<get_mode_string(mode) << " " << graph->get_name(node_id) << " no exist\n";
        ASSERT(false);
    }
    return to_map_id[mode][type][node_id];
}

int BC_map::get_graph_id(int map_id){
    // return map_id>>2;
    return map_id>>1;
}

Mode BC_map::get_graph_id_mode(int map_id){
    return LATE;
    // return (map_id&2)? LATE:EARLY;
}

Transition_Type BC_map::get_graph_id_type(int map_id){
    return (map_id&1)? FALL:RISE;
}

string BC_map::get_node_name(int map_id){
    int graph_id = get_graph_id(map_id);
    Mode mode = get_graph_id_mode(map_id);
    Transition_Type type = get_graph_id_type(map_id);
    return graph->nodes[graph_id].name+":"+get_mode_string(mode)+":"+get_transition_string(type);
}

void BC_map::add_edge(int from, int to, float delay){
    G[from].emplace_back(from, to, delay);
    Gr[to].emplace_back(to, from, delay);
    in_degree[to]++;
}

void BC_map::build(){
    // add node
    // every graph node has 2 nodes in map
    num_node = 0;
    for(size_t i=0; i<graph->nodes.size(); i++){
        // to_map_id[EARLY][RISE].emplace_back(num_node++);
        // to_map_id[EARLY][FALL].emplace_back(num_node++);
        // if(!graph->nodes[i].exist) continue;
        to_map_id[LATE][RISE].emplace_back(num_node++);
        to_map_id[LATE][FALL].emplace_back(num_node++);
    }
    G.resize(num_node);
    Gr.resize(num_node);
    in_degree.resize(num_node);
    vis.resize(num_node);
    level.resize(num_node);
	is_disable.resize(num_node);
	is_valid.resize(num_node);
	is_through.resize(num_node);

    // dfs build map
    for(int i=0; i<(int)graph->nodes.size(); i++) if(i!=graph->clock_id) {
        Graph::Node &node = graph->nodes[i];
        if(node.type == CLOCK || node.type == PRIMARY_IN)
			build_map(i);
    }

    //bfs build level
    queue<int>* q = new queue<int>();
    for(int i=0; i<num_node; i++) if(in_degree[i]==0) {
    	q->push(i);
	}

    while(!q->empty()){
        int x = q->front(); q->pop();
        for(const auto& e:G[x]){
            in_degree[e.to]--;
            if(in_degree[e.to]==0) q->push(e.to);
            level[e.to] = max(level[e.to], level[x]+1);
        }
    }

	delete q;
    LOG(NORMAL) << "BCmap nodes = " << num_node << "\n";
}

void BC_map::build_map(int root){
    if(vis[root]) return;
    vis[root] = 1;
    for(const auto& adj_pair:graph->adj[root]){
        int to = adj_pair.first;
        const auto& e = adj_pair.second;
        if(e->type == RC_TREE){
            // float delay = e->tree? e->tree->get_delay(EARLY, graph->get_name(to)):0;
            // add_edge(get_index(EARLY, RISE, root), get_index(EARLY, RISE, to), delay);
            // add_edge(get_index(EARLY, FALL, root), get_index(EARLY, FALL, to), delay);
            //
            // delay = e->tree? e->tree->get_delay(LATE, graph->get_name(to)):0;
            // // cout << "Rc edge ?? ";
            // int from = e->from;
            // ASSERT(from == root);
            // // cout << graph->nodes[from].exist << " " << graph->nodes[to].exist << " ";
            // // cout << graph->nodes[from].name << " " << graph->nodes[to].name << "\n";
            //
            float delay = 0;
            add_edge(get_index(LATE, RISE, root), get_index(LATE, RISE, to), -delay);
            add_edge(get_index(LATE, FALL, root), get_index(LATE, FALL, to), -delay);
        }
        else{
            // for(int mm=0; mm<2; mm++){
                for(int ii=0; ii<2; ii++){
                    for(int jj=0; jj<2; jj++){
                        // Mode mode = MODES[mm];
                        Mode mode = LATE;
                        Transition_Type from_type = TYPES[ii];
                        Transition_Type to_type   = TYPES[jj];

                        for(const auto& arc:e->arcs[mode]){
                            if(!arc->is_transition_defined(from_type, to_type)) continue;
                            float input_slew = graph->nodes[root].slew[mode][from_type];
                            float cload = graph->nodes[to].tree->get_downstream(mode, graph->nodes[to].name);
                            // float cload = e->tree->get_downstream(mode, graph->nodes[to].name);
                            float delay = arc->get_delay(from_type, to_type, input_slew, cload);

                            if(mode==LATE) delay *= -1;
                            add_edge(get_index(mode, from_type, root), get_index(mode, to_type, to), delay);
                        }
                    }
                }
            // }
        }
        build_map(to);
    }
}

/******************************************
*           k shortest path               *
******************************************/
void BC_map::k_shortest_path(vector<int>& through, const vector<int>& disable, int nworst, vector<Path>& ans)
{
/* get next_level */
	std::sort(through.begin(), through.end(), [this](int a,int b) {
				return this->level[a] < this->level[b];
		   	});

	mark_point(through, disable);

	cout << "sorted through by level\n";
	for(auto x:through) {
		cout << get_node_name(x) << "(" << level[x] << ") -> ";
	}
	cout << '\n';

/* mark searching space */
	kth_start.clear(); // FF:clk or Pin
	kth_dest.clear();  // FF:D or Pout
	std::fill(vis.begin(), vis.end(), 0);

	if(through.size()) {
		for(int i=0; i<(int)through.size(); i++) {
			if(level[ through[i] ] == next_level[ 0 ]) {
				if(search_fout( through[i], 1)) {
					is_valid[i] = 1;
					search_fin( through[i] );
				}
			}
			else break;
		}
	}
	else {
		ASSERT(next_level.size() == 0);
		// take all of FF:clk and Pin as a start point
		for(int i=0; i<(int)G.size(); i++){
			// i is a FF:clk or Pin
			if(Gr[i].size() == 0) {
				kth_start.push_back(i);
				if(search_fout(i, 0)) is_valid[i] = 1;
			}
		}
	}

	cout << "kth_start:" << kth_start.size() << "\n";
	for(auto x:kth_start) {
		cout << get_node_name(x) << ":" << level[x] << "\n";
	} cout << '\n';

	cout << "kth_dest:" << kth_dest.size() << "\n";
	for(auto x:kth_dest) {
		cout << get_node_name(x) << ":" << level[x] << "\n";
	} cout << '\n';

/* query kth */

	if( kth_start.size() < kth_dest.size() ) {

	}
	else {

	}
}

void BC_map::mark_point(const vector<int>& through, const vector<int>& disable) {


	std::fill(is_valid.begin(), is_valid.end(), 0);
	std::fill(is_disable.begin(), is_disable.end(), 0);
	std::fill(is_through.begin(), is_through.end(), 0);
	next_level.clear();

    if(through.size()) next_level.emplace_back(level[through[0]]);
    for(int i=1; i<(int)through.size(); i++) {
        if(level[through[i]] != next_level.back()) {
            next_level.emplace_back(level[through[i]]);
        }
    }

	for(auto &x: disable) is_disable[x] = true;
	for(auto &x: through) is_through[x] = true;
}

void BC_map::search_fin(int x) {
	
	is_valid[x] = true;
	if(Gr[x].size() == 0) {
		kth_start.push_back(x) ;
		return;
	}

	if(vis[x]) return;
	vis[x] = 1;

	for(auto& e: Gr[x]) {
		if(is_disable[e.to] == false) search_fin(e.to);
	}
}

bool BC_map::search_fout(int x,int next_level_id) {

	if(vis[x]) return is_valid[x];
	vis[x] = true;

	// no fout: FF:D or Pout
	if(G[x].size()==0) {
		if(next_level_id == (int)next_level.size()) {
			kth_dest.push_back( x );
			return is_valid[x] = true;
		}
		else return is_valid[x] = false;
	}
	else {
		for(auto &e: G[x]) {
			int to = e.to;
			if(is_disable[to] == true) continue;

			if(next_level_id == (int) next_level.size()) {
				if(search_fout(to, next_level_id)) is_valid[x] = true;
			}
			/* next_level_id < next_level.size() */
			else if(level[to] == next_level[next_level_id] and is_through[to]) {
				if(search_fout(to, next_level_id+1)) is_valid[x] = true;
			}
			else if(level[to] < next_level[next_level_id]) {
				if(search_fout(to, next_level_id)) is_valid[x] = true;
			}
		}
		return is_valid[x];
	}
}
