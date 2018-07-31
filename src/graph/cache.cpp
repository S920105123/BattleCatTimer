#include "cache.h"

Cache::Cache(BC_map* map):bc_map(map) {  }

void Cache::init_cache() {

	trans_id.resize(bc_map->num_node + 3);
	vis_timestamp.resize(bc_map->num_node+3);
	level_to_nodes.resize(bc_map->max_level+3);
	now_timestamp = 1;
}

void Cache::clear() {
	nodes.clear();
	level_to_nodes.clear();
	disable.clear();
	all_vec.clear();
	dist_to_dest.clear();
	sptTree.clear();

	fill(level_to_nodes.begin(), level_to_nodes.end(), -1);
}

void Cache::add_cache_node(CacheNode* node) {
	nodes.emplace_back(node);
	for(int i = node->start_level; i<=node->end_level; i++)
		level_to_nodes[i] = nodes.size() - 1;
}

void Cache::set_disable(const vector<int>& dis) {
	for(auto& x: dis) disable.emplace(x, true);
}

void Cache::update_cacheNode() {
	//#pragma omp parallel for schedule(dynamic)
	for(size_t i=0; i<nodes.size(); i++) nodes[i]->update();
}

void Cache::kth(vector<Path*>& ans, int k) {
	// Make sure that all cache nodes have updated
	
	build_SPT();

	if(dist_to_dest[src_tree] == INF) return;

	if(k == 1) {
		auto pfx = new PrefixNode(nullptr, src_tree, src_tree, 0, 0);
		ans.push_back( new Path() );
		recover_path(pfx, dest_tree, ans.back());
		ans.back()->dist = dist_to_dest[ src_tree ] + pfx->delta;
		delete pfx;
		return;
	}

	kth_Q.push( new PrefixNode(nullptr, src_tree, src_tree, 0, 0) );
	vector<PrefixNode*> trash_can;
	for(int i=0; i<k; i++) {
		if(kth_Q.empty()) break;
		auto pfx = kth_Q.top(); kth_Q.pop();

		ans.push_back( new Path() );
		recover_path(pfx, dest_tree, ans.back());
		trash_can.push_back(pfx);

		ans.back()->dist = dist_to_dest[ src_tree ] + pfx->delta;
		if(i+1 < k) super(pfx);
	}

	for(auto &x: trash_can) delete x;
	while(kth_Q.size()) {
		delete kth_Q.top();
		kth_Q.pop();
	}
}

int Cache::add_vec(int bc_id) {

	trans_id[bc_id] = all_vec.size();
	vis_timestamp[bc_id] = now_timestamp;
	all_vec.push_back(bc_id);
	sptTree.push_back(-1);
	dist_to_dest.push_back(INF);

	return trans_id[bc_id];
}

float Cache::dfs_build_SPT(int x) {
	// x is visited
	if(vis_timestamp[x] == now_timestamp) return dist_to_dest[ trans_id[x] ];
	else if(x == bc_map->num_node) {
		dest_tree = add_vec(x);
		sptTree[dest_tree] = dest_tree;
		return dist_to_dest[dest_tree] = 0;
	}

	int id = add_vec(x);

	for(const auto&e: get_edges(x)) if(!is_disable(e.to)){
		float delay = dfs_build_SPT(e.to);
		if(delay!=INF and delay + e.delay < dist_to_dest[id]) {
			dist_to_dest[id] = delay + e.delay;
			sptTree[id] =  trans_id[e.to];
		}
	}

	return dist_to_dest[id];
}

void Cache::build_SPT() {

	src_tree = 0;
	dfs_build_SPT(src_tree);
	now_timestamp++;
	Logger::add_record("spt vec", all_vec.size());
}

void Cache::super(PrefixNode* pfx) {

	int cur = pfx->to;
	while(cur != dest_tree) {
		for(auto &e: get_edges( all_vec[cur] )) {
			int from_id = trans_id[e.from];
			int to_id = trans_id[e.to];

			ASSERT(from_id == cur);
			if(sptTree[from_id] == to_id or is_disable(e.to) or dist_to_dest[to_id] == INF) continue;

			float delta = -dist_to_dest[from_id]+dist_to_dest[to_id]+e.delay;
			PrefixNode* node = new PrefixNode(pfx, from_id, to_id, e.delay, delta);
			kth_Q.push(node);
		}
		cur = sptTree[cur];
	}
}

void Cache::recover_path(PrefixNode* pfx, int dest, Path* path) {
	if(pfx->parent != nullptr) {
		recover_path( pfx->parent, pfx->from, path );
		path->delay.push_back( pfx->delay );
	}

	int cur = pfx->to;
	while(cur != dest) {
		path->path.push_back( all_vec[cur] );
		path->delay.push_back( dist_to_dest[cur]-dist_to_dest[sptTree[cur]] );
		cur = sptTree[cur];
	}
	path->path.push_back( all_vec[dest] );
}

const vector<Cache_Edge>& Cache::get_edges(int x) {
	int level = bc_map->level[x];
	return nodes[ level_to_nodes[level] ]->get_valid_edge(x);
}

const vector<Cache_Edge>& Cache::get_edges_reverse(int x) {
	int level = bc_map->level[x] - 1;
	level = level<0? 0:level;

	return nodes[ level_to_nodes[level] ]->get_valid_edge_reverse(x);
}

bool Cache::is_disable(int x) {
	//return disable.get_val(x);
	auto it = disable.find(x);
	if(it==disable.end()) return false;
	return it->second;
}

