#include "cache_node.h"

CacheNode::CacheNode(BC_map* map, int src, CacheNode_Type type) {
	//source = src;
	//cache_type = type;
	bc_map = map;
	is_valid.resize(map->num_node + 4);
	vis.resize(map->num_node + 4);

	init(src, type);
}

CacheNode::~CacheNode() {

}

/*
 * call init after called `clear` or created it.
 *
 * */
void CacheNode::init(int src, CacheNode_Type type) {
	source_level = bc_map->level[ src ];
	cache_type = type;
	source = src;

	if(type == CACHE_FIN) {
		searched_level = source_level;

		endpoints.push_back(bc_map->get_index(LATE, RISE, bc_map->get_graph_id(src)));
		endpoints.push_back(bc_map->get_index(LATE, FALL, bc_map->get_graph_id(src)));

		is_valid[ bc_map->get_index(LATE, RISE, bc_map->get_graph_id(src)) ] = true;
		is_valid[ bc_map->get_index(LATE, FALL, bc_map->get_graph_id(src)) ] = true;

		valid_points.push_back( bc_map->get_index(LATE, RISE, bc_map->get_graph_id(src)) );
		valid_points.push_back( bc_map->get_index(LATE, FALL, bc_map->get_graph_id(src)) );

		if(bc_map->Jr[source].size() == 0) {
			kth_src.push_back( bc_map->get_index(LATE, RISE, bc_map->get_graph_id(src)) );
			kth_src.push_back( bc_map->get_index(LATE, FALL, bc_map->get_graph_id(src)) );
		}

		need_update = false;
	}
	else need_update = true;

}

void CacheNode::update() {
	if(!need_update) return;

	if(cache_type == CACHE_FIN) {
		for(auto& x: endpoints) {
			vis[x] = 0;
			search_source(x);
		}
		endpoints.swap(next_endpoints);
		next_endpoints.clear();
	}
	else {
		int g_id = bc_map->get_graph_id(source);

		search_dest(bc_map->get_index(LATE, RISE, g_id));
		search_dest(bc_map->get_index(LATE, FALL, g_id));
	}

	need_update = false;
}

void CacheNode::search_source(int now) {
	if(vis[now]) return;
	vis[now] = 1;
	visited_points.push_back(now);

	if(bc_map->Jr[now].size() == 0) {
		kth_src.push_back(now);
	}

	if(bc_map->level[now] <= searched_level) {
		next_endpoints.push_back(now);
		return;
	}

	//for(const auto&e: bc_map->Gr[now]) {
		//if(bc_map->condensed_by[e->to] != -1) {
			//Logger::add_record("waste time", 1);
			//continue;
		//}

		//is_valid[e->to] = true;
		//valid_points.push_back(e->to);
		//search_source(e->to);
	//}
	for(const auto& e: bc_map->Jr[now]) {
		is_valid[e->to] = true;
		valid_points.push_back(e->to);
		search_source(e->to);
	}
}

bool CacheNode::search_dest(int now) {
	if(vis[now]) return is_valid[now];
	vis[now] = true;
	visited_points.push_back(now);

	if(bc_map->J[now].size() == 0) {
		kth_dest.push_back(now);

		valid_points.push_back(now);
		return is_valid[now] = true;
	}

	bool ok = false;
	//for(const auto& e: bc_map->G[now]) {
		//if(bc_map->condensed_by[e->to] != -1) {
			//Logger::add_record("wast time\n", 1);
			//continue;
		//}
		//if(search_dest(e->to)) {
			//ok = true;
		//}
	//}
	for(const auto& e: bc_map->J[now]) {
		if(search_dest(e->to)) {
			ok = true;
		}
	}

	if(ok) valid_points.push_back(now);
	return is_valid[now] = ok;
}

void CacheNode::clear() {
	for(auto& x: valid_points) is_valid[x] = 0;
	for(auto& x: visited_points) vis[x] = 0;

	visited_points.clear();
	valid_points.clear();

	kth_src.clear();
	kth_dest.clear();
	need_update = false;

	endpoints.clear();
	next_endpoints.clear();
	used_cnt = 0;
}

void CacheNode::set_target_level(int target_level) {
	if(target_level < searched_level) {
		searched_level  = target_level;
		need_update = true;
	}
}

const vector<int>& CacheNode::get_kth_src() {
	return kth_src;
}

const vector<int>& CacheNode::get_kth_dest() {
	return kth_dest;
}

void CacheNode::print() {
	int g_id = bc_map->get_graph_id(source);
	cout << "Source: " << bc_map->graph->nodes[ g_id].name << '(';
	cout << bc_map->level[source] << ") ";

	if(cache_type == CACHE_FIN) {
		cout << "searched_level = " << searched_level << "\n";
		cout << "	Type: CACHE_FIN\n";

		if(kth_src.size()) {
			cout << "	kth_src:\n";
			for(auto& x: kth_src) cout << "		" << bc_map->get_node_name(x) << '\n';
		}
		cout << "	valid points(" << valid_points.size() << ")\n";
		for(auto &x: valid_points) {
			cout << "		" << bc_map->get_node_name(x) << ' ';
			cout << "(" << bc_map->level[x] << ")\n";
		} cout << '\n';
		cout << "	endpoints(" << endpoints.size() << ")\n";
		for(auto &x: endpoints) {
			cout << "		" << bc_map->get_node_name(x) << ' ';
			cout << "(" << bc_map->level[x] << ")\n";
		}
	}
	else { // CACHE_FOUT
		cout << "\n	Type: CACHE_FOUT\n";
		cout << "	valid points(" << valid_points.size() << ")\n";
		for(auto &x: valid_points) {
			cout << "		" << bc_map->get_node_name(x) << ' ';
			cout << "(" << bc_map->level[x] << ")\n";
		}

		cout << "	kth_dest:\n";
		for(auto& x: kth_dest) cout << "		" << bc_map->get_node_name(x) << '\n';
	}
}

