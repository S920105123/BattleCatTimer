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

void Kth::add_edge(int from, int to, float delay){
    G[from].emplace_back(from, to, delay);
}

void Kth::add_edge(int from, int to, float delay, float clock_delay){
    G[from].emplace_back(from, to, delay);
    G[from][ G[from].size()-1 ].at_delay = clock_delay;
}

string Kth::get_node_name(int kth_id){
    if(kth_id==source_kth) return "SuperSource";
    if(kth_id==dest_kth)   return "SuperDest";
    int map_id = to_bc_id[kth_id];
    return map->get_node_name(map_id);

}

void Kth::print(){
    LOG(CERR) << num_node << " " << G.size() << endl;
    for(int i=0; i<num_node; i++){
        for(auto x:G[i]){
            int to = x.to;
            LOG(CERR) << get_node_name(i) << " -> " << get_node_name(to) << " " << x.delay << endl;
        }
    }
}

void Kth::mark_through(const vector<pair<Transition_Type,int>>& through){

    mark.resize( map->num_node );
    if(through.size()==0) return;

    vector<pair<int,int>> level; // level , node_id
    // vector<int> level;

    for(int i=0; i<(int)through.size(); i++){
        int map_id = through[i].second;
        int graph_id = map->get_graph_id(map_id);

        map_id = map->get_index(EARLY, through[i].first, graph_id);
        mark[map_id] = 1;
        // level.emplace_back(map->level[map_id]);
        level.emplace_back(map->level[map_id], map_id);
        // LOG(CERR) << map->get_node_name(map_id) << " " << map->level[map_id] << endl;

        map_id = map->get_index(LATE, through[i].first, graph_id);
        mark[map_id] = 1;
        // level.emplace_back(map->level[map_id]);
        level.emplace_back(map->level[map_id], map_id);
        // LOG(CERR) << map->get_node_name(map_id) << " " << map->level[map_id] << endl;

    }
    sort(level.begin(), level.end());

    for(auto x:level){
        LOG(CERR) << map->get_node_name(x.second) << " level = " << x.first << endl;
    }
    if(level.size()) object.emplace_back(level[0].first);
    for(int i=1; i<(int)level.size(); i++){
        if(level[i].first != object.back()){
            object.emplace_back(level[i].first);
        }
    }
}

// src: in bc_map
void Kth::build_from_src(const vector<pair<Transition_Type,int>>& through, int src, bool specify){

    LOG(CERR) << "builf from src\n";
    // super dest, every ff:d will connect to this node, and the weight is cppr(src, ff:d) + slack(ff:d)
    dest_kth = add_node(-1);
    source_kth = add_node(-1);

    mark_through(through);

    // source_kth to all may source(early, late, rise, fall), and dfs them
    if(!specify){ // it has 4 src node in map
        // trun bc_map to graph id , then get bc_map id by specifing mode and transition
        int graph_id = map->get_graph_id(src);
        int map_id;
        float clock_at;

        for(int i=0; i<2; i++){
            for(int j=0; j<2; j++){
                Mode mode = MODES[i];
                Transition_Type type = TYPES[j];
                map_id = map->get_index(mode, type, graph_id);
                if(map->graph->nodes[graph_id].is_clock)
                    clock_at = map->graph->nodes[graph_id].at[mode][type];
                else clock_at = 0;
                add_edge(source_kth, get_kth_id(map_id), 0, clock_at);
                // LOG(CERR) << "connect " << get_node_name(source_kth) << " to " << get_node_name(get_kth_id(map_id)) << endl;
                forward_build(map_id, 0);
            }
        }
        LOG(CERR) << "dfs ok\n";
    }
    else{
        // just specify Transition_Type, so it has two mode
        int graph_id = map->get_graph_id(src);
        Transition_Type type = map->get_graph_id_type(src);
        for(int i=0; i<2; i++){
            Mode mode = MODES[i];
            float clock_at = map->graph->nodes[map->get_graph_id(src)].at[mode][type];
            int map_id = map->get_index(mode, type, graph_id);
            if(!map->graph->nodes[map->get_graph_id(src)].is_clock) clock_at = 0;
            add_edge(source_kth, get_kth_id(map_id) , clock_at);
            // LOG(CERR) << "*connect " << get_node_name(source_kth) << " to " << get_node_name(get_kth_id(map_id)) << endl;
            forward_build(map_id, 0);
        }
    }

    for(auto x:all_leave ){
        LOG(CERR) << "leaf : " << map->get_node_name(x) << endl;
    }
    // build all ff:d to desk_kth
    for(auto x:all_leave){
        int src_id = map->get_graph_id(src);
        int leaf_id = map->get_graph_id(x);
        Transition_Type leaf_type = map->get_graph_id_type(x);
        Mode mode = map->get_graph_id_mode(x);
        const auto& leaf_node = map->graph->nodes[ leaf_id ];
        float delay = 0;

        LOG(CERR) << "cppr: " <<  map->get_node_name(src) << " " << map->get_node_name(x);
        if(map->graph->nodes[src_id].is_clock and map->graph->nodes[leaf_id].node_type!=PRIMARY_OUT){
            int leaf_clk_id= map->graph->nodes[leaf_id].constrained_clk; // ff:d's clk
            if(leaf_clk_id==-1) LOG(ERROR) << "[kth][build_from_src] leaf'ck is -1, leaf: " << map->get_node_name(x) << endl;
            ASSERT(leaf_clk_id!=-1);

            Transition_Type src_clk_type = map->graph->nodes[src_id].clk_edge;       // clk trigger type
            Transition_Type leaf_clk_type = map->graph->nodes[leaf_clk_id].clk_edge;

            delay += cppr->cppr_credit(mode, src_id, src_clk_type, leaf_clk_id, leaf_clk_type);
            LOG(CERR) << "(" << map->get_node_name( map->get_index(mode, leaf_clk_type, leaf_clk_id)) << ") " << delay << endl;
        }
        else LOG(CERR) << " no cppr\n";

        // cppr plus slack = Woffest
        if(mode==EARLY) delay += leaf_node.at[mode][leaf_type] - leaf_node.rat[mode][leaf_type];
        else delay +=  leaf_node.rat[mode][leaf_type] - leaf_node.at[mode][leaf_type];

        // connect to dest_kth
        add_edge(get_kth_id(x), dest_kth, delay);
    }
    print();
}