void Cache::print() {
	//cout << "level to nodes\n";
	//for(int i=0; i<level_to_nodes.size(); i++) {
	//cout << i << " : " << level_to_nodes[i] << '\n';
	//} 

	cout << "----------------------------------\n";
	//cout << "Disable:\n";
	//for(const auto& p: disable)
		//cout << bc_map->get_node_name(p.first) << '\n';

	cout << "All Vec:(" << all_vec.size() << ")\n";
	for(const auto& x: all_vec) {
		cout << bc_map->get_node_name(x) << "(" << bc_map->level[x] << ") ";
		cout << dist_to_dest[trans_id[x]] << " parent = " << bc_map->get_node_name( all_vec[sptTree[trans_id[x]]] );
		cout << '\n';
	} cout << '\n';

	for(size_t i=0; i<nodes.size(); i++) {
		cout << "[+] cache " << i << " : ";
		nodes[i]->print();
	}
}

void Cache::output_shortest_path() {
	cout << "Shortest path: " << dist_to_dest[src_tree] << "\n";

	int x = src_tree;
	while(x != dest_tree) {
		cout <<  bc_map->get_node_name( all_vec[x] ) << "\n";
		x = sptTree[x];
	}
	cout <<  bc_map->get_node_name( all_vec[dest_tree] ) << "\n ";
}

void print_name(ostream &fout, const string &name) {
	for(size_t i=0; i<name.size(); i++) {
		if(name[i] == ':') fout << '/';
		else fout << name[i];
	}
}

void Path::output(ostream &fout, Graph *graph) {
	int len = path.size();

	fout << "Endpoint:   "; print_name(fout, graph->nodes[ graph->bc_map->get_graph_id(path[len-2]) ].name ); fout << '\n';
	fout << "Beginpoint: "; print_name(fout, graph->nodes[ graph->bc_map->get_graph_id(path[1]) ].name ); fout << '\n';
	fout << "= Required Time               " << delay.back() << '\n';
	fout << "- Arrival Time                " << delay.back() - dist<< '\n';
	fout << "= Slack Time                  " << dist << "\n";

	const char* tab = "      ";
	const char type_sym[] = { '^', 'v' };
	fout << tab << "----------------------------------------\n";
	fout << tab << "Delay     Arrival    Edge  Pin\n";
	fout << tab << "          Time\n";
	fout << tab << "----------------------------------------\n";

	// output control
	fout << std::left << std::fixed << std::setprecision(OUTPUT_PRECISION);
	float at = 0;

	auto print_node_detail = [&](int x, float delay) {
		int g_id = graph->bc_map->get_graph_id( x );
		Transition_Type type = graph->bc_map->get_graph_id_type( x );
		at += -delay;

		fout << tab; fout << std::setw(10);
		if( delay ==  INF) fout << "-";
		else fout << -delay;

		fout << std::setw(11); fout <<  at;
		fout << std::setw(6); fout << type_sym[type];
		print_name(fout, graph->nodes[g_id].name ); fout << '\n';

	};

	float pre_delay = 0;
	for(size_t i=1; i<path.size()-1; i++) {
		print_node_detail(path[i], delay[i-1] - pre_delay);
		pre_delay = 0;

		//if(i+1 < path.size() - 1) {
			//for(auto &e: graph->bc_map->G[ path[i] ]) {
				//if(e->jump and e->jump->to == path[i+1] ) {
					//if( fabs(e->jump->delay - delay[i]) > 1e-3 ) {
						//continue;
					//}
					//auto cur_e = e;
					//while(cur_e->to != e->jump->to) {
						//print_node_detail(cur_e->to, cur_e->delay);
						//pre_delay += cur_e->delay;
						//cur_e = graph->bc_map->G[ cur_e->to ].front();
					//}
					//break;
				//}
			//}
		//}
	}
	fout << tab << "----------------------------------------\n";

	fout << '\n';
}

void print_name( const string& name, Writer& buf) {

	for(size_t i=0; i<name.size(); i++) {
		if(name[i] == ':') buf.addchar('/');
		else buf.addchar(name[i]);
	}
}

void print_float(float x, Writer& buf) {
	char s[100];
	sprintf(s, "%.2f", x);
	buf.addstring(s);
}

void Path::fast_output(Writer& buf, Graph* graph) {


	int len = path.size();
	buf.addstring("Endpoint: "); print_name(graph->nodes[ graph->bc_map->get_graph_id(path[len-2]) ].name, buf );  buf.addchar('\n');
	buf.addstring("Beginpoint: "); print_name(graph->nodes[ graph->bc_map->get_graph_id(path[1]) ].name, buf); buf.addchar('\n');
	buf.addstring("= Required Time "); print_float(delay.back(), buf); buf.addchar('\n');
	buf.addstring("- Arrival Time "); print_float(delay.back() - dist, buf); buf.addchar('\n');
	buf.addstring("= Slack Time ");  print_float(dist, buf); buf.addchar('\n');

	const char type_sym[] = { '^', 'v' };
	float at = 0;

	for(size_t i=1; i<path.size()-1; i++) {

		int x = path[i];
		int g_id = graph->bc_map->get_graph_id( x );
		Transition_Type type = graph->bc_map->get_graph_id_type( x );
		at += -delay[i-1];

		if( delay[i-1] ==  INF) buf.addchar('-');
		else print_float(-delay[i-1], buf);
		buf.addchar(' ');

		print_float(at, buf); buf.addchar(' ');

		buf.addchar(type_sym[type]); buf.addchar(' ');

		print_name(graph->nodes[g_id].name, buf);  buf.addchar('\n');
	}
	buf.addchar('\n');
}
