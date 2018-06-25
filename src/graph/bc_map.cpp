#include "bc_map.h"

BC_map::BC_map(Graph* graph, CPPR* cppr){
    this->graph = graph;
	this->cppr = cppr;
	this->cache_timestamp = 0;

	for(int i=0; i<NUM_THREAD; i++) {
		kths[i] = new Kth(this, cppr, graph);
	}
}

BC_map::~BC_map() {
	for(int i=0; i<NUM_THREAD; i++) {
		delete kths[i];
	}

	for(auto& p: caches) {
		delete p;
	}
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
	Edge *e = new Edge(from, to, delay);
	Edge *rev = new Edge(to, from, delay);

    G[from].emplace_back(e);
    Gr[to].emplace_back(rev);
	//e->rev_edge = rev;
	//rev->rev_edge = e;
	e->id = edge_counter;
	rev->id = edge_counter;
	edge_counter += 1;

	es.push_back(e);
    in_degree[to]++;
}

void BC_map::build(){
    // add node
    // every graph node has 2 nodes in map
    num_node = 0;
	edge_counter = 0;
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
	// is_valid.resize(num_node);
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
            in_degree[e->to]--;
            if(in_degree[e->to]==0) q->push(e->to);
            level[e->to] = max(level[e->to], level[x]+1);
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

int BC_map::cal_cache_difference(Cache* cache, const vector<int>& through, const vector<int>& disable) {

	int num_search = 0;
	for(auto& x:cache->through) {
		if(is_through[x] == 0) num_search++;
	}
	return num_search;
	//const auto& pre_through = cache->get_through();

	//int j_th = pre_through.size()-1;
	//int i_th = through.size()-1;
	//vector<int> now_node;

	//auto same_pin = [&](int x,int y)->bool {
		//return (level[x] == level[y] and get_graph_id(x) == get_graph_id(y));
	//};

	//auto get_now_node = [&](int now_level)->bool {

		//now_node.clear();
		//// get the now_node
		//while(i_th >=0 and level[through[i_th]] == now_level) now_node.push_back(through[i_th--]);

		//// check whether all the now_node are the same pins
		//if(now_node.size() > 4) return false;
		//for(auto& x:now_node){
			//for(auto& y:now_node) {
				//// we can't find a valid path going through the nodes having same the level but different pin.
				//if(!same_pin(x, y)) {
					//return false;
				//}
			//}
		//}
		//return true;
	//};

	//bool now_need_search_fout = false;
	//bool next_need_search_fout = false;
	//int num_search_fout = 0;

	//next_level.push_back(-1);
	//for(int II=next_level.size()-2; II>=0; II--){
		//int now_level = next_level[II];

		//if(!get_now_node(now_level)) {
			//return std::numeric_limits<int>::max();
		//}

		//now_need_search_fout = next_need_search_fout;
		//next_need_search_fout = false;

		//while(j_th >=0 and level[ pre_through[j_th] ] > now_level) {
			//now_need_search_fout = true;
			//j_th--;
		//}

		//if(j_th >=0 and level[pre_through[j_th]] == now_level) {
			//vector<int> pre_node;
			//while(j_th >=0 and level[pre_through[j_th]] == now_level) pre_node.push_back(pre_through[j_th--]);

			//ASSERT(now_node.size()!=0);
			//ASSERT(pre_node.size()!=0);
			//if(same_pin(now_node[0], pre_node[0])) {
				//for(auto& y:now_node) {
					//for(auto& x:pre_node) {
						//if(y!=x) next_need_search_fout = true;
					//}
					//if(next_need_search_fout) break;
				//}
			//}
			//else {
				//now_need_search_fout = true;
				//next_need_search_fout = true;
			//}

		//}
		//else{
			//now_need_search_fout = true;
			//next_need_search_fout = true;
		//}

		//if(now_need_search_fout) {
			//for(auto& x:now_node){
				//search_fout_layer(x, next_level[II+1]);
				//num_search_fout++;
			//}
		//}
		//now_need_search_fout = false;
	//}

	//if(now_need_search_fout or next_need_search_fout) {
		//for(auto& x:now_node){
			//num_search_fout++;
			//search_fin(x);
		//}
	//}

	//return num_search_fout;
}

void BC_map::choose_cache(const vector<int>& through, const vector<int>& disable) {

	int min_val = through.size();
	Cache* best = nullptr;
	for(auto& cac: caches) {
		int diff = cal_cache_difference(cac, through, disable);
		// int diff = cac->cal_difference(through, disable);
		if(diff < min_val) {
			min_val = diff;
			best = cac;
		}
	}

	// we can't find a similiar cache for this query
	if(best == nullptr) {
		if(caches.size() < NUM_CACHE ) {
			//current_cache = new Cache(through, disable, this);
			current_cache = new Cache(this, new Kth(this, cppr, graph));
			caches.push_back( current_cache );
		}
		else {
			// choose the oldest cache and clear it
			min_val = 0;
			best = nullptr;
			for(size_t i=0; i<caches.size(); i++) {
				if(caches[i]->get_timestamp() > min_val) {
					min_val = caches[i]->get_timestamp();
					best =  caches[i];
				}
			}
			current_cache = best;
			current_cache->clear();
		}
	}
	else current_cache = best;

	// LOG(CERR) << "current_cache size = " << caches.size() << "\n";
	// LOG(CERR) << "Current_cache: " << current_cache << "\n";
	// LOG(CERR) << "cache through: ";
	// for(auto& x:current_cache->get_through())
	// 	LOG(CERR) << get_node_name(x) << "(" << level[x] << "), ";
	//  LOG(CERR) << '\n';

	current_cache->set_timestamp( cache_timestamp++ );
}

void BC_map::k_shortest_path(vector<int>& through, const vector<int>& disable, int k, vector<Path*>& ans, bool cppr_on)
{
    cppr_on = true;
/* get next_level */
	mark_point(through, disable);
    Logger::start();
	choose_cache(through, disable);
    Logger::stop( "choose cache ");

    Logger::start();
/* mark searching space*/
	if(through.size()) search_modify(through, disable);
	else search_all();

	current_cache->set_through(through);
	current_cache->set_disable(disable);
    Logger::stop( "search space ");

	// for(int i=0; i<num_node; i++) {
	// 	if(current_cache->get_vert_valid(i)) {
	// 		LOG(CERR) << this->get_node_name(i) << " valid\n";
	// 	}
	// }
	// LOG(CERR)<< "kth dest = " << kth_dest.size() << "\n";
	// for(auto& x:kth_dest) LOG(CERR) << get_node_name(x) << ", "; LOG(CERR)<< "\n";
	// LOG(CERR)<< "kth start= " << kth_start.size() << "\n";
	// for(auto& x:kth_start) LOG(CERR) << get_node_name(x) << ", "; LOG(CERR)<< "\n";
    //
	// LOG(CERR)<< "edge valid: ";
	// LOG(CERR)<< current_cache->edge_valid.size() << "\n";
	// for(auto& p :current_cache->edge_valid) {
	// 	int eid = p.first;
	// 	LOG(CERR) << get_node_name(es[eid]->from) << " => " << get_node_name(es[eid]->to) << " " << p.second << "\n";
	// }

/* query kth */
    Logger::start();
	if(cppr_on) {
		if(kth_start.size()>0 and  kth_start.size() < kth_dest.size() ) {
			auto f = [](Kth* kth, int src, int k, vector<Path*>& container) {
				kth->KSP_from_source(src, k, container);
			};
			do_kth(kth_start, k, f, ans);
		}
		else {
			auto f = [](Kth* kth, int dest, int k, vector<Path*>& container) {
				 kth->KSP_to_destination(dest, k, container);
			};
			do_kth(kth_dest, k, f, ans);
		}
	}
	else { // no cppr
		current_cache->kth->clear(); // may do some modification instead of clearing it
		current_cache->kth->KSP_without_CPPR(kth_dest, k, ans);
	}
    Logger::stop("do kth ");

	//cout << "vert size:";
	//cout << current_cache->vert_valid.size() << '\n';
	//cout << "edge size:";
	//cout << current_cache->edge_valid.size() << '\n';
	//cout << " ok!\n";
}

void BC_map::mark_point(vector<int>& through, const vector<int>& disable) {

	std::sort(through.begin(), through.end(), [this](int a,int b) {
			if(this->level[a] != this->level[b]) return this->level[a] < this->level[b];
			else { // if the two nodes has the level, then we compare their unique graph id
				int graph_a = this->get_graph_id(a);
				int graph_b = this->get_graph_id(b);
				return graph_a < graph_b;
			}
	});

	// std::fill(is_valid.begin(), is_valid.end(), 0);
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

// return true if we can find a valid path
bool BC_map::search_modify(const vector<int>& through, const vector<int>& disable) {

	kth_start.clear(); // FF:clk or Pin
	kth_dest.clear();  // FF:D or Pout
	std::fill(vis.begin(), vis.end(), 0);

	const auto& pre_through = current_cache->get_through();

	int j_th = pre_through.size()-1;
	//int j_di = pre_disable.size()-1;
	int i_th = through.size()-1;
	int i_di = disable.size()-1;
	vector<int> now_node;

	auto same_pin = [&](int x,int y)->bool {
		return (level[x] == level[y] and get_graph_id(x) == get_graph_id(y));
	};

	auto disable_fout = [&](int x) {
		// LOG(CERR) << " Disable " << get_node_name(x) << '\n';
		for(auto& epr: G[x]) {
			this->current_cache->set_edge_valid(epr->id, false);
		}
		for(auto& epr: Gr[x]) {
			this->current_cache->set_edge_valid(epr->id, false);
		}
		current_cache->set_vert_valid(x, false);
	};

	auto get_now_node = [&](int now_level)->bool {

		now_node.clear();
		// get the now_node
		while(i_th >=0 and level[through[i_th]] == now_level) now_node.push_back(through[i_th--]);

		// check whether all the now_node are the same pins
		if(now_node.size() > 4) return false;
		for(auto& x:now_node){
			for(auto& y:now_node) {
				// we can't find a valid path going through the nodes having same the level but different pin.
				if(!same_pin(x, y)) {
					return false;
				}
			}
		}
		return true;
	};

	bool now_need_search_fout = false;
	bool next_need_search_fout = false;

	// LOG(CERR) << "through: ";
	// for(auto& x:through) LOG(CERR) << get_node_name(x) << ", ";  LOG(CERR)<< "\n";

    int cnt_search_out = 0, cnt_search_in = 0;
	next_level.push_back(-1);
	for(int II=next_level.size()-2; II>=0; II--){
		int now_level = next_level[II];

		if(!get_now_node(now_level)) return false;

		// LOG(CERR) << "i = " << II << " now level = " << now_level << "\n";
		// LOG(CERR) << " => now node\n";
		// for(auto& x: now_node) LOG(CERR) << get_node_name(x) << ", "; LOG(CERR) << "\n";

		now_need_search_fout = next_need_search_fout;
		next_need_search_fout = false;

		while(j_th >=0 and level[ pre_through[j_th] ] > now_level) {
			disable_fout(pre_through[j_th]);
			now_need_search_fout = true;
			j_th--;
		}

		while(i_di >=0 and level[disable[i_di]] >= now_level) {
			disable_fout(disable[i_di]);
			i_di--;
		}

		if(j_th >=0 and level[pre_through[j_th]] == now_level) {
			vector<int> pre_node;
			while(j_th >=0 and level[pre_through[j_th]] == now_level) pre_node.push_back(pre_through[j_th--]);

			ASSERT(now_node.size()!=0);
			ASSERT(pre_node.size()!=0);
			if(same_pin(now_node[0], pre_node[0])) {
				for(auto& x:pre_node) {
					bool find = false;
					for(auto& y:now_node) {
						if(y == x) find = true;
					}
					if(!find) disable_fout(x);
				}

				for(auto& y:now_node) {
					for(auto& x:pre_node) {
						if(y!=x) next_need_search_fout = now_need_search_fout = true;
					}
				}
			}
			else {
				for(auto& x:pre_node) disable_fout(x);
				now_need_search_fout = true;
				next_need_search_fout = true;
			}

		}
		else{
			now_need_search_fout = true;
			next_need_search_fout = true;
		}

		if(now_need_search_fout) {
			for(auto& x:now_node){
				// LOG(CERR) << " -----> search fout " << get_node_name(x) << " to " << next_level[II+1] << "\n";
				search_fout_layer(x, next_level[II+1]);
                cnt_search_out++;
			}
		}
		else if(II==(int)next_level.size()-2) {
			for(auto& x: now_node) {
				if(G[x].size()==0) kth_dest.push_back(x);
			}
		}
		now_need_search_fout = false;
	}
	// LOG(CERR)<< "====\n";

	if(now_need_search_fout or next_need_search_fout) {
		for(auto& x:now_node){
			// LOG(CERR) << " -----> search fin " << get_node_name(x) << "\n";
            vis[x] = 0;
			search_fin(x);
            cnt_search_in++;
		}
	}
	else {
		for(auto& x:now_node) {
			if(Gr[x].size()==0) kth_start.push_back(x);
		}
	}

    Logger::add_record("search_out", cnt_search_out);
    Logger::add_record("search_in", cnt_search_in);
	return true;
}

bool BC_map::search_fout_layer(int x,int target_level) {

    // Logger::add_record("touch points ", 1);
	if(vis[x]) return current_cache->get_vert_valid(x);
	vis[x] = true;

	if(target_level==-1 and G[x].size()==0)  // ff:d or POUT
	{
		kth_dest.push_back(x);
		return current_cache->set_vert_valid(x, true);
	}

	if(level[x] == target_level){
		if(is_through[x])  return current_cache->set_vert_valid(x, true);
		else return false;
	}

	bool ok = false; // if x is valid
	for(auto& epr: G[x]) {
		int to = epr->to;
		if(is_disable[to]) continue;
		if(level[to] > target_level) continue;
        if(level[to] == target_level and is_through[to]==0) continue;

		if(search_fout_layer(to, target_level)) {
			ok = true;
			current_cache->set_edge_valid(epr->id, true);
		}
	}

	if(ok) {
		//cout << "set vert_valid" << get_node_name(x) << "\n";
		return current_cache->set_vert_valid(x, true);
	}
	else return false;
}

void BC_map::search_all() {

	ASSERT(next_level.size() == 0);
	// take all of FF:clk and Pin as a start point
	for(int i=0; i<(int)G.size(); i++){
		//const auto& node = graph->nodes[ get_graph_id(i) ];
		// i is a FF:clk or Pin
		if(G[i].size() != 0 && Gr[i].size() == 0) {
			kth_start.push_back(i);
			if(search_fout(i, 0)) {
				// is_valid[i] = 1;
				current_cache->set_vert_valid(i, true);
			}
		}
	}
}

void BC_map::search(const vector<int>& through) {
	ASSERT(true==false);
	kth_start.clear(); // FF:clk or Pin
	kth_dest.clear();  // FF:D or Pout
	std::fill(vis.begin(), vis.end(), 0);
	current_cache->clear();

	if(through.size()) {
		for(auto &x: through) {
			// start from the point has the lowest level
			if(level[x] == next_level[0]) {
				if(search_fout(x, 1)) {
					// is_valid[x] = 1;
					current_cache->set_vert_valid(x, true);
					vis[x] = 0;
					search_fin(x);
				}
			}
			else break;
		}
	}
	else {
		ASSERT(next_level.size() == 0);
		// take all of FF:clk and Pin as a start point
		for(int i=0; i<(int)G.size(); i++){
			//const auto& node = graph->nodes[ get_graph_id(i) ];
			// i is a FF:clk or Pin
			if(G[i].size() != 0 && Gr[i].size() == 0) {
				kth_start.push_back(i);
				if(search_fout(i, 0)) {
					// is_valid[i] = 1;
					current_cache->set_vert_valid(i, true);
				}
			}
		}
	}
}

void BC_map::search_fin(int x) {

	// any point found by the search_fin is valid
	// is_valid[x] = true;
	current_cache->set_vert_valid(x, true);
	if(Gr[x].size() == 0) {
		kth_start.push_back(x) ;
		return;
	}

	if(vis[x]) return;
	vis[x] = 1;

	for(auto& p_e: Gr[x]) {
		auto& e = *p_e;

		if(is_disable[e.to]) continue;
		search_fin(e.to);

		current_cache->set_edge_valid(e.id, true);
	}
}

bool BC_map::search_fout(int x,int next_level_id) {

	ASSERT(true==false);
	// if(vis[x]) return is_valid[x];
	if(vis[x]) {
		return current_cache->get_vert_valid(x);
		// return is_valid[x];
	}
	vis[x] = true;

	// no fout: FF:D or Pout
	if(G[x].size()==0) {
		if(next_level_id == (int)next_level.size()) {
			kth_dest.push_back( x );
			// return is_valid[x] = true;
			return current_cache->set_vert_valid(x, true);
		}
		// else return is_valid[x] = false;
		else return false;
	}
	else {
		for(auto& p_e: G[x]) {
			auto& e = *p_e;
			int to = e.to;
			if(is_disable[to] == true) continue;

			bool ok = false;
			if(next_level_id == (int) next_level.size()) {
				if(search_fout(to, next_level_id)) ok = true;
			}
			/* next_level_id < next_level.size() */
			else if(level[to] == next_level[next_level_id] and is_through[to]) {
				if(search_fout(to, next_level_id+1)) ok = true;
			}
			else if(level[to] < next_level[next_level_id]) {
				if(search_fout(to, next_level_id)) ok = true;
			}
			else {
				// we don't look at the point which level is greater than the next_level
			}

			if(ok) {
				 //cout << get_node_name(e.from) << "(" << e.from << ") " << get_node_name(e.to) << "(" << e.to << ") is valid\n";
				// cout << "rev: " << get_node_name(e.rev_edge->from) << "(" << e.rev_edge->from << ") " << get_node_name(e.rev_edge->to) << "(" << e.rev_edge->to << ") is valid\n";
				current_cache->set_vert_valid(x, true);
				current_cache->set_edge_valid(e.id, true);
			}
		}
		// return is_valid[x];
		return current_cache->get_vert_valid(x);
	}
}

void BC_map::do_kth(const vector<int>& condidate, size_t k, std::function<void(Kth*,int,int,vector<Path*>&)> fun, vector<Path*>& ans) {

	auto compare_path = [](const Path* a, const Path* b) {
		return a->dist < b->dist;
	};
	priority_queue<Path*, vector<Path*>, decltype(compare_path)> path_heap(compare_path);
	threshold = std::numeric_limits<float>::max(); // slack should less than threshold

	Logger::add_record("num_kth", condidate.size());

	// enumerate every condidat to do kth-sortest path with cppr
    #pragma omp parallel for
	for(size_t i=0; i<condidate.size(); i++)
	{
		int tid = omp_get_thread_num();
		int st = condidate[i];

		Kth* kth = kths[tid];
		kth->clear();
		path_kth[tid].clear();

		fun(kth, st, k, path_kth[tid]);

		// update path heap
		#pragma omp critical
		{
			// cout << "\ntid: " << tid << " launch kth: " << this->get_node_name(st) << '\n';
			// for(auto& p:path_kth[tid]) p->print();
			size_t index = 0;
			while(path_heap.size() < k and index<path_kth[tid].size()) {
				path_heap.push( path_kth[tid][index++] );
			}

            // cout << "Size " << path_heap.size()  << "\n" << std::flush;
			while(index<path_kth[tid].size() and path_heap.top()->dist > path_kth[tid][index]->dist ) {
				auto pt = path_heap.top();
				//delete path_heap.top();
				path_heap.pop();
				delete pt;

				path_heap.push( path_kth[tid][index++] );
			}

			// update threshold
			if(path_heap.size() >=k ) threshold = path_heap.top()->dist;
		}
	}

	ans.clear();
	while(!path_heap.empty()) {
		ans.emplace_back( path_heap.top() );
		path_heap.pop();
	}
	std::reverse(ans.begin(), ans.end());
}
