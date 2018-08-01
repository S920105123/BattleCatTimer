#include "bc_map.h"

BC_map::BC_map(Graph* graph, CPPR* cppr){
    this->graph = graph;
	this->cppr = cppr;
	cache = new Cache(this);

	query_cnt = 0;
}

BC_map::~BC_map() {
	for(size_t i=0; i<cache_node_collector.size(); i++) delete cache_node_collector[i];

	int i;
	#pragma omp parallel for 
	for(i=0; i<num_node; i++) {
		for(auto& e: G[i]) delete e;
		for(auto& e: Gr[i]) delete e;
		for(auto& e: J[i]) delete e;
		for(auto& e: Jr[i]) delete e;
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

void BC_map::add_jump_edge(int from, int to, float delay) {
	ASSERT(level[from] < level[to]);
	J[from].push_back(new Edge(from, to, delay));
	Jr[to].push_back(new Edge(to, from, delay));
	in_degree[to]++;

	nodes_on_jump.emplace_back(unordered_map<int,bool>());
	for(auto& x: nodes_jump_edge) {
		nodes_on_jump.back()[x] = 1;
	}
	J[from].back()->nodes_on_jump_id = nodes_on_jump.size() - 1;
	Jr[to].back()->nodes_on_jump_id = nodes_on_jump.size() - 1;
	Logger::add_record("Jump", 1);
}

void BC_map::build(){

// 1. Map pins in graph to bc_map node, every pin has four nodes in bc_map (E/L)*(R/F)
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
    LOG(NORMAL) << "BCmap nodes = " << num_node << "\n";

    G.resize(num_node + 2);
    Gr.resize(num_node + 2);
    J.resize(num_node + 2);
    Jr.resize(num_node + 2);
    in_degree.resize(num_node + 2);
    vis.resize(num_node + 2);
    level.resize(num_node + 2);
	cache_nodes[CACHE_FIN].resize(num_node + 2);
	cache_nodes[CACHE_FOUT].resize(num_node + 2);

	condensed_by.resize( num_node + 2 );
	std::fill(condensed_by.begin(), condensed_by.end(), -1);

	for(int i=0; i<=num_node; i++) {
		cache_nodes[CACHE_FIN][i] = cache_nodes[CACHE_FOUT][i] = nullptr;
	}

	for(int i=0; i<MAX_CACHE; i++) {
		cache_node_collector.push_back(new CacheNode(this, i));
		free_cache_node.push( cache_node_collector.back() );
	}

// 2. build graph G based on timing arc

    // dfs build map
    for(int i=0; i<(int)graph->nodes.size(); i++) if(i!=graph->clock_id) {
		Graph::Node &node = graph->nodes[i];
		if(node.type == CLOCK || node.type == PRIMARY_IN)
        //if(node.type == CLOCK )
			build_map(i);
    }

// 3. build level of G
    queue<int>* q = new queue<int>();
    for(int i=0; i<num_node; i++) if(in_degree[i]==0) {
    	q->push(i);
	}

    while(!q->empty()) {
        int x = q->front(); q->pop();
        for(const auto& e:G[x]) {
            in_degree[e->to]--;
            if(in_degree[e->to]==0) q->push(e->to);
            level[e->to] = max(level[e->to], level[x]+1);
        }
    }
	delete q;

	// level starts from 1
	max_level = 0;
	for(int i=1; i<num_node; i++) {
		level[i]++;
		max_level = max(max_level, level[i]);
	}
	level[0] = 0, level[num_node] = max_level + 1;

//. 4. condense G get new graph J
	Logger::start();
	for(int i=0; i<=num_node; i++) topological_order.push_back(i);
	std::sort(topological_order.begin(), topological_order.end(), [&](int v1,int v2) {
			return level[v1] < level[v2];
	});

	std::fill(vis.begin(), vis.end(), 0);
	for(int i=1; i<num_node; i++) if(!vis[topological_order[i]]) {
		condense(topological_order[i]);
	}
	Logger::stop("condense");
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

// *****************************
// *         condense          *
// *****************************
bool BC_map::is_jump_tail(int x) { // if there is one fanout of x which has mulitple fanfin, then it is jump tail. Or x doesn't have fanout.

	if(G[x].size() == 0 or Gr[x].size() > 1) return true;
	return false;
}

void BC_map::condense(int x, int root, float delay) {
	if(is_jump_tail(x)) {
		// create jump from root to x with delay
		add_jump_edge(root, x, delay);
		return;
	}

	ASSERT(condensed_by[x] == -1);
	condensed_by[x] = root;
	nodes_jump_edge.push_back(x);
	for(auto& e: G[x]) {
		condense(e->to, root, delay+e->delay);
	}
	nodes_jump_edge.pop_back();
}

void BC_map::condense(int x) {
	if(vis[x]) return;
	vis[x] = 1;
	if(condensed_by[x]!=-1) return;

	for(auto& e: G[x]) {
		//if(is_jump_tail(e->to)) continue;
		condense(e->to, x, e->delay);
	}
	ASSERT(nodes_jump_edge.size() == 0);
}

void BC_map::check_condense() {
	for(int i=1; i<num_node; i++) {
		if(Jr[i].size()) {
			ASSERT(is_jump_tail(i));
			ASSERT(Gr[i].size() == 1);
			ASSERT(Jr[i].size() == 1);
			ASSERT(condensed_by[i] == -1);

			auto jump_e = Jr[i].front();
			int root = jump_e->to;
			ASSERT(jump_e->from == i);

			//cout << "Jump " << get_node_name(jump_e->to) << " to " << get_node_name(jump_e->from) << '\n';
			int cur = i, len = 1;
			//cout << get_node_name(cur);
			while(cur!=root) {
				len++;
				//cout << " -> ";
				ASSERT(Gr[cur].size()==1);
				cur = Gr[cur].front()->to;
				//cout << get_node_name(cur) << "(" << get_node_name(condensed_by[cur]) ;
				//cout << ")";
				if(cur!=root) ASSERT(condensed_by[cur] == root);
			}

			Logger::add_record("jump_" + std::to_string(len), 1);
			int find = 0;
			for(auto& e: J[root]) {
				if(e->from == root and e->to == i and e->delay == jump_e->delay) find++;
			}
			ASSERT(find == 1);
			//cout << "\nFind = " << find ;
			//cout << "\n\n";
		}
	}
}

/******************************************
*           k shortest path               *
******************************************/

// *****************************
// *         cache node       *
// *****************************

void BC_map::erase_cache_node(CacheNode* node) {

	ASSERT( cache_nodes[ node->cache_type ][ node->source] != nullptr );
	node->last_used = -1;
	cache_nodes[ node->cache_type ][ node->source] = nullptr;
}

CacheNode* BC_map::add_cache_node(int source, CacheNode_Type type) {
	CacheNode* node = nullptr;

	if( free_cache_node.size() ) {
		node = free_cache_node.front();
		free_cache_node.pop();

	}
	else {
		int min_cnt_used = std::numeric_limits<int>::max(), who = -1;
		for(size_t i=0; i<cache_node_collector.size(); i++) {
			if(cache_node_collector[i]->last_used != query_cnt and cache_node_collector[i]->used_cnt < min_cnt_used) {
				min_cnt_used = cache_node_collector[i]->used_cnt;
				who = i;
			}
		}

		ASSERT(who != -1);
		node = cache_node_collector[who];
		ASSERT(node->last_used != -1);
		erase_cache_node(node);
	}
	Logger::start();
	node->clear();
	Logger::stop("1.1 clear cache node");

	node->init(source, type);
	cache_nodes[type][source] = node;
	return node;
}

CacheNode* BC_map::get_cache_node(int source, CacheNode_Type type) {

	ASSERT(source != 0);
	ASSERT(source != num_node);

	source = get_index(LATE, RISE, get_graph_id(source));
	if( cache_nodes[type][source]==nullptr ) return add_cache_node(source, type);
	else {
        Logger::add_record("Save", 1);
        return cache_nodes[type][source];
    }
}

void BC_map::k_shortest_path(vector<int>& through, vector<int>& disable, int k, vector<Path*>& ans) {
	if(through.size() == 0) {
		return;
	}
Logger::start();
	cache->clear();
	cache->set_through(through);

	for(auto& x: through) if(condensed_by[x] != -1) {
		x = condensed_by[x];
	}
	std::sort(through.begin(), through.end(), [&](int v1, int v2) {
			return level[v1] < level[v2];
	});
	through.resize( std::unique(through.begin(), through.end()) - through.begin() );

	vector<vector<int>> through_level;

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
Logger::stop("0. init through");

Logger::start();
	query_cnt++;
	for(auto& node: cache_node_collector) {
		if(node->last_used !=-1 and query_cnt - node->last_used >= 10) {
			free_cache_node.push(node);
			erase_cache_node(node);
		}
	}
	for(size_t i=0; i<through_level.size(); i++) {
		CacheNode* node = get_cache_node(through_level[i].front(), CACHE_FIN);
		node->last_used = query_cnt;
		node->used_cnt++;
		if(i == 0) {
			node->set_target_level(1); // to PIN or FF:CLK
			cache->add_cache_node(node, 0, level[ through_level[i].front() ]);
		}
		else {
			node->set_target_level( level[through_level[i-1].front()] );
			cache->add_cache_node(node, level[ through_level[i-1].front() ], level[ through_level[i].front() ]);
		}
		
	}

	CacheNode* node = get_cache_node(through_level.back().front(), CACHE_FOUT);
	node->last_used = query_cnt;
	node->used_cnt++;
	cache->add_cache_node(node, level[ through_level.back().front() ], max_level);
Logger::stop("1. choose cache");

	for(size_t i=0; i<through_level.size(); i++) {
		bool used_transition[2] = {0, 0};
		for(auto &x: through_level[i]) used_transition[ get_graph_id_type(x) ] = 1;
		for(int j=0; j<2; j++) if(used_transition[j]==0) {
			disable.emplace_back(get_index(LATE, j, get_graph_id(through_level[i].front())));
		}
	}

	cache->set_disable(disable);

Logger::start();
	cache->update_cacheNode();
Logger::stop("2. search space");
Logger::start();
	cache->build_graph();
Logger::stop("3. build valid graph");

Logger::start();
	cache->kth(ans, k);
Logger::stop("4. do kth");
}

