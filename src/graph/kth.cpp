#include "kth.h"

Kth::Kth(BC_map *_map, CPPR *_cppr, Graph *_graph){
    map = _map;
    cppr = _cppr;
    graph = _graph;
    num_node = num_edge = 0;
    to_kth_id.resize( map->num_node );
    is_good.resize( map->num_node );
    has_kth_id.resize( map->num_node );
    vis.resize( map->num_node );
    mark.resize( map->num_node );
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

Kth::Edge::Edge(int id, int from, int to, float delay) {
	this->id = id;
	this->from = from;
	this->to = to;
	this->delay = delay;
	this->at_delay = delay;
}

void Kth::add_edge(int from, int to, float delay){
    G[from].emplace_back(this->num_edge++, from, to, delay);
}

void Kth::add_edge(int from, int to, float delay, float clock_delay){
    G[from].emplace_back(this->num_edge++, from, to, delay);
    G[from][ G[from].size()-1 ].at_delay = clock_delay;
}

// string Kth::get_node_name(int kth_id){
//     if(kth_id==source_kth) return "SuperSource";
//     if(kth_id==dest_kth)   return "SuperDest";
//     int map_id = to_bc_id[kth_id];
//     int graph_id = map->get_graph_id(map_id);
//     string name = map->graph->get_name(graph_id);
//     for(int i=0; i<(int)name.size(); i++){
//         if(name[i]==':') name[i] = '/';
//     }
//     return name;
// }

// void Kth::print(){
//     LOG(CERR) << num_node << " " << G.size() << '\n';
//     for(int i=0; i<num_node; i++){
//         for(auto &x : G[i]){
//             int to = x.to;
//             LOG(CERR) << get_node_name(i) << " -> " << get_node_name(to) << " " << x.delay << '\n';
//         }
//     }
// }

void Kth::mark_through(const vector<pair<Transition_Type,int>>& through){

    if(through.size()==0) return;

    vector<pair<int,int>> level; // level , node_id
    // vector<int> level;

    for(int i=0; i<(int)through.size(); i++){
        int map_id = through[i].second;
        int graph_id = map->get_graph_id(map_id);

        /*just setup check*/
        // map_id = map->get_index(EARLY, through[i].first, graph_id);
        // mark[map_id] = 1;
        // // level.emplace_back(map->level[map_id]);
        // level.emplace_back(map->level[map_id], map_id);
        // LOG(CERR) << map->get_node_name(map_id) << " " << map->level[map_id] << '\n';

        map_id = map->get_index(LATE, through[i].first, graph_id);
        mark[map_id] = 1;
        // level.emplace_back(map->level[map_id]);
        level.emplace_back(map->level[map_id], map_id);
        // LOG(CERR) << map->get_node_name(map_id) << " " << map->level[map_id] << '\n';

    }
    sort(level.begin(), level.end());

    // for(auto x:level){
    //     LOG(CERR) << map->get_node_name(x.second) << " level = " << x.first << '\n';
    // }
    if(level.size()) object.emplace_back(level[0].first);
    for(int i=1; i<(int)level.size(); i++){
        if(level[i].first != object.back()){
            object.emplace_back(level[i].first);
        }
    }
}

// src: in bc_map
void Kth::build_from_src(const vector<pair<Transition_Type,int>>& through, int src, bool specify){

    // LOG(CERR) << "builf from src\n";
    // super dest, every ff:d will connect to this node
    dest_kth = add_node(-1);
    source_kth = add_node(-1);
    mark_through(through);

    // source_kth to all likely source(early, late, rise, fall), and dfs them
    if(!specify){ // it has 4 src node in map
        // trun bc_map to graph id , then get bc_map id by specifing mode and transition
        int graph_id = map->get_graph_id(src);
        int map_id;
        float clock_at;

        const auto& src_node = map->graph->nodes[graph_id];
        if(src_node.type != CLOCK and src_node.type!=PRIMARY_IN){
            LOG(CERR) << "from node: " << map->graph->nodes[graph_id].name << " is not clock or PRIMARY_IN\n";
            return;
        }
        /* just setup check*/
        for(int i=0; i<1; i++){
            for(int j=0; j<2; j++){
                Mode mode = MODES[i];
                mode = LATE;
                Transition_Type type = TYPES[j];
                map_id = map->get_index(mode, type, graph_id);
                clock_at = src_node.at[mode][type];
                if(src_node.type == CLOCK and type != src_node.clk_edge) continue;

                /*  source at time became delay*/
                float delay = 0;
                if(mode==EARLY) delay = src_node.at[mode][type];
                else delay = -src_node.at[mode][type];

                add_edge(source_kth, get_kth_id(map_id), delay, clock_at);
                forward_build(map_id, 0);
            }
        }
    }
    else{
        // just specify Transition_Type, so it has two mode
        int graph_id = map->get_graph_id(src);
        Transition_Type type = map->get_graph_id_type(src);

        /* just setup check*/
        for(int i=0; i<1; i++){
            Mode mode = MODES[i];
            mode = LATE;
            float clock_at = map->graph->nodes[map->get_graph_id(src)].at[mode][type];
            int map_id = map->get_index(mode, type, graph_id);

            /* source at time became delay */
            float delay = 0;
            if(mode==EARLY) delay = map->graph->nodes[graph_id].at[mode][type];
            else delay = -map->graph->nodes[graph_id].at[mode][type];

            add_edge(source_kth, get_kth_id(map_id), delay, clock_at);
            forward_build(map_id, 0);
        }
    }

    // print();
    // build all ff:d to desk_kth
    for(auto x : all_leave){
        // int src_id = map->get_graph_id(src);
        int leaf_id = map->get_graph_id(x);
        Transition_Type leaf_type = map->get_graph_id_type(x);
        Mode mode = map->get_graph_id_mode(x);
        const auto& leaf_node = map->graph->nodes[ leaf_id ];
        float delay = 0;

        // LOG(CERR) << "cppr: " <<  map->get_node_name(src) << " " << map->get_node_name(x);
        // if(map->graph->nodes[src_id].is_clock and map->graph->nodes[leaf_id].node_type!=PRIMARY_OUT){
        //     int leaf_clk_id= map->graph->nodes[leaf_id].constrained_clk; // ff:d's clk
        //     if(leaf_clk_id==-1){
        //         LOG(ERROR) << "[kth][build_from_src] leaf'ck is -1, leaf: " << map->get_node_name(x) << '\n';
        //         LOG(CERR) << "[kth][build_from_src] leaf'ck is -1, leaf: " << map->get_node_name(x) << '\n';
        //         continue;
        //     }
        //     // ASSERT(leaf_clk_id!=-1);
        //
        //     Transition_Type src_clk_type = map->graph->nodes[src_id].clk_edge;       // clk trigger type
        //     Transition_Type leaf_clk_type = map->graph->nodes[leaf_clk_id].clk_edge;
        //
        //     delay += cppr->cppr_credit(mode, src_id, src_clk_type, leaf_clk_id, leaf_clk_type);
        //     LOG(CERR) << "(" << map->get_node_name( map->get_index(mode, leaf_clk_type, leaf_clk_id)) << ") " << delay << '\n';
        // }
        // else LOG(CERR) << " no cppr\n";


        if(mode==EARLY) delay += - leaf_node.rat[mode][leaf_type];
        else delay += leaf_node.rat[mode][leaf_type];

        // connect to dest_kth
        add_edge(get_kth_id(x), dest_kth, delay);
    }
}

bool Kth::forward_build(int now, int next_object){
    if(vis[now]) return is_good[now];
    vis[now] = 1;

    // LOG(CERR) << "dfs: " << map->get_node_name(now) << " my level: " << map->level[now] ;
    // if(next_object<(int)object.size())
    //     cout << " next_object level " << object[next_object] << '\n';
    // else cout << " next_object level " << " ok\n";
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

void Kth::build_from_dest(const vector<int>& dest){
    source_kth = add_node(-1);
    dest_kth   = add_node(-1);

    for(int i=0; i<(int)dest.size(); i++){
        int graph_id = map->get_graph_id( dest[i] );

        int map_id = dest[i];
        const auto& node = map->graph->nodes[graph_id];
        Mode mode = map->get_graph_id_mode(map_id);
        // cout << "dest = " << node.name << std::endl;
        ASSERT(map->graph->nodes[graph_id].exist==true);

        /* just setup check*/
        if(mode==EARLY){
            LOG(ERROR) << "[Kth][build_from_dest] early mode in dest " << '\n';
            continue;
        }
        Transition_Type type = map->get_graph_id_type(map_id);

        float delay = 0;
        if(mode==EARLY) delay = -node.rat[mode][type];
        else delay = node.rat[mode][type];

        // no through points
        backward_build(map_id, object.size());
        add_edge(get_kth_id(map_id), dest_kth, delay);
    }

    // source_kth to all src
    for(auto x:all_leave){
        Mode mode = map->get_graph_id_mode(x);
        Transition_Type type = map->get_graph_id_type(x);
        const auto& node = map->graph->nodes[ map->get_graph_id(x) ];
        // cout << "src: " << map->graph->get_name(map->get_graph_id(x)) << std::endl;
        /* just setup check*/
        if(mode==EARLY){
            LOG(ERROR) << "[Kth][build_from_dest] early mode in dest " << '\n';
            continue;
        }

        float delay = 0, at_delay = 0;
        if(mode==EARLY) delay = node.at[mode][type];
        else delay = -node.at[mode][type];

        at_delay = node.at[mode][type];
        add_edge(source_kth, get_kth_id(x), delay, at_delay);
    }
    // print();
}

void Kth::build_from_dest(const vector<pair<Transition_Type,int>>& through, int dest, bool specify){

    // LOG(CERR) << "builf from dest\n";
    mark_through(through);
    // hight level to low level
    reverse(object.begin(), object.end());

    source_kth = add_node(-1);
    dest_kth   = add_node(-1);

    // may dest(early, late, rise, fall) to dest_kth, and dfs them
    if(!specify){
        int graph_id = map->get_graph_id(dest);
        int map_id;

		/* just setup check*/
        for(int i=0; i<1; i++){
            for(int j=0; j<2; j++){
                Mode mode = MODES[i];
                mode = LATE;
                Transition_Type type = TYPES[j];

                map_id = map->get_index(mode, type, graph_id);
                // LOG(CERR) << "connnect " << get_node_name(dest_kth) << " " << map->get_node_name(map_id) << '\n';
                float delay = 0;
                if(mode==EARLY) delay = -map->graph->nodes[graph_id].rat[mode][type];
                else delay = map->graph->nodes[graph_id].rat[mode][type];

                add_edge(get_kth_id(map_id), dest_kth, delay);
                backward_build(map_id, 0);
            }
        }
    }else{
        // type is specified by map id
        Transition_Type type = map->get_graph_id_type(dest);
        int graph_id = map->get_graph_id(dest);
		/* just setup check*/
        for(int i=0; i<1; i++){
            Mode mode = MODES[i];
            mode = LATE;
            int map_id = map->get_index(mode, type, graph_id);
            // LOG(CERR) << "*connnect " << get_node_name(dest_kth) << " " << map->get_node_name(map_id) << '\n';

            float delay = 0;
            if(mode==EARLY) delay = -map->graph->nodes[graph_id].rat[mode][type];
            else delay = map->graph->nodes[graph_id].rat[mode][type];

            add_edge(get_kth_id(map_id), dest_kth, delay);
            backward_build(map_id, 0);
        }
    }

    // connect source_kth to all src
    for(auto x:all_leave){
        // int src_clk_id
        // int dest_id = map->get_graph_id(dest);
        int leaf_id = map->get_graph_id(x);
        Mode mode = map->get_graph_id_mode(x);
        Transition_Type type = map->get_graph_id_type(x);
        // const auto & dest_node = map->graph->nodes[dest_id];
        const auto & leaf_node = map->graph->nodes[leaf_id];
        // Transition_Type dest_type = map->get_graph_id_type(dest);
        float delay = 0, at_clock = 0;
		/* just setup check*/
        if(mode==EARLY){
            LOG(ERROR) << "[Kth][build_from_dest] early mode in dest " << '\n';
            continue;
        }
        // LOG(CERR) << "cppr: " <<  map->get_node_name(x) << " " << map->get_node_name(dest) ;
        // if(map->graph->nodes[dest_id].node_type!=PRIMARY_OUT and map->graph->nodes[leaf_id].is_clock){
        //     int dest_ck_id = map->graph->nodes[dest_id].constrained_clk;
        //     if(dest_ck_id==-1){
        //         LOG(CERR) << "[kth][build_from_dest] dest'ck is not primary out or register/D, dest: " << map->get_node_name(dest) << '\n';
        //         LOG(ERROR) << "[kth][build_from_dest] dest'ck is not primary out or register/D, dest: " << map->get_node_name(dest) << '\n';
        //         continue;
        //     }
        //     // ASSERT(dest_ck_id!=-1);
        //
        //     Transition_Type dest_ck_type = map->graph->nodes[dest_ck_id].clk_edge;
        //     Transition_Type leaf_ck_type = map->graph->nodes[leaf_id].clk_edge;
        //     delay += cppr->cppr_credit(mode, dest_ck_id, dest_ck_type, leaf_id, leaf_ck_type);
        //     LOG(CERR) << "(" << map->get_node_name( map->get_index(mode, dest_ck_type, dest_ck_id)) << ") " << delay << '\n';
        //
        // }
        // else LOG(CERR) << " no cppr\n";

        /* src at time became delay */
        if(mode==EARLY) delay += leaf_node.at[EARLY][type]; // -leaf_id.rat[EARLY][dest_type];
        else delay += -leaf_node.at[LATE][type];
        // else delay += dest_node.rat[LATE][dest_type] - dest_node.at[LATE][dest_type];

        at_clock = map->graph->nodes[leaf_id].at[mode][ map->get_graph_id_type(x) ];
        add_edge(source_kth, get_kth_id(x), delay, at_clock);
    }

    // print();
}

bool Kth::backward_build(int now, int next_object){
    if(vis[now]) return is_good[now];
    vis[now] = 1;

    // LOG(CERR) << "dfs " << map->get_node_name(now) << " my level: " << map->level[now];
    // if(next_object<(int)object.size())
    //     cout << " next_object level " << object[next_object] << '\n';
    // else cout << " next_object level " << " ok\n";
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
    return is_good[now];
}

void Kth::build_from_throgh(const vector<pair<Transition_Type,int>>& through){
    if(through.size()==0) return;
    mark_through(through);

    source_kth = add_node(-1);
    dest_kth = add_node(-1);

    int low_level = object[0];
    for(int i=0; i<(int)through.size(); i++){
        if( map->level[ through[i].second ] == low_level){
            int graph_id = map->get_graph_id( through[i].second );
            Transition_Type type = map->get_graph_id_type( through[i].second );
            // search this point can connect what point

            /* just setup check*/
            // forward_build(map->get_index(EARLY, type, graph_id), 1);
            // vis[map->get_index(EARLY, type, graph_id)] = 0;
            forward_build(map->get_index(LATE, type, graph_id), 1);
            vis[map->get_index(LATE, type, graph_id)] = 0;
        }
    }
    // can't go through all asked points
    if(all_leave.size()==0) return;

    // leaf to dest_kth
    for(auto x:all_leave){
        Mode mode = map->get_graph_id_mode(x);
        Transition_Type type = map->get_graph_id_type(x);
        const auto& node = map->graph->nodes[map->get_graph_id(x)];

        /* just setup check*/
        if(mode==EARLY){
            LOG(ERROR) << "[Kth][build_from_through] early mode in all_leave" << '\n';
            continue;
        }

        float delay = 0;
        if(mode==EARLY) delay = -node.rat[mode][type];
        else delay = node.rat[mode][type];

        add_edge(get_kth_id(x), dest_kth, delay);
    }
    // for(auto x:all_leave) LOG(CERR) << " can go to " << map->get_node_name(x) << '\n';

    all_leave.clear();
    for(int i=0; i<(int)through.size(); i++){
        if(map->level[through[i].second]==low_level){
            int graph_id = map->get_graph_id( through[i].second );
            Transition_Type type = map->get_graph_id_type( through[i].second );
            // just backward search which point can connenct this points
            /* just setup check*/
            // backward_build(map->get_index(EARLY, type, graph_id), object.size());
            backward_build(map->get_index(LATE, type, graph_id), object.size());
        }
    }
    // source_kth to leaf
    for(auto x : all_leave){
        Mode mode = map->get_graph_id_mode(x);
        Transition_Type type = map->get_graph_id_type(x);
        const auto& node = map->graph->nodes[map->get_graph_id(x)];

/* just setup check*/
        if(mode==EARLY){
            LOG(ERROR) << "[Kth][build_from_through] early mode in all_leave" << '\n';
            continue;
        }
        float delay = 0;
        if(mode==EARLY) delay = node.at[mode][type];
        else delay = -node.at[mode][type];


        add_edge(source_kth, get_kth_id(x), delay, node.at[mode][type]);
    }
    // for(auto x:all_leave) LOG(CERR) << " cant go from " << map->get_node_name(x) << '\n';
}
// *********************************************
// *       kth algorithm implementation        *
// *********************************************

void Kth::single_dest_dfs(int v) {
	// Find dist[v], successor[v] after calling this function
	// successor == -1: not visited, successor == -2: not reachable
	// cout<<"DFS "<<v<<'\n'<<std::flush;
    // cout << "single " << get_node_name(v) << '\n';
	for (auto it=G[v].begin(); it!=G[v].end(); ++it) {
		const Edge &e = *it;
		int to = e.to;
		if (this->successor[to] == NOT_VISITED) single_dest_dfs(to);
		if (this->successor[to] == NOT_REACHABLE) continue;

		// Always choose "worst" path, that is, shortest dist, then smallest at
		float relax = this->dist[to] + e.delay;
		float new_at = this->at_dist[to] + e.at_delay;
		// cout<<"Use "<<relax<<" relax "<<v<<'\n';
		if (this->successor[v] == NOT_VISITED || this->dist[v] > relax) {
			// cout<<"RELAX\n";
			this->dist[v] = relax;
			this->at_dist[v] = new_at;
			this->successor[v] = to;
			this->use_edge[v] = e.id;
		}
		else if (this->dist[v] == relax && this->at_dist[v] > new_at) {
			// Tie breaker
			// cout<<"TIE RELAX\n";
			this->dist[v] = relax;
			this->at_dist[v] = new_at;
			this->successor[v] = to;
			this->use_edge[v] = e.id;
		}
	}

	if (this->successor[v] == NOT_VISITED) this->successor[v] = NOT_REACHABLE;
}

bool Kth::build_single_dest_tree(int dest) {
	/* Initialization, successor also record visit or not, parent of root is root itself */
	this->successor.resize( this->G.size() );
	this->dist.resize( this->G.size() );
	this->at_dist.resize( this->G.size() );
	this->use_edge.resize( this->G.size() );
	std::fill(successor.begin(), successor.end(), NOT_VISITED);
	this->successor[dest] = dest;
	this->dist[dest] = 0.0;
	this->at_dist[dest] = 0.0;
	this->use_edge[dest] = -1;

	/* For each vertex v, find its distance to dest. */
	for (int i=0; i<(int)this->G.size(); i++) {
		if (this->successor[i] == NOT_VISITED) {
			this->single_dest_dfs(i);
		}
		// cout<<"SUCC "<<successor[i]<<'\n';
	}

	/* Return false if dest isn't reachable. */
	if (this->successor[source_kth] == NOT_REACHABLE) return false;

	/* For each edge e, compute its delta */
	for (int i=0; i < (int)G.size(); i++) {
		for (auto it = G[i].begin(); it!=G[i].end(); ++it) {
			Edge &e = *it;
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
	int v = path->eptr==NULL ? source_kth : path->eptr->to;
	while (v != this->dest_kth && v != NOT_VISITED && v != NOT_REACHABLE) {
		for (auto it = G[v].begin(); it != G[v].end(); ++it) {
			Edge &e = *it;
			if (successor[e.to] == NOT_REACHABLE || e.id == this->use_edge[v]) continue;
			Prefix_node *next_path = new Prefix_node(path, &e); // IMPORTANT, one should delete this after pop.
//			cout << "Use " << path << " to extend "<<next_path<<" eptr="<<path->eptr<<'\n'<<std::flush;
			this->pq.push(next_path);
		}

		v = successor[v];
	}
}

void Kth::get_explicit_path_helper(Path *exp_path, const Prefix_node *imp_path, int dest) {
	// Recursive subroutine of get_explicit_path
	// *** Assume dest havn't been pushed into path
//	cout<<"NOW "<<imp_path<<" "<<imp_path->eptr<<'\n'<<std::flush;
	int sz = exp_path->path.size(), dsz = exp_path->delay.size(), v;
	if (imp_path->eptr == NULL) {
		// Root of prefix tree
		v = this->source_kth;
	}
	else {
		// From where last sidetrack points to
		v = imp_path->eptr->to;
		exp_path->delay.emplace_back(imp_path->eptr->delay);
	}

	// Go through all vertices from v to dest
	while (v != dest && v != NOT_VISITED && v != NOT_REACHABLE) {
		exp_path->path.emplace_back(v);
		exp_path->delay.emplace_back(dist[ v ] - this->dist[ this->successor[v] ]);
		v = this->successor[v];
	}
	// cout<<"PUSH "<<v<<'\n'<<std::flush;
	exp_path->path.emplace_back(v);
	std::reverse(exp_path->path.begin() + sz, exp_path->path.end());
	std::reverse(exp_path->delay.begin() + dsz, exp_path->delay.end());

	if (imp_path->eptr != NULL) {
		get_explicit_path_helper(exp_path, imp_path->parent, imp_path->eptr->from);
	}
}

void Kth::get_explicit_path(Path *exp_path, const Prefix_node *imp_path) {
	// Recover path from implicit representation to explicit representaion
	// Store in exp_path, path will be reverse order.
	// IMPORTANT, the paths are in BC id.
	exp_path->dist = this->dist[ this->source_kth ] + imp_path->delta;
	exp_path->path.clear();
	// cout<<"Begin find exp path\n"<<std::flush;
	get_explicit_path_helper(exp_path, imp_path, dest_kth);
	exp_path->mark.resize(exp_path->path.size());
	for (int i=0; i<(int)exp_path->path.size(); i++) {
		int v = this->to_bc_id[ exp_path->path[i] ];
		exp_path->path[i] = v;
		if (v == -1) continue; // SuperSrc/Dest has BC id == 1
		if (this->mark[v]) {
			exp_path->mark[i] = true;
		}
		else {
			exp_path->mark[i] = false;
		}
	}
	// cout<<"FIND DONE\n"<<std::flush;
}

void Kth::k_shortest_path(int k, vector<Path> &container) {
	// cout<<"BUILD TREE START\n"<<std::flush;
	if (!this->build_single_dest_tree(this->dest_kth)) {
		container.clear();
		return;
	}
	// cout<<"BUILD TREE DONE\n"<<std::flush;

	container.resize(k);

	// This loop and extend can be optimized by branch and bound method
	Prefix_node *root = new Prefix_node(); // Empty set represent SP itself
	this->pq.push(root);
	for (int i=0; i<k; i++) {
		// cout<<"Extract path"<<i<<'\n'<<std::flush;
		if (pq.empty()) {
			container.resize(i);
			break;
		}
		Prefix_node *next_path = pq.top();
		pq.pop();
		get_explicit_path(&container[i], next_path);
		// container[i].print();
		if (i+1 < k) this->extend(next_path);
		this->trash_can.emplace_back(next_path);
	}

	// clean nodes
	for (auto it = this->trash_can.begin(); it != this->trash_can.end(); ++it) {
		delete *it;
	}
	trash_can.clear();
	while (!pq.empty()) {
		Prefix_node *next_path = pq.top();
		pq.pop();
		delete next_path;
	}
}

// void Kth::print_path(const Path& p) {
//     /* Test function */
//     float total = 0, delay = 0;
//     int width = 10;
//     vector<int> path = p.path;
//     vector<float> pdelay = p.delay;
//     reverse(path.begin(), path.end());
//     reverse(pdelay.begin(), pdelay.end());
//     LOG(CERR) << std::setw(28) << get_node_name(path[0]) << std::setw(width) << 0 << std::setw(width) << 0 << '\n';
// 	for (int i=1; i<(int)path.size(); i++) {
//         delay = pdelay[i-1];
//         total += delay;
//         LOG(CERR) << std::setw(28) << get_node_name(path[i]) << std::setw(width) << std::fixed << std::setprecision(3)
//         << delay << std::setw(width) << std::fixed << std::setprecision(3) << total << '\n';
// 	}
// 	LOG(CERR) <<  "Length: " << p.dist << '\n' << '\n';
// }

int Kth::get_type(int index) {
	// Return 0 if rising, 1 if falling
	// Use BC id
	return index&1;
}

void Kth::clear(){
    to_bc_id.clear();
    to_kth_id.clear();
    is_good.clear();
    has_kth_id.clear();
    vis.clear();
    G.clear();
    mark.clear();
    object.clear();
    all_leave.clear();
    dist.clear();
    at_dist.clear();
    successor.clear();
    use_edge.clear();
    while(!pq.empty()) pq.pop();
    num_node = 0;
    num_edge = 0;

    to_kth_id.resize( map->num_node );
    is_good.resize( map->num_node );
    has_kth_id.resize( map->num_node );
    vis.resize( map->num_node );
    mark.resize( map->num_node );
}

void Path::print_name(ostream &fout, const string &name) const {
    for(int i=0; i<(int)name.size(); i++){
        if (name[i]==':') fout << '/';
        else fout << name[i];
    }
}

// void Path::check_condensed_pin(ostream &fout, Graph *graph, int from_bc_id, int to_bc_id, float total) const {
//     BC_map *bc = graph->get_bc_map();
//     int width = 8, from_gid = bc->get_graph_id(from_bc_id), to_gid = bc->get_graph_id(to_bc_id);
//     auto it = graph->adj[from_gid].find(to_gid);
//     ASSERT(it != graph->adj[from_gid].end());
//     Graph::Edge *eptr = it->second;
//     if (eptr->through == -1) return;
//     int gid = eptr->through;
//
//     const char *tab = "      ", *type_ch[2] = {"^   ", "v   "};
//     fout << tab << std::left << std::fixed << std::setprecision(OUTPUT_PRECISION) << std::setw(width) << 0.0 << "  ";
//     fout        << std::left << std::fixed << std::setprecision(OUTPUT_PRECISION) << std::setw(width) << total << "   ";
//     fout << type_ch[bc->get_graph_id_type(from_bc_id)] << "  ";
//     print_name(fout, graph->nodes[gid].name);
//     fout << '\n';
// }

void Path::output(ostream &fout, Graph *graph) const {
	// Paths are using BC id
    const vector<int> &path = this->path;
    const vector<float> &delay = this->delay;
    const vector<bool> &mark = this->mark;
    BC_map *bc = graph->get_bc_map();

    int width = 8, n = path.size();
    float rat = delay[0], slack = this->dist, at = rat - slack, total = delay[n-2];
    if(delay[n-2]!=0) total *= -1;
    const char *tab = "      ", *spline = "----------------------------------------", *type_ch[2] = {"^   ", "v   "};

    // path[0] is SuperDest, path[n-1] is SuperSrc
    if (path.empty()) return;
    fout << '\n';
	fout << "Endpoint:   ";
    print_name(fout, graph->nodes[bc->get_graph_id(path[1])].name);
    fout << '\n';
	fout << "Beginpoint: ";
    print_name(fout, graph->nodes[bc->get_graph_id(path[n-2])].name);
    fout << '\n';

	fout << "= Required Time              " << std::fixed << std::setw(7) << std::setprecision(OUTPUT_PRECISION) << rat    << '\n';
	fout << "- Arrival Time               " << std::fixed << std::setw(7) << std::setprecision(OUTPUT_PRECISION) << at     << '\n';
	fout << "= Slack Time                 " << std::fixed << std::setw(7) << std::setprecision(OUTPUT_PRECISION) << rat-at << '\n';
	fout << tab << spline << '\n';
	fout << tab << "Delay     Arrival    Edge  Pin" << '\n';
	fout << tab << "          Time" << '\n';
	fout << tab << spline << '\n';

    /* Output first pin (special case, no need to print delay) */
	fout << tab << "-         " << std::left << std::fixed << std::setprecision(OUTPUT_PRECISION) << std::setw(width) << total
    << "   " << type_ch[bc->get_graph_id_type(path[n-2])] << "  ";
    print_name(fout, graph->nodes[bc->get_graph_id(path[n-2])].name);
    // if(mark[n-2]) fout << " ->";
    fout << '\n';
    // if (n-3 >= 1) {
    //     check_condensed_pin(fout, graph, path[n-2], path[n-3], total);
    // }

    /* Output remaining */
	for (int i = n-3; i>=1; i--) {
		total -= delay[i]; // Delay is negative
        float del = delay[i];
        if(del!=0) del*=-1;
		fout << tab << std::left << std::fixed << std::setprecision(OUTPUT_PRECISION) << std::setw(width) << -del << "  ";
		fout        << std::left << std::fixed << std::setprecision(OUTPUT_PRECISION) << std::setw(width) << total << "   ";
		fout << type_ch[bc->get_graph_id_type(path[i])] << "  ";
        print_name(fout, graph->nodes[bc->get_graph_id(path[i])].name);
        // if(mark[i] ) fout << " ->";
        fout << '\n';
        // if (i-1 >= 1) {
        //     check_condensed_pin(fout, graph, path[i], path[i-1], total);
        // }
	}
	fout << tab << spline << '\n' << '\n';
}

void Path::print() {
	LOG(CERR) << "Path:\nEnd at " << this->path[0] << '\n';
	for (int i=1; i<(int)path.size(); i++) {
		LOG(CERR) << "delay " << std::setw(6) << delay[i-1] << " from " << this->path[i] << '\n';
	}
	LOG(CERR) << "Length: " << this->dist << "\n\n";
}

// vector<vector<Kth::Edge>>& Kth::getG() {
// 	/* Just for unit test */
// 	return this->G;
// }

void Kth::set_st(int s, int t) {
	this->source_kth = s;
	this->dest_kth = t;
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

Kth::Prefix_node::Prefix_node() {
	// An empty set is SP itself
	this->parent = NULL;
	this->eptr = NULL;
	this->delta = 0.0;
	this->at_delta = 0.0;
}

bool Kth::Prefix_node::Compare::operator()(const Prefix_node *n1, const Prefix_node *n2) {
	// This function is for comparison in std::priority_queue
	// If we want to pick "worst" node, this comparison should return "is n1 better than n2"
	return (n1->delta > n2->delta) || ( n1->delta == n2->delta && n1->at_delta > n2->at_delta );
}

// ----------------- For Testing ---------------------------

#ifdef TEST_KTH

int main() {
	// Let's ICPC!
	int V, E, s, t, k;
	Kth algo;
	auto &G = algo.getG();
	freopen("unit_test/kth.txt", "r", stdin);
	while (cin>>V>>E>>s>>t>>k) {
		G.resize(V);
		algo.set_st(s,t);
		for (int i=0; i<V; i++) G[i].clear();
		for (int i=0; i<E; i++) {
			int fr, to;
			float w;
			cin>>fr>>to>>w;
			algo.add_edge(fr, to, w, 1);
		}

		vector<Path> vp;
		algo.k_shortest_path(k, vp);
		cout<<"------------------------------\n\n"<<vp.size()<<" paths found\n"<<std::flush;
		for (int i=0; i<(int)vp.size(); i++) {
			this->output_path(cout, vp[i])
		}
	}
}

#endif
