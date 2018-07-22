#include "bc_map.h"

BC_map::BC_map(Graph* graph, CPPR* cppr){
    this->graph = graph;
	this->cppr = cppr;
	cache = new Cache(this);
	cache_node_collector.resize(MAX_CACHE);
	std::fill(cache_node_collector.begin(), cache_node_collector.end(), nullptr);
	query_cnt = 0;

	//for(int i=0; i<NUM_THREAD; i++) {
		//kths[i] = new Kth(this, cppr, graph);
	//}
}

BC_map::~BC_map() {
	//for(int i=0; i<NUM_THREAD; i++) {
		//delete kths[i];
	//}
	for(size_t i=0; i<this->cache_node_collector.size(); i++) {
		if(cache_node_collector[i] != nullptr ) delete cache_node_collector[i];
	}

	delete cache;
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
	map_id--;
    return map_id>>1;
}

Mode BC_map::get_graph_id_mode(int map_id){
    return LATE;
    // return (map_id&2)? LATE:EARLY;
}

Transition_Type BC_map::get_graph_id_type(int map_id){
    //return (map_id&1)? FALL:RISE;
    return (map_id&1)? RISE:FALL;
}

string BC_map::get_node_name(int map_id){
	if(map_id == 0) return "pseudo source";
	if(map_id == num_node) return "pseudo destination";

    int graph_id = get_graph_id(map_id);
    Mode mode = get_graph_id_mode(map_id);
    Transition_Type type = get_graph_id_type(map_id);
    return graph->nodes[graph_id].name+":"+get_mode_string(mode)+":"+get_transition_string(type);
}

void BC_map::add_edge(int from, int to, float delay){
	Edge *e = new Edge(from, to, delay);
	Edge *re = new Edge(to, from, delay);

    G[from].emplace_back(e);
    Gr[to].emplace_back(re);
	e->id = re ->id = num_edge++;
    in_degree[to]++;
}

