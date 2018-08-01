#include "cache.h"

Cache::Cache(BC_map* map):bc_map(map) {  }

void Cache::clear() {
	nodes.clear();

	level_to_nodes.clear();
	level_to_nodes.resize(bc_map->max_level+3);

	marked_level.clear();
	marked_level.resize(bc_map->max_level+3);

	//next_level.clear();
	//next_level.resize(bc_map->max_level+3);

	next_object.clear();
	next_object.resize(bc_map->max_level+3);

	disable.clear();
	through.clear();
	through.resize(bc_map->max_level+3);

	for(auto &x: all_vec) {
		G[x].clear();
		Gr[x].clear();
	}
	G.resize(bc_map->num_node+3);
	Gr.resize(bc_map->num_node+3);

	trans_id.resize(bc_map->num_node+3);
	all_vec.clear();
	dist_to_dest.clear();
	sptTree.clear();

	vis.resize(bc_map->num_node+3);
	for(auto& x: visited_points) vis[x] = 0;
	visited_points.clear();
}

void Cache::add_cache_node(CacheNode* node,int start_level, int end_level) {
	nodes.emplace_back(node);
	marked_level[ node->source_level ] = 1;
	//next_level[start_level] = end_level;

	for(int i = start_level+1; i<=end_level; i++)
		level_to_nodes[i] = nodes.size() - 1;
}

void Cache::set_disable(const vector<int>& dis) {
	for(auto& x: dis) disable.emplace(x, true);
}

void Cache::set_through(const vector<int>& thr) {
	for(auto& x: thr) {
		through[ bc_map->level[x] ].push_back(x);
	}
}

void Cache::update_cacheNode() {
	for(size_t i=0; i<nodes.size(); i++) nodes[i]->update();
}

bool Cache::is_disable(int x) {
	auto it = disable.find(x);
	if(it==disable.end()) return false;
	return it->second;
}

bool Cache::is_valid_edge(int from, int to) {
	if(is_disable(from) or is_disable(to)) return false;
	int level_from = bc_map->level[from];
	int level_to = bc_map->level[to];
	int node_to = level_to_nodes[level_to];
	int node_from = level_to_nodes[level_from];

	if(marked_level[level_from] or marked_level[level_to]) {
		if(marked_level[level_from] and marked_level[level_to]) {
			// avoid jumping through points ==> check_through will do this
			//if(next_level[level_from] != level_to) {
				//Logger::add_record("next level good", 1);
				//return false;
			//}
			ASSERT(nodes[node_from]->is_valid_point(from));
			return nodes[node_to]->is_valid_point(from) and nodes[node_to]->is_valid_point(to);
		}
		else if(marked_level[level_from]) {
			//ASSERT(node_from + 1 == node_to); 
			ASSERT(nodes[node_from]->is_valid_point(from));
			if(node_from + 1!=node_to) return false;

			return nodes[node_to]->is_valid_point(from) and nodes[node_to]->is_valid_point(to);
		}
		else {
			ASSERT(marked_level[level_to]);
			//ASSERT(node_to == node_from);
			if(node_to != node_from) {
				return false;
				//cout << bc_map->get_node_name(from) << " " << bc_map->get_node_name(to) << '\n';
				//cout << level_from << " " << level_to << '\n';
				//print();
				//exit(-1);
			}
			ASSERT(nodes[node_from]->is_valid_point(from));
			ASSERT(node_from+1 < (int)nodes.size());
			return nodes[node_from]->is_valid_point(to) and nodes[node_from+1]->is_valid_point(to);
		}
	}
	else {
		if(node_to != node_from) return false;

		ASSERT(nodes[node_to]->is_valid_point(from));
		return nodes[node_to]->is_valid_point(to) ;
	}
}

const vector<int>& Cache::get_kth_src() {
	return nodes.front()->get_kth_src();
}

const vector<int>& Cache::get_kth_dest() {
	return nodes.back()->get_kth_dest();
}

void Cache::add_edge(int from, int to, float delay) {
	G[from].emplace_back(from, to, delay);
	Gr[to].emplace_back(to, from, delay);
}

void Cache::connect_pseudo_edge_source() {
	for(auto &x: get_kth_src()) {
		int g_id = bc_map->get_graph_id(x);
		Mode mode = bc_map->get_graph_id_mode(x);
		Transition_Type type = bc_map->get_graph_id_type(x);

		add_edge(pseudo_src, x, -bc_map->graph->nodes[g_id].at[mode][type]);
		all_vec.push_back(x);
	}
}

