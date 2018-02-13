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

    // super dest, every ff:d will connect to this node, and the weight is cppr(src, ff:d) + slack(ff:d)
    dest_kth = add_node(-1);
    source_kth = add_node(-1);

    mark_through(through);

    // source_kth to all
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
        int src_clk_id = map->get_graph_id( src ); // clk id in graph
        int data_clk_id = map->graph->nodes[ map->get_graph_id(x) ].constrained_clk; // ff:d's clk
        Mode mode = map->get_graph_id_mode(x);
        Transition_Type type = map->get_graph_id_type(x);

        Transition_Type clk_mode = map->graph->nodes[src_clk_id].clk_edge;       // clk trigger type
        Transition_Type data_clk_mode = map->graph->nodes[data_clk_id].clk_edge;

        float delay = cppr->cppr_credit(mode, src_clk_id, clk_mode, data_clk_id, data_clk_mode);
        const auto& node = map->graph->nodes[ map->get_graph_id(x) ];
        // cppr plus slack = Woffest
        if(mode==EARLY) delay += node.at[mode][type] - node.rat[mode][type];
        else delay +=  node.rat[mode][type] - node.at[mode][type];

        // connect to dest_kth
        add_edge(get_kth_id(x), dest_kth, delay);
    }
    print();
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

bool Kth::forward_build(int now, int next_object){
    if(vis[now]) return is_good[now];
    vis[now] = 1;

    LOG(CERR) << "dfs: " << map->get_node_name(now) << " my level: " << map->level[now]
    << " next level: " << object[next_object] << endl;
    if(map->G[now].size()==0){ // at primary out or ff:d
        if(next_object==(int)object.size()){
            all_leave.emplace_back(now);
            return true;
        }
        else return false;
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

    mark_through(through);
    // backward_build(dest, 0);
}

// *********************************************
// *       kth algorithm implementation        *
// *********************************************

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

bool Kth::build_single_dest_tree(int dest) {
	/* Initialization, successor also record visit or not, parent of root is root itself */
	this->successor.resize( this->G.size() );
	this->dist.resize( this->G.size() );
	std::fill(successor.begin(), successor.end(), -1);
	successor[dest] = dest;

	/* For each vertex v, find its distance to dest. */
	for (int i=0; i<(int)this->G.size(); i++) {
		this->single_dest_dfs(i);
	}
	
	/* Return false if dest isn't reachable. */
	bool reach = false;
	for (int i=0; i<G.size(); i++) {
		if (successor[i] == dest) {
			reach = true;
			break;
		}
	}
	if (!reach) return false;
	
	/* For each edge e, compute its delta */
	for (int i=0; i<G.size(); i++) {
		for (auto it = G[i].begin(); it!=G[i].end(); ++it) {
			const Edge &e = *it;
			e.delta = e.delay + this->dist[e.to] - this->dist[e.from];
			e.at_delta = e.at_delay + this->at_dist[e.to] - this->at_dist[e.from];
		}
	}
	
	return true;
}

void Kth::extend(Prefix_node *path) {
	// From "path", extend its childs
	// A child can only be obtained by adding a sidetrack edge, in which its "from" on the path after last sidetrack of this path
	// All its children are longer than "path"
	
	// For every v from head of last sidetrack to t.
	int v = path->eptr->to;
	while (v != this->kth_dest) {
		for (auto it = G[v].begin(); it != G[v].end(); ++it) {
			const Edge &e = *it;
			if (e.to == this->successor[v]) continue;
			Prefix_node *next_path = new Prefix_node(path, &e); // IMPORTANT, need a mechanism to free these guys.
			this->pq.push(next_path);
		}
		
		v = successor[v];
	}
}

void Kth::get_explicit_path_helper(Path *exp_path, const Prefix_node *imp_path, int dest) {
	// Recursive subroutine of get_explicit_path
	// *** Assume dest havn't been pushed into path
	int sz = exp_path->path.size(), v;
	if (imp_path->parent->eptr == NULL) {
		// Root of prefix tree
		v = this->kth_src;
	}
	else {
		// from where last sidetrack points to
		v = imp_path->eptr->to;
	}
	
	// Go through all vertices from v to dest
	while (v != dest) {
		exp_path.push_back(v);
		v = this->successor[v];
	}
	exp_path.push_back(v);
	std::reverse(exp_path->path.begin() + sz, exp_path->path.end);
	
	if (imp_path->parent->eptr != NULL) {
		exp_path.push_back(imp_path->eptr->to); // Don't push "eptr->from" because our assumption.
		get_explicit_path_helper(exp_path, imp_path->parent, imp_path->eptr->from);
	}
}

void Kth::get_explicit_path(Path *exp_path, const Prefix_node *imp_path) {
	// Recover path from implicit representation to explicit representaion
	// Store in exp_path, path will be reverse order.
	exp_path->delay = this->dist[ this->kth_dest ] - imp_path->delta;
	exp_path->path.clear();
	get_explicit_path_helper(exp_path, imp_path, kth_dest);
}

// *************************************
// *            Prefix_node            *
// *************************************

Kth::Prefix_node::Prefix_node(Prefix_node *p, Edge *e) {
	this->parent = p;
	this->eptr = e;
	this->delta = p->delta + e->delta;
	this->at_delta = p->at_delta + e->at_delta;
}

bool Kth::Prefix_node::compare(Prefix_node *n1, Prefix_node *n2) {
	// This function is for comparison in std::priority_queue
	// If we want to pick "worst" node, this comparison should return "is n1 better than n2"
	return (n1->delta > n2->delta) || ( n1->delta == n2->delta && n1->at_delta > n2->at_delta );
}

// ----------------- For Testing ---------------------------

#ifdef TEST_KTH

int main() {
	
}

#endif