void BC_map::build(){
    // add node
    // every graph node has 2 nodes in map
	num_edge = 0;
    num_node = 1; // num id starts from 1
    for(size_t i=0; i<graph->nodes.size(); i++){
        // to_map_id[EARLY][RISE].emplace_back(num_node++);
        // to_map_id[EARLY][FALL].emplace_back(num_node++);
        // if(!graph->nodes[i].exist) continue;
        to_map_id[LATE][RISE].emplace_back(num_node++);
        to_map_id[LATE][FALL].emplace_back(num_node++);
    }
    G.resize(num_node + 2);
    Gr.resize(num_node + 2);
    in_degree.resize(num_node + 2);
    vis.resize(num_node + 2);
    level.resize(num_node + 2);
	//is_disable.resize(num_node);
	//is_valid.resize(num_node);
	//is_through.resize(num_node);
	cache_nodes.resize(num_node + 2);

    // dfs build map
    for(int i=0; i<(int)graph->nodes.size(); i++) if(i!=graph->clock_id) {
		Graph::Node &node = graph->nodes[i];
		if(node.type == CLOCK || node.type == PRIMARY_IN)
        //if(node.type == CLOCK )
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
	// level starts from 1
	max_level = 0;
	for(int i=1; i<num_node; i++) {
		level[i]++;
		max_level = max(max_level, level[i]);
	}
	level[0] = 0, level[num_node] = max_level + 1;

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

void BC_map::erase_cache_node(CacheNode* node) {
	//cout << "Del cache " << get_node_name(node->source) << " -> " << get_node_name( node->dest ) << '\n';

	auto it = cache_nodes[node->source].find( node->dest );
	ASSERT( it != cache_nodes[node->source].end() );

	cache_nodes[node->source].erase(it);

	//Logger::start();
	//delete node;
	//Logger::stop("Del");
}

CacheNode* BC_map::add_cache_node(int from, int to) {
	CacheNode* node = nullptr;

	int find = false;
	for(size_t i=0; i<cache_node_collector.size(); i++) if(cache_node_collector[i] == nullptr) {
		node = new CacheNode(this, from, to);
		cache_node_collector[i] = node;
		find = true;
		break;
	}

	if(!find) {

		// delete those for no using in long time
		for(size_t i=0; i<cache_node_collector.size(); i++) if(cache_node_collector[i] != nullptr) {
			if( query_cnt - cache_node_collector[i]->last_used >= 10) {
				erase_cache_node(cache_node_collector[i]);
				node = cache_node_collector[i];
				find = true;
				break;
			}
		}

		if(!find) {
			int min_cnt_used = std::numeric_limits<int>::max(), who = -1;
			for(size_t i=0; i<cache_node_collector.size(); i++) if(cache_node_collector[i] != nullptr) {
				if(cache_node_collector[i]->last_used != query_cnt and cache_node_collector[i]->used_cnt < min_cnt_used) {
					min_cnt_used = cache_node_collector[i]->used_cnt;
					who = i;
				}
			}

			// The number of cache node needed in this query exceeds the MAX_CACHE.
			ASSERT( who != -1 );
			erase_cache_node(cache_node_collector[who]);
			node = cache_node_collector[who];
		}
        // Logger::start();
		node->clear();
        // Logger::stop("clear");
		node->set_src_dest(from, to);
	}

	cache_nodes[from].emplace(to, node);
	return node;
}

CacheNode* BC_map::get_cache_node(int from, int to) {

	if(from !=0 and from !=num_node) from = get_index(LATE, RISE,  get_graph_id(from));
	if(to !=0 and to !=num_node) to = get_index(LATE, RISE,  get_graph_id(to));

	auto it = cache_nodes[from].find( to );
	if(it == cache_nodes[from].end()) return add_cache_node(from, to);
	else {
        Logger::add_record("Save", 1);
        return it->second;
    }
}

void BC_map::k_shortest_path(vector<int>& through, vector<int>& disable, int k, vector<Path*>& ans)
{
	if(through.size() == 0) {
		return;
	}
Logger::start();
	cache->clear();

	std::sort(through.begin(), through.end(), [&](int v1, int v2) {
			return level[v1] < level[v2];
	});

	vector<vector<int>> through_level;
	vector<vector<int>> disable_level;

	// put them to their corresponding level box

	// pseudo source
	through_level.push_back(vector<int>());
	through_level.back().push_back(0);

	for(size_t i=0; i<through.size(); ) {
		through_level.push_back( vector<int>() );
		int cur_level = level[ through[i] ];
		while(i < through.size() and cur_level == level[ through[i] ] ) {
			through_level.back().push_back( through[i] );
			i++;
		}

		// make sure that the same level points should have the same graph id
		for(size_t j=0; j<through_level.back().size() - 1; j++) {
			if( get_graph_id(through_level.back()[j]) != get_graph_id(through_level.back()[j+1]) )
				return;
		}
	}

	// pseudo destination
	through_level.push_back(vector<int>());
	through_level.back().push_back(num_node);

	//cout << "start put disable\n";
	disable_level.resize(max_level+4);
	for(auto& x: disable) disable_level[ level[x] ].push_back(x);
Logger::stop("init through");

	//cout << "start add cache node\n";
Logger::start();
	query_cnt++;
	for(size_t i=0; i<through_level.size() - 1; i++) {
		CacheNode* node = get_cache_node(through_level[i].front(), through_level[i+1].front() );
		node->last_used = query_cnt;
		node->used_cnt++;
		cache->add_cache_node(node);
	}
Logger::stop("choose cache");

	//cout << "start cal disable through\n";
	// Except pseudo source and destination
	for(size_t i=1; i<through_level.size()-1; i++) {
		bool used_transition[2] = {0, 0};
		for(auto &x: through_level[i]) used_transition[ get_graph_id_type(x) ] = 1;
		for(int j=0; j<2; j++) if(used_transition[j]==0) {
			disable.emplace_back(get_index(LATE, j, get_graph_id(through_level[i].front())));
		}
	}

	cache->set_disable(disable);

Logger::start();
	cache->update_cacheNode();
Logger::stop("search space");

Logger::start();
	cache->kth(ans, k);
Logger::stop("do kth");

	//cache->output_shortest_path();

	//cache->print();
	//cout << "======================== fin =======================\n";
}

//void BC_map::search(vector<int>& through) {

	//kth_start.clear(); // FF:clk or Pin
	//kth_dest.clear();  // FF:D or Pout
	//std::fill(vis.begin(), vis.end(), 0);

	//// clean the mark of the edges
	//for(auto &e: valid_edge)  e->valid = false;
	//valid_edge.clear();

	//if(through.size()) {
		//for(auto &x: through) {
			//// start from the point has the lowest level
			//if(level[x] == next_level[0]) {
				//if(search_fout(x, 1)) {
					//is_valid[x] = 1;
					//vis[x] = 0;
					//search_fin(x);
				//}
			//}
			//else break;
		//}
	//}
	//else {
		//ASSERT(next_level.size() == 0);
		//// take all of FF:clk and Pin as a start point
		//for(int i=0; i<(int)G.size(); i++){
			//// const auto& node = graph->nodes[ get_graph_id(i) ];
			//// i is a FF:clk or Pin
			//if(G[i].size() != 0 && Gr[i].size() == 0) {
				//kth_start.push_back(i);
				//if(search_fout(i, 0)) is_valid[i] = 1;
			//}
		//}
	//}
//}

//void BC_map::search_fin(int x) {

	//// any point found by the search_fin is valid
	//is_valid[x] = true;
	//if(Gr[x].size() == 0) {
		//kth_start.push_back(x) ;
		//return;
	//}

	//if(vis[x]) return;
	//vis[x] = 1;

	//for(auto& p_e: Gr[x]) {
		//auto& e = *p_e;
		//// cout << "search fin " << get_node_name(x) << " " << get_node_name(e.to) << "\n";
		//if(is_disable[e.to] == false) search_fin(e.to);

		//valid_edge.push_back( &e );
		//valid_edge.push_back( e.rev_edge );
		//e.valid = true;
		//e.rev_edge->valid = true;
		//// cout << get_node_name(e.from) << "(" << e.from << ") " << get_node_name(e.to) << "(" << e.to << ") is valid\n";
		//// cout << "rev: " << get_node_name(e.rev_edge->from) << "(" << e.rev_edge->from << ") " << get_node_name(e.rev_edge->to) << "(" << e.rev_edge->to << ") is valid\n";
	//}
//}

//bool BC_map::search_fout(int x,int next_level_id) {

	//if(vis[x]) return is_valid[x];
	//vis[x] = true;

	//// no fout: FF:D or Pout
	//if(G[x].size()==0) {
		//if(next_level_id == (int)next_level.size()) {
			//kth_dest.push_back( x );
			//return is_valid[x] = true;
		//}
		//else return is_valid[x] = false;
	//}
	//else {
		//for(auto& p_e: G[x]) {
			//auto& e = *p_e;
			//int to = e.to;
			//if(is_disable[to] == true) continue;

			//bool ok = false;
			//if(next_level_id == (int) next_level.size()) {
				//if(search_fout(to, next_level_id)) ok = true;
			//}
			//[> next_level_id < next_level.size() <]
			//else if(level[to] == next_level[next_level_id] and is_through[to]) {
				//if(search_fout(to, next_level_id+1)) ok = true;
			//}
			//else if(level[to] < next_level[next_level_id]) {
				//if(search_fout(to, next_level_id)) ok = true;
			//}
			//else {
				//// we don't look at the point which level is greater than the next_level
			//}

			//if(ok) {
				//// cout << get_node_name(e.from) << "(" << e.from << ") " << get_node_name(e.to) << "(" << e.to << ") is valid\n";
				//// cout << "rev: " << get_node_name(e.rev_edge->from) << "(" << e.rev_edge->from << ") " << get_node_name(e.rev_edge->to) << "(" << e.rev_edge->to << ") is valid\n";
				//is_valid[x] = true;
				//valid_edge.push_back( &e );
				//valid_edge.push_back( e.rev_edge );
				//e.valid = true;
				//e.rev_edge->valid = true;
			//}
		//}
		//return is_valid[x];
	//}
//}

//void BC_map::do_kth(const vector<int>& condidate, size_t k, std::function<void(Kth*,int,int,vector<Path*>&)> fun, vector<Path*>& ans) {

	//auto compare_path = [](const Path* a, const Path* b) {
		//return a->dist < b->dist;
	//};
	//priority_queue<Path*, vector<Path*>, decltype(compare_path)> path_heap(compare_path);
	//threshold = 0; // slack should less than threshold

	//Logger::add_record("num_kth", condidate.size());

	//// enumerate every condidat to do kth-sortest path with cppr
    //#pragma omp parallel for
	//for(size_t i=0; i<condidate.size(); i++)
	//{
		//int tid = omp_get_thread_num();
		//int st = condidate[i];

		//Kth* kth = kths[tid];
		//kth->clear();
		//path_kth[tid].clear();

		//fun(kth, st, k, path_kth[tid]);

		//// update path heap
		//#pragma omp critical
		//{
			//// cout << "\ntid: " << tid << " launch kth: " << this->get_node_name(st) << '\n';
			//// for(auto& p:path_kth[tid]) p->print();
			//size_t index = 0;
			//while(path_heap.size() < k and index<path_kth[tid].size()) {
				//path_heap.push( path_kth[tid][index++] );
			//}

            //// cout << "Size " << path_heap.size()  << "\n" << std::flush;
			//while(index<path_kth[tid].size() and path_heap.top()->dist > path_kth[tid][index]->dist ) {
				//auto pt = path_heap.top();
				////delete path_heap.top();
				//path_heap.pop();
				//delete pt;

				//path_heap.push( path_kth[tid][index++] );
			//}

			//// update threshold
			//if(path_heap.size() >=k ) threshold = path_heap.top()->dist;
		//}
	//}

	//ans.clear();
	//while(!path_heap.empty()) {
		//ans.emplace_back( path_heap.top() );
		//path_heap.pop();
	//}
	//std::reverse(ans.begin(), ans.end());
//}