void Cache::connect_pseudo_edge_dest() {
	for(auto &x: get_kth_dest()) {
		int g_id = bc_map->get_graph_id(x);
		Mode mode = bc_map->get_graph_id_mode(x);
		Transition_Type type = bc_map->get_graph_id_type(x);

		add_edge(x, pseudo_dest, bc_map->graph->nodes[g_id].rat[mode][type]);
		all_vec.push_back(x);
	}
}

/* build valid graph */
void Cache::build_graph() {
	int cur = 0;
	for(int i=0; i<(int)through.size(); i++) {
		if(through[i].size()) {
			for(int j=cur; j<i; j++) next_object[j] = i;
			cur = i;
		}
	}

	for(int j=cur; j<bc_map->max_level; j++) next_object[j] = -1;
	pseudo_src = 0;
	pseudo_dest = bc_map->num_node;

	connect_pseudo_edge_source();
	connect_pseudo_edge_dest();
	all_vec.push_back(pseudo_src);
	all_vec.push_back(pseudo_dest);

	for(auto &x: get_kth_src()) build_graph(x);

	std::sort(all_vec.begin(), all_vec.end(), [&](int v1, int v2){
			return bc_map->level[v1] < bc_map->level[v2];
	});

	all_vec.resize( std::unique(all_vec.begin(), all_vec.end()) - all_vec.begin() );

}

void Cache::kth(vector<Path*>& ans, int k) {

/* build shortest path tree */
	build_SPT();

/*do kth*/
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

		trash_can.push_back(pfx);
	}

	for(auto &x: trash_can) delete x;
	while(kth_Q.size()) {
		delete kth_Q.top();
		kth_Q.pop();
	}
}

bool Cache::check_through(int from, int to, unordered_map<int,bool>& pass_nodes) {
	int from_level = bc_map->level[from]; 
	int to_level = bc_map->level[to];

	// there is no through betwen level from and to
	if(next_object[from_level] == next_object[to_level] and through[from_level].size() == 0 and through[to_level].size() == 0) {
		return true; 
	}

	for(int i=from_level; i<=to_level; i++) {
		if(through[i].size() != 0) {
			int find = false;
			for(auto& x: through[i]) {
				//if(pass_nodes[x]==1) find = true;
				if(pass_nodes.find(x) != pass_nodes.end()) find = true;
				if(x==from or x==to) find = true;
			}
			if(!find) return false;
		}
	}
	return true;
}

void Cache::build_graph(int x) {
	if(vis[x]) return;
	vis[x] = 1;
	visited_points.push_back(x);

	all_vec.push_back(x);
	//for(auto e: bc_map->G[x]) {
		//if(bc_map->condensed_by[ e->to ] != -1) {
			//Logger::add_record("wast time cache", 1);
			//continue;
		//}
		//if(is_valid_edge(e->from, e->to)) {
			//add_edge(e->from, e->to, e->delay);
			//build_graph(e->to);
		//}
	//}
	for(auto& e: bc_map->J[x]) {
		if(is_valid_edge(e->from, e->to) and check_through(e->from, e->to, bc_map->nodes_on_jump[e->nodes_on_jump_id])) {
			add_edge(e->from, e->to, e->delay);
			build_graph(e->to);
		}
	}
}

void Cache::build_SPT() {

	for(size_t i=0; i<all_vec.size(); i++) trans_id[ all_vec[i] ] = i;
	dist_to_dest.resize(all_vec.size() + 2);
	sptTree.resize(all_vec.size() + 2);

	for(size_t i=0; i<all_vec.size(); i++) {
		sptTree[i] = -1;
		dist_to_dest[i] = INF;
	}
	//std::fill(sptTree.begin(), sptTree.end(), -1);
	//std::fill(dist_to_dest.begin(), dist_to_dest.end(), INF);

	src_tree = 0;
	dest_tree = all_vec.size() - 1;

	dist_to_dest[ dest_tree ] = 0;
	sptTree[ dest_tree ] = dest_tree;

	for(int i=all_vec.size()-1; i>=0; i--) if(!is_disable(all_vec[i]) and dist_to_dest[i] != INF){
		int x = all_vec[i];
		for(const auto &e: Gr[x]) {
			int from = trans_id[e.from];
			int to = trans_id[e.to];

			ASSERT( x == e.from );
			//ASSERT(all_vec[from] == e.from);
			//ASSERT(all_vec[to] == e.to);

			if(dist_to_dest[from] + e.delay < dist_to_dest[to]) {
				dist_to_dest[to] = dist_to_dest[from] + e.delay;
				sptTree[to] = from;
			}
		}
	}
}

