#include "cache.h"

Cache::Cache(BC_map* map):bc_map(map) {  }

void Cache::clear() {
	nodes.clear();
	level_to_nodes.clear();
	disable.clear();
	all_vec.clear();
	trans_id.clear();
	dist_to_dest.clear();
	sptTree.clear();

	level_to_nodes.resize( bc_map->max_level+3 );
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

	for(size_t i=0; i<nodes.size(); i++) {
		for(auto &x: nodes[i]->get_topological_order())
			all_vec.push_back(x);
	}

	all_vec.resize(std::unique(all_vec.begin(), all_vec.end()) - all_vec.begin());

	build_SPT();

	if(dist_to_dest[src_tree] == INF) return;

	kth_Q.push( new PrefixNode(nullptr, src_tree, src_tree, 0, 0) );
	vector<PrefixNode*> trash_can;
	for(int i=0; i<k; i++) {
		if(kth_Q.empty()) break;
		auto pfx = kth_Q.top(); kth_Q.pop();

		ans.push_back( new Path() );
		recover_path(pfx, dest_tree, ans.back());
		ans.back()->dist = dist_to_dest[ src_tree ] + pfx->delta;
		if(i+1 < k) super(pfx);
	}

	for(auto &x: trash_can) delete x;
	while(kth_Q.size()) {
		delete kth_Q.top();
		kth_Q.pop();
	}
}

void Cache::build_SPT() {

	for(size_t i=0; i<all_vec.size(); i++) trans_id.emplace(all_vec[i], i);
	dist_to_dest.resize(all_vec.size());
	sptTree.resize(all_vec.size());

	std::fill(sptTree.begin(), sptTree.end(), -1);
	std::fill(dist_to_dest.begin(), dist_to_dest.end(), INF);

	src_tree = 0;
	dest_tree = all_vec.size() - 1;

	dist_to_dest[ dest_tree ] = 0;
	sptTree[ dest_tree ] = dest_tree;

	for(int i=all_vec.size()-1; i>=0; i--) if(!is_disable(all_vec[i]) and dist_to_dest[i] != INF){
		int x = all_vec[i];
		for(const auto &e: get_edges_reverse(x)) {
			int from = trans_id[e->from];
			int to = trans_id[e->to];

			ASSERT( x == e->from );
			if(dist_to_dest[from] + e->delay < dist_to_dest[to]) {
				dist_to_dest[to] = dist_to_dest[from] + e->delay;
				sptTree[to] = from;
			}
		}
	}
}

void Cache::super(PrefixNode* pfx) {

	int cur = pfx->to;
	while(cur != dest_tree) {
		for(auto &e: get_edges( all_vec[cur] )) {
			int from_id = trans_id[e->from];
			int to_id = trans_id[e->to];

			ASSERT(from_id == cur);
			if(sptTree[from_id] == to_id or is_disable(e->to) or dist_to_dest[to_id] == INF) continue;

			float delta = -dist_to_dest[from_id]+dist_to_dest[to_id]+e->delay;
			PrefixNode* node = new PrefixNode(pfx, from_id, to_id, e->delay, delta);
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
		path->delay.push_back(  dist_to_dest[cur]-dist_to_dest[sptTree[cur]]);
		cur = sptTree[cur];
	}
	path->path.push_back( all_vec[dest] );
}

const vector<Cache_Edge*> Cache::get_edges(int x) {
	int level = bc_map->level[x];
	return nodes[ level_to_nodes[level] ]->get_valid_edge(x);
}

const vector<Cache_Edge*> Cache::get_edges_reverse(int x) {
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

	cout << "All Vec:\n";
	for(const auto& x: all_vec) {
		cout << bc_map->get_node_name(x) << "(" << bc_map->level[x] << "), ";
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

void Path::output(ostream &fout, Graph *graph) const {
	int len = path.size();

	fout << "Endpoint:   "; print_name(fout, graph->nodes[ graph->bc_map->get_graph_id(path[len-2]) ].name ); fout << '\n';
	fout << "Beginpoint: "; print_name(fout, graph->nodes[ graph->bc_map->get_graph_id(path[1]) ].name ); fout << '\n';
	fout << "= Required Time               " << delay.back() << '\n';
	fout << "- Arrival Time                " << delay.back() - dist<< '\n';
	fout << "= Slack Time                  " << dist << "\n";

	const string tab = "      ";
	const char type_sym[] = { '^', 'v' };
	fout << tab << "----------------------------------------\n";
	fout << tab << "Delay     Arrival    Edge  Pin\n";
	fout << tab << "          Time\n";
	fout << tab << "----------------------------------------\n";

	// output control
	fout << std::left << std::fixed << std::setprecision(OUTPUT_PRECISION);
	float at = 0;
	for(size_t i=1; i<path.size()-1; i++) {
		int g_id = graph->bc_map->get_graph_id( path[i] );
		Transition_Type type = graph->bc_map->get_graph_id_type( path[i] );
		at += -delay[i-1];

		fout << tab; fout << std::setw(10);
		if( i == 1 ) fout << "-";
		else fout << -delay[i-1];

		fout << std::setw(11); fout <<  at;
		fout << std::setw(6); fout << type_sym[type];
		print_name(fout, graph->nodes[g_id].name ); fout << '\n';
	}
	fout << tab << "----------------------------------------\n";

	fout << '\n';
}

