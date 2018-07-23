#include "cache_node.h"

CacheNode::CacheNode(BC_map* map, int src,int det) {
	bc_map = map;
	//vis.resize( bc_map->num_node + 4 );

	vis.resize(map->num_node + 4);
	is_valid.resize(map->num_node + 4);
	valid_edges.resize(map->num_node + 4);
	valid_edges_reverse.resize(map->num_node + 4);

	set_src_dest(src, det);
	clear();
}

CacheNode::CacheNode(BC_map* map, int id) {
	bc_map = map;

	vis.resize(map->num_node + 4);
	is_valid.resize(map->num_node + 4);
	valid_edges.resize(map->num_node + 4);
	valid_edges_reverse.resize(map->num_node + 4);

	this->id = id;
	clear();
}

CacheNode::~CacheNode() {
	for(auto &e : edge_collector) delete e;
}

void CacheNode::set_src_dest(int src, int det) {
	source = src;
	dest = det;

	/* if src is ff:clk or PIN, then src = 0, and level[src] = 0 */
	start_level = this->bc_map->level[ src ];

	/* if dest is ff:d or POUT, then dest = bc->num_node, and level[dest] = bc->max_level + 1 */
	end_level = this->bc_map->level[ dest ];

}

void CacheNode::clear() {
	for(auto &e : edge_collector) delete e;

	for(size_t i=0; i<topological_order.size(); i++) {
		valid_edges[ topological_order[i] ].clear();
		valid_edges_reverse[ topological_order[i] ].clear();
		is_valid[ topological_order[i] ] = false;
	}
	for(size_t i=0; i<visited_points.size(); i++) {
		vis[ visited_points[i] ] = false;
	}

	//valid_edges.clear();
	//valid_edges_reverse.clear();
	kth_src.clear();
	kth_dest.clear();
	topological_order.clear();
	edge_collector.clear();
	visited_points.clear();

	//std::fill(vis.begin(), vis.end(), -1);
	//vis.clear();
	//is_valid.clear();
	has_built = false;
	used_cnt = 0;
	cnt_for_using = 0;
}

void CacheNode::wait_for_update() {
	if(has_built == false) {
		// std::unique_lock<std::mutex> lock(mut_update);
		// while(has_built == false) cv_update.wait(lock);
		while(has_built == false) ;
	}
	return;
}

void CacheNode::update() {

	if(has_built == false) {
		// search space
		//if(source== 0) {
			//// from dest backward search all ff:clk or PIN
			//int g_id = bc_map->get_graph_id( dest );
			//search_source(bc_map->get_index(LATE, RISE, g_id));
			//search_source(bc_map->get_index(LATE, FALL, g_id));
		//}
		//else {
			//int g_id = bc_map->get_graph_id( source );
			//search_dest(bc_map->get_index(LATE, RISE, g_id));
			//search_dest(bc_map->get_index(LATE, FALL, g_id));
		//}

		if(dest == bc_map->num_node) {
			int g_id = bc_map->get_graph_id(source);
			search_dest(bc_map->get_index(LATE, RISE, g_id));
			search_dest(bc_map->get_index(LATE, FALL, g_id));
		}else {
			int g_id = bc_map->get_graph_id(dest);
			search_source(bc_map->get_index(LATE, RISE, g_id));
			search_source(bc_map->get_index(LATE, FALL, g_id));
		}

		if(source==0) {
			topological_order.push_back(0);
			connect_pseudo_edge_source();
		}
		if(dest==bc_map->num_node) {
			topological_order.push_back(dest);
			connect_pseudo_edge_dest();
		}

		std::sort(topological_order.begin(), topological_order.end(), [&](int v1, int v2){
				return bc_map->level[v1] < bc_map->level[v2];
		});
		has_built = true;
		cv_update.notify_one();
	}
}

void CacheNode::add_edge(int from, int to, float delay) {
	valid_edges[from].emplace_back(new Cache_Edge(from, to, delay));
	valid_edges_reverse[to].emplace_back(new Cache_Edge(to, from, delay));

	edge_collector.push_back(valid_edges[from].back());
	edge_collector.push_back(valid_edges_reverse[to].back());
}

void CacheNode::connect_pseudo_edge_source() {
	for(auto &x: kth_src) {
		int g_id = bc_map->get_graph_id(x);
		Mode mode = bc_map->get_graph_id_mode(x);
		Transition_Type type = bc_map->get_graph_id_type(x);

		add_edge(source, x, -bc_map->graph->nodes[g_id].at[mode][type]);
	}
}