void Cache::super(PrefixNode* pfx) {

	int cur = pfx->to;
	while(cur != dest_tree) {
		for(auto &e: G[ all_vec[cur] ]) {
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
		//cout << "-> " << bc_map->get_node_name(all_vec[cur]) << '\n';
		path->delay.push_back( dist_to_dest[cur]-dist_to_dest[sptTree[cur]] );
		cur = sptTree[cur];
	}
		//cout << "-> " << bc_map->get_node_name(all_vec[dest])<< '\n';
	path->path.push_back( all_vec[dest] );
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

void Cache::print() {
	//cout << "level to nodes\n";
	//for(int i=0; i<level_to_nodes.size(); i++) {
	//cout << i << " : " << level_to_nodes[i] << '\n';
	//} 

	cout << "----------------------------------\n" << std::flush;
	//cout << "Disable:\n";
	//for(const auto& p: disable)
		//cout << bc_map->get_node_name(p.first) << '\n';

	cout << "Marked level\n";
	for(int i=0; i<bc_map->max_level; i++) {
		if(marked_level[i]) cout << "	level " << i << '\n';
	}
	cout << "level_to_nodes:\n";
	for(int i=0; i<bc_map->max_level; i++) {
		cout << "	level " << i << ":";
		cout << level_to_nodes[i];
		cout << " " << next_object[i];
		cout <<'\n';
	}
	for(size_t i=0; i<nodes.size(); i++) {
		cout << "[+] cache " << i << " : ";
		nodes[i]->print();
	}
	cout << "Valid points:\n";
	for(auto& x: all_vec) {
		cout << "	" << bc_map->get_node_name(x) << "(";
		cout << bc_map->level[x] << ") ";
		cout << dist_to_dest[ trans_id[x] ] << ' ';
		if(sptTree[trans_id[x]]!=-1) {
			cout << " parent = " << bc_map->get_node_name( all_vec[ sptTree[trans_id[x]] ] );
		}
		cout << '\n';
	}
	cout << "Valid edge: \n";
	for(auto& x:all_vec) {
		for(auto&e : G[x]) {
			cout << "	" << bc_map->get_node_name(e.from) << "->" << bc_map->get_node_name(e.to) << " " << e.delay<<'\n';
		}
	}

	cout << "Valid reverse edge: \n";
	for(auto& x:all_vec) {
		for(auto&e : Gr[x]) {
			cout << "	" << bc_map->get_node_name(e.from) << "->" << bc_map->get_node_name(e.to) << " " << e.delay<<'\n';
		}
	}
	cout << "src_tree = " << bc_map->get_node_name(all_vec[src_tree]) << '\n';
	cout << "dest_tree= " << bc_map->get_node_name(all_vec[dest_tree]) << '\n';
	cout << "Through: \n";
	for(int i=0; i<bc_map->max_level; i++) {

		if(through[i].size()) {
			cout << "	level: " << i << '\n';
			for(auto& x: through[i]) cout << bc_map->get_node_name(x) << " ";
			cout << '\n';
		}
	}
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

	auto print_node_detail = [&](int x, float delay) {
			int g_id = graph->bc_map->get_graph_id( x );
			Transition_Type type = graph->bc_map->get_graph_id_type( x );
			at += -delay;

			if( delay ==  INF) buf.addchar('-');
			else print_float(-delay, buf);
			buf.addchar(' ');

			print_float(at, buf); buf.addchar(' ');

			buf.addchar(type_sym[type]); buf.addchar(' ');

			print_name(graph->nodes[g_id].name, buf);  buf.addchar('\n');

	};

	auto bc_map = graph->bc_map; for(size_t i=1; i<path.size()-1; i++) {
		int x = path[i];

		if(i==1) {
			print_node_detail(x, delay[i-1]);
			continue;
		}

		bool is_jump = true;
		vector<pair<int,float>> nodes;

		for(const auto&e : bc_map->Jr[x]) {
			if( e->to == path[i-1] and fabs(e->delay -delay[i-1])<0.001 ){
				if( bc_map->nodes_on_jump[e->nodes_on_jump_id].size() == 0 ) is_jump = false;
				else {
					for(const auto& ge: bc_map->Gr[x]) {
						if( bc_map->nodes_on_jump[e->nodes_on_jump_id].find(ge->to) != bc_map->nodes_on_jump[e->nodes_on_jump_id].end() ) {
							auto cur_e = ge;
							int root = path[i-1];
							while(true) {
								nodes.emplace_back(cur_e->from, cur_e->delay);
								if(cur_e->to == root) break;
								cur_e = bc_map->Gr[cur_e->to].front();
							}
							break;
						}
					}
				}
				break;
			}
		}
		if(!is_jump) print_node_detail(x, delay[i-1]);
		else {
			for(int j=nodes.size()-1; j>=0; j--) {
				print_node_detail( nodes[j].first, nodes[j].second );
			}
		}
	}
	buf.addchar('\n');
}
