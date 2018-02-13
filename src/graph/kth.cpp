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
	for (int i=0; i<this->G.size; i++) {
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
