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
	for (int i=0; i<this->G.size; i++) {
		this->single_dest_dfs(i);
	}
}

bool Kth::Prefix_node::compare(Prefix_node *n1, Prefix_node *n2) {
	// This function is for comparison in std::priority_queue
	// If we want to pick "worst" node, this comparison should return "is n1 better than n2"
	// 
}