bool Kth::forward_build(int now, int next_object){
    if(vis[now]) return is_good[now];
    vis[now] = 1;

    LOG(CERR) << "dfs: " << map->get_node_name(now) << " my level: " << map->level[now]
    << " next level: " << object[next_object] << endl;
    if(map->G[now].size()==0){ // at primary out or ff:d
        if(next_object==(int)object.size()){
            all_leave.emplace_back(now);
            is_good[now] = true;
            return true;
        }
        else return is_good[now] = false;
    }

    for(int i=0; i<(int)map->G[now].size(); i++){
        int to = map->G[now][i].to;
        // has searched all through point
        if(next_object >= (int)object.size() ){
            add_edge(get_kth_id(now), get_kth_id(to), map->G[now][i].delay);
            forward_build(to, next_object);
            is_good[now] = true;
        }
        // find object
        else if(mark[to] and map->level[to]==object[next_object]){
            if( forward_build(to, next_object+1) ){
                add_edge(get_kth_id(now), get_kth_id(to), map->G[now][i].delay);
                is_good[now] = true;
            }
        }
        else if(map->level[to] < object[next_object]){
            if(forward_build(to, next_object)){
                add_edge(get_kth_id(now), get_kth_id(to), map->G[now][i].delay);
                is_good[now] = true;
            }
        }
    }
    return is_good[now];
}