void CacheNode::connect_pseudo_edge_dest() {
	for(auto &x: kth_dest) {
		int g_id = bc_map->get_graph_id(x);
		Mode mode = bc_map->get_graph_id_mode(x);
		Transition_Type type = bc_map->get_graph_id_type(x);

		add_edge(x, dest, bc_map->graph->nodes[g_id].rat[mode][type]);
	}
}

bool CacheNode::search_source(int now) {
	if(vis[now]) return is_valid[now];
	vis[now] = true;
	visited_points.push_back(now);

	if(bc_map->Gr[now].size() == 0) {
		kth_src.push_back(now);
		topological_order.push_back(now);

		return is_valid[now] = true;
	}
	else if( bc_map->level[now] == start_level and bc_map->get_graph_id(now) == bc_map->get_graph_id(source)) {
		topological_order.push_back(now);

		return is_valid[now] = true;
	}

	bool ok = false;
	for(size_t i=0; i<bc_map->Gr[now].size(); i++) {
		const auto& e = bc_map->Gr[now][i];
		if(bc_map->level[e->to] >= start_level and search_source(e->to)) {
			ok = true;
			add_edge(e->to, e->from, e->delay);
		}
	}

	if(ok) {
		topological_order.push_back(now);

		return is_valid[now] = true;
	}
	return ok;
}

bool CacheNode::search_dest(int now) {
	if(vis[now]) return is_valid[now];
	vis[now] = true;
	visited_points.push_back(now);

	if(dest==bc_map->num_node) {
		if( bc_map->G[now].size() == 0 ) {
			kth_dest.push_back(now);
			topological_order.push_back(now);

			return is_valid[now] = true;
		}
	}
	else if( bc_map->level[now] == end_level and bc_map->get_graph_id(now) == bc_map->get_graph_id(dest)) {
		topological_order.push_back(now);

		return is_valid[now] = true;
	}

	bool ok = false;
	for(const auto& e: bc_map->G[now]) {
		//if(e->jump != nullptr) {
			//if(bc_map->level[e->jump->to] <= end_level) {
				//if(search_dest(e->jump->to)) {
					//add_edge(e->jump->from, e->jump->to, e->jump->delay);
					//ok = true;
				//}
			//}
		//}
		if(bc_map->level[e->to] <= end_level){
			if(search_dest(e->to)) {
				add_edge(e->from, e->to, e->delay);
				ok = true;
			}
		}
	}

	if(ok) {
		topological_order.push_back(now);
		is_valid[now] = true;
	}
	return ok;
}

void CacheNode::print() {
	cout << start_level << " -> " << end_level << std::flush << ' ';

	if(source == 0) cout << "pseudo source ";
	else cout << bc_map->graph->get_name( bc_map->get_graph_id(source) ) << ' ';

	cout << "-> ";
	if(dest== bc_map->num_node) cout << "pseudo dest\n";
	else cout <<  bc_map->graph->get_name( bc_map->get_graph_id(dest) ) << '\n';

	if(source == 0) {
		cout << "	all source: \n";
		for(auto&x :kth_src) cout << "		" << this->bc_map->get_node_name(x) << '\n';
	}
	if(dest == bc_map->num_node) {
		cout << "	all destination: \n";
		for(auto&x :kth_dest) cout << "		" << this->bc_map->get_node_name(x) << '\n';
	}

	cout << "	valid edges: \n";
	int cnt = 0;
	for(auto& p: valid_edges) {
		// for(auto &e: p.second) {
		for(auto &e: p) {
			int to = e->to;
			int from = e->from;
			cout << "		" << bc_map->get_node_name(from) << " -> " << bc_map->get_node_name(to) << '\n';
			cnt ++;
		}
	}
	cout << "	total : " << cnt << '\n';

	cout << "	valid reverse edges: \n";
	cnt = 0;
	for(auto& p: valid_edges_reverse) {
		// for(auto &e: p.second) {
		for(auto &e: p) {
			int from = e->from;
			int to = e->to;
			cout << "		" << bc_map->get_node_name(from) << " -> " << bc_map->get_node_name(to) << '\n';
			cnt++;
		}
	}
	cout << "	total : " << cnt << '\n';

	cout << "	valid nodes(" << topological_order.size() << ")" << ": \n";
	cout << "		";
	for(auto& x: topological_order) {
		cout << bc_map->get_node_name(x) << "(" << bc_map->level[x] << "), ";
	}
	cout << "\n";

	cout << "\n";
}