void Kth::build_from_dest(const vector<pair<Transition_Type,int>>& through, int dest, bool specify){

    LOG(CERR) << "builf from dest\n";
    mark_through(through);
    // hight level to low level
    reverse(object.begin(), object.end());

    source_kth = add_node(-1);
    dest_kth   = add_node(-1);

    // may dest(early, late, rise, fall) to dest_kth, and dfs them
    if(!specify){
        int graph_id = map->get_graph_id(dest);
        int map_id;

        for(int i=0; i<2; i++){
            for(int j=0; j<2; j++){
                Mode mode = MODES[i];
                Transition_Type type = TYPES[j];

                map_id = map->get_index(mode, type, graph_id);
                // LOG(CERR) << "connnect " << get_node_name(dest_kth) << " " << map->get_node_name(map_id) << endl;
                add_edge(get_kth_id(map_id), dest_kth, 0);
                backward_build(map_id, 0);
            }
        }
    }else{
        // type is specified by map id
        Transition_Type type = map->get_graph_id_type(dest);
        int graph_id = map->get_graph_id(dest);
        for(int i=0; i<2; i++){
            Mode mode = MODES[i];
            int map_id = map->get_index(mode, type, graph_id);
            // LOG(CERR) << "*connnect " << get_node_name(dest_kth) << " " << map->get_node_name(map_id) << endl;
            add_edge(get_kth_id(map_id), dest, 0);
            backward_build(map_id, 0);
        }
    }

    // connect all leave to source_kth
    for(auto x:all_leave){
        // int src_clk_id
        int dest_id = map->get_graph_id(dest);
        int leaf_id = map->get_graph_id(x);
        Mode mode = map->get_graph_id_mode(x);
        const auto & dest_node = map->graph->nodes[dest_id];
        Transition_Type dest_type = map->get_graph_id_type(dest);
        float delay = 0, at_clock = 0;

        LOG(CERR) << "cppr: " <<  map->get_node_name(x) << " " << map->get_node_name(dest) ;
        if(map->graph->nodes[dest_id].node_type!=PRIMARY_OUT and map->graph->nodes[leaf_id].is_clock){
            int dest_ck_id = map->graph->nodes[dest_id].constrained_clk;
            if(dest_ck_id==-1) LOG(ERROR) << "[kth][build_from_dest] dest'ck is -1, dest: " << map->get_node_name(dest) << endl;
            ASSERT(dest_ck_id!=-1);

            Transition_Type dest_ck_type = map->graph->nodes[dest_ck_id].clk_edge;
            Transition_Type leaf_ck_type = map->graph->nodes[leaf_id].clk_edge;
            delay += cppr->cppr_credit(mode, dest_ck_id, dest_ck_type, leaf_id, leaf_ck_type);
            LOG(CERR) << "(" << map->get_node_name( map->get_index(mode, dest_ck_type, dest_ck_id)) << ") " << delay << endl;

        }
        else LOG(CERR) << " no cppr\n";

        if(mode==EARLY) delay += dest_node.at[EARLY][dest_type]-dest_node.rat[EARLY][dest_type];
        else delay += dest_node.rat[LATE][dest_type] - dest_node.at[LATE][dest_type];

        if(map->graph->nodes[leaf_id].is_clock){
            at_clock = map->graph->nodes[leaf_id].at[mode][ map->get_graph_id_type(x) ];
        }
        add_edge(source_kth, get_kth_id(x), delay, at_clock);
    }

    print();
}

bool Kth::backward_build(int now, int next_object){
    if(vis[now]) return is_good[now];
    vis[now] = 1;

    LOG(CERR) << "dfs " << map->get_node_name(now) << " my level: " << map->level[now];
    if(next_object<(int)object.size())
        cout << " next_object level " << object[next_object] << endl;
    else cout << " next_object level " << " ok\n";
    if(map->Gr[now].size()==0){
        if(next_object>=(int)object.size()){
            all_leave.emplace_back(now);
            is_good[now] = true;
            return true;
        }
        else return is_good[now] = false;
    }

    for(int i=0; i<(int)map->Gr[now].size(); i++){
        int to = map->Gr[now][i].to;
        float delay = map->Gr[now][i].delay;
        if(next_object>=(int)object.size()){
            is_good[now] = true;
            add_edge(get_kth_id(to), get_kth_id(now), delay);
            backward_build(to, next_object);
        }
        else if(mark[to] and map->level[to]==object[next_object]){
            if(backward_build(to, next_object+1)){
                is_good[now] = true;
                add_edge(get_kth_id(to), get_kth_id(now), delay);
            }
        }else if(map->level[to] > object[next_object]){
            if(backward_build(to, next_object)){
                is_good[now] = true;
                add_edge(get_kth_id(to), get_kth_id(now), delay);
            }
        }
        // to level is lower than now's obeject
    }
}

void Kth::single_dest_dfs(int v) {
	/* Find dist[v], successor[v] after calling this function */
	for (auto it=G[v].begin(); it!=G[v].end(); ++it) {
		const Edge &e = *it;
		int to = e.to;
		if (this->successor[to] == -1) single_dest_dfs(to);

		// Always choose "worst" path, that is, shortest dist, then smallest at
		float relax = this->dist[to] + e.delay;
		float new_at = this->at[to] + e.at_delay;
		if (this->successor[v] == -1 || this->dist[v] > relax) {
			this->dist[v] = relax;
			this->at[v] = new_at;
			this->successor[v] = to;
		}
		else if (this->dist[v] == relax && this->at[v] > new_at) {
			// Tie breaker
			this->dist[v] = relax;
			this->at[v] = new_at;
			this->successor[v] = to;
		}
	}
}

void Kth::build_single_dest_tree(int dest) {
	/* Initialization, successor also record visit or not, parent of root is root itself */
	this->successor.resize( this->G.size() );
	this->dist.resize( this->G.size() );
	std::fill(successor.begin(), successor.end(), -1);
	successor[dest] = dest;

	/* For each vertex v, find its distance to dest. */
	for (int i=0; i<(int)this->G.size(); i++) {
		this->single_dest_dfs(i);
	}
}

bool Kth::Prefix_node::compare(Prefix_node *n1, Prefix_node *n2) {
	// This function is for comparison in std::priority_queue
	// If we want to pick "worst" node, this comparison should return "is n1 better than n2"
	//
}
