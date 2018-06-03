#include "kth.h"

// **************************
// *       Interface        *
// **************************

Kth::Kth(BC_map *_map, CPPR *_cppr, Graph *_graph){
    this -> bc_map = _map;
    this -> cppr = _cppr;
    this -> graph = _graph;
}

void Kth::clear() {
    dist.clear();
    dest.clear();
    LUT.clear();
    successor.clear();
    topo_order.clear();
    pseudo_src_edge.clear();
    pseudo_dest_edge.clear();
    while (!pq.empty()) {
        Prefix_node *tmp = pq.top();
        pq.pop();
        delete tmp;
    }
    for (Prefix_node *tmp : trash_can) delete tmp;
    trash_can.clear();
}

void Kth::KSP_to_destination(int destination, int k, vector<Path*> &result_container) {
    result_container.clear();
    this -> clear();
    this -> from_src = false;
    this -> cppr_on = true;
    this -> dest.push_back(destination);
    // cout << graph->nodes[bc_map->get_graph_id(dest)].name << " as destination\n";
    KSP(k, result_container, bc_map -> G, bc_map -> Gr);
    for(auto &x : result_container) {
        /* Without reversing pseudo source, we can see it as pseudo destination. */
		std::reverse( x->delay.begin(), x->delay.end() );
		std::reverse( x->path.begin(), prev(x->path.end()) );
	}
}

void Kth::KSP_from_source(int source, int k, vector<Path*> &result_container) {
    result_container.clear();
    this -> clear();
    this -> dest.push_back(source);
    this -> from_src = true;
    this -> cppr_on = true;
    // cout << graph->nodes[bc_map->get_graph_id(src)].name << " as source\n";
    KSP(k, result_container, bc_map -> Gr, bc_map -> G);
}

void Kth::KSP_without_CPPR(vector<int> &destinations, int k, vector<Path*> &result_container) {

    result_container.clear();
    this -> clear();
    this -> cppr_on = false;
    this -> from_src = false;
    this -> dest = destinations;
    KSP(k, result_container, bc_map -> G, bc_map -> Gr);
    for(auto &x : result_container) {
        /* Without reversing pseudo source, we can see it as pseudo destination. */
		std::reverse( x->delay.begin(), x->delay.end() );
		std::reverse( x->path.begin(), prev(x->path.end()) );
	}
}

// *********************************************
// *       kth algorithm implementation        *
// *********************************************

int Kth::set_id(int v) {
    /* If "v" is in "bc_to_kth" table, return its id.
       otherwise, return after add it.                */

    auto it = LUT.find(v);
    if (it == LUT.end()) {
        int id = LUT.size();
        LUT.emplace(v, id);
        successor.emplace_back(NOT_VISITED);
        dist.emplace_back(INF);
        return id;
    }
    else {
        return it -> second;
    }
}

void Kth::get_topological_order(int v, const vector<vector<Edge*>> &radj) {
    /* A DFS program to construct topological order
       Use "successor" array to store visited information in this function. */

    successor[ set_id(v) ] = VISITED;
    for (const auto &p_e : radj[v]) {
		const auto& edg = *p_e;
		// cout << bc_map->get_node_name(v) << " " << bc_map->get_node_name(edg.to) << " " << edg.valid << '\n';
		//if (!edg.valid) continue;
		if (!this->bc_map->current_cache->get_edge_valid(edg.id) ) continue;
        // std::cerr << graph->get_name(bc_map->get_graph_id(edg.from)) << " " << graph->get_name(bc_map->get_graph_id(edg.to)) << edg.delay << "\n";
        int id = set_id(edg.to);
        if (successor[id] == NOT_VISITED) {
            get_topological_order(edg.to, radj);
        }
    }
    this -> topo_order.push_back(v);
}

bool Kth::build_SDSP_tree(const vector<vector<Edge*>> &radj) {
	/*
        Algorithm:
            Use **REVERSE** adjacency list, build a single source destination tree to "dest".
            Note that we only traverse the valid vertices marked in BC_map.
            All tables (dist, successor ...) is initially empty,
             vertices are added to table only when it is traversed,
             so that we can reduce the search space.

        Return false if no possible source vertex.
    */

	/* Create an entry for dest, and do initialization.
       Here, we use "successor" array to store visited information.
       We also collect the possible source vertices in this part.   */
    for (int d : this->dest) {
        int dest_id = set_id(d);
        if (successor[dest_id] == NOT_VISITED) {
            get_topological_order(d, radj);
        }
    }

    /* Create pseudo destination, and then update distance of the real destinations. */
    if (this -> from_src) {
        for (int d : this->dest) {
            int src_gid = bc_map -> get_graph_id(d);
            Mode src_mode = bc_map -> get_graph_id_mode(d);
            Transition_Type src_type = bc_map -> get_graph_id_type(d);
            float at = graph->nodes[src_gid].at[src_mode][src_type];
            pseudo_dest_edge.emplace_back(d, pseudo_dest, -at);

            int id = LUT[d];
            successor[id] = pseudo_dest;
            dist[id] = -at;
        }
    }
    else {
        for (int d : this->dest) {
            int dest_gid = bc_map -> get_graph_id(d);
            Mode dest_mode = bc_map -> get_graph_id_mode(d);
            Transition_Type dest_type = bc_map -> get_graph_id_type(d);
            float rat = graph->nodes[dest_gid].rat[dest_mode][dest_type];
            // std::cerr << "[KSP] Create an edge for D " << graph->nodes[dest_gid].name << ", where RAT = " << rat << "\n";
            pseudo_dest_edge.emplace_back(d, pseudo_dest, rat);

            int id = LUT[d];
            successor[id] = pseudo_dest;
            dist[id] = rat;
        }
    }


    /* A DP-based algorithm to calculate "dist" and "succssor" array. */
    /******************************************************************
     *    Possible improvement: Maybe store edges in sorted order?    *
     ******************************************************************/
    int id = set_id(pseudo_dest);
    dist[id] = 0;
    successor[id] = pseudo_dest;
    for (int i=topo_order.size()-1; i>=0; i--) {
        int v = topo_order[i], v_id = LUT[v];
        if (radj[v].size() == 0) {
            /* The calculation of delay will be later. */
            this->pseudo_src_edge.emplace_back(pseudo_src, v, 0.0);
            continue;
        }
        for (const auto &p_edg : radj[v]) {
			const auto &edg = *p_edg;
			if (!this->bc_map->current_cache->get_edge_valid(edg.id) ) continue;
            int to_id = LUT[edg.to];
            float relax = dist[v_id] + edg.delay;
            if (relax < dist[to_id]) {
                successor[to_id] = v;
                dist[to_id] = relax;
            }
        }
    }

    /* Return false if given destination is not reachable. */
    if (this -> pseudo_src_edge.empty()) {
        std::cerr << "[Error] In KSP, none of the given destination is reachable.\n";
        return false;
    }

    /* Create pseudo source: kth_id = N, bc_id = -1. */
    id = set_id(pseudo_src);
    if (this -> from_src) {
        for (auto &edg : pseudo_src_edge) {
            int dest_gid = bc_map -> get_graph_id(edg.to);
            Mode dest_mode = bc_map -> get_graph_id_mode(edg.to);
            Transition_Type dest_type = bc_map -> get_graph_id_type(edg.to);
            float rat = graph->nodes[dest_gid].rat[dest_mode][dest_type];
            float cpp = 0.0;
            if (cppr_on) {
                int src_gid = bc_map -> get_graph_id(dest[0]);
                Mode src_mode = bc_map -> get_graph_id_mode(dest[0]);
                Transition_Type src_type = bc_map -> get_graph_id_type(dest[0]);
                cpp = cppr->cppr_credit(src_mode, src_gid, src_type, dest_gid, dest_type);
            }
            edg.delay = rat + cpp;

            float relax = dist[ LUT[edg.to] ] + edg.delay;
            if (relax < dist[id]) {
                dist[id] = relax;
                successor[id] = edg.to;
            }
        }
    }
    else {
        for (auto &edg : pseudo_src_edge) {
            int src_gid = bc_map -> get_graph_id(edg.to);
            Mode src_mode = bc_map -> get_graph_id_mode(edg.to);
            Transition_Type src_type = bc_map -> get_graph_id_type(edg.to);
            float at = graph->nodes[src_gid].at[src_mode][src_type];
            float cpp = 0.0;
            if (cppr_on) {
                int dest_gid = bc_map -> get_graph_id(dest[0]);
                Transition_Type dest_type = bc_map -> get_graph_id_type(dest[0]);
                cpp = cppr->cppr_credit(src_mode, src_gid, src_type, dest_gid, dest_type);
            }
            edg.delay = -at + cpp;
            // std::cerr << "[KSP] Create an edge for CK " << graph->nodes[src_gid].name << ", where AT = " << at << " " << cpp << "\n";

            float relax = dist[ LUT[edg.to] ] + edg.delay;
            if (relax < dist[id]) {
                dist[id] = relax;
                successor[id] = edg.to;
            }
        }
    }

	return true;
}

void Kth::queueing(Prefix_node *path) {
    /* A function to push something into priority queue
       This function exist because there will be more optimization done before pushing into queue. */
    if (path -> delta + dist[ LUT[pseudo_src] ] < bc_map -> threshold) {
        pq.push(path);
    }
}

void Kth::extend(Prefix_node *path, const vector<vector<Edge*>> &adj) {
	/* From "path", extend its childs
	   A child can only be obtained by adding a sidetrack edge,
        whose "from" after last sidetrack of this path.         */

	/* For every v from head of last sidetrack to t. */
	int v = path -> parent ? path -> last.to : pseudo_src;

    /* Generate all children paths, note that when we do forward searching,
        not all valid edges are in our search space. Do LUT lookup to check.  */
	while (v != pseudo_dest) {
        int v_id = LUT[v];
        if (v == pseudo_src) {
            for (const Edge &edg : pseudo_src_edge) {
                if (edg.to == successor[v_id]) continue;
                Prefix_node *next_path = new Prefix_node(this, path, edg.from, edg.to, edg.delay);
                queueing(next_path);
            }
        }
        else {
            for (const auto& p_edg : adj[v]) {
				const Edge& edg = *p_edg;
                if (LUT.find(edg.to) == LUT.end() || edg.to == successor[v_id]) continue;
    			Prefix_node *next_path = new Prefix_node(this, path, edg.from, edg.to, edg.delay);
                queueing(next_path);
    		}
        }

		v = successor[ v_id ];
	}
}

void Kth::get_explicit_path_helper(Path *exp_path, const Prefix_node *imp_path, int end) {
	/* Recursive subroutine of get_explicit_path
	   *** Assume "end" havn't been added into path *** */
	int sz = exp_path->path.size(), dsz = exp_path->delay.size(), v;
	if (imp_path -> parent == NULL) {
		/* Root of prefix tree */
		v = this -> pseudo_src;
	}
	else {
		/* From where last sidetrack points to. */
		v = imp_path -> last.to;
		exp_path -> delay.emplace_back(imp_path -> last.delay);
	}

	/* Go through all vertices from v to dest */
	while (v != end) {
        int v_id = LUT[v];
		exp_path -> path.emplace_back(v);
		exp_path -> delay.emplace_back(dist[ v_id ] - this->dist[ LUT[this->successor[v_id]] ]);
		v = this -> successor[ v_id ];
	}
    if (v != pseudo_dest) exp_path->path.emplace_back(v);

	std::reverse(exp_path->path.begin() + sz, exp_path->path.end());
	std::reverse(exp_path->delay.begin() + dsz, exp_path->delay.end());

	if (imp_path -> parent != NULL) {
		get_explicit_path_helper(exp_path, imp_path -> parent, imp_path -> last.from);
	}
}

void Kth::get_explicit_path(Path *exp_path, const Prefix_node *imp_path) {
	/* Recover path from implicit representation to explicit representaion
	   Store in exp_path, path will be in reverse order.
	   IMPORTANT, the vetices are using BC id.                                   */

	exp_path -> dist = dist[ LUT[pseudo_src] ] + imp_path -> delta;
	exp_path -> path.clear();
    exp_path -> delay.clear();

	get_explicit_path_helper(exp_path, imp_path, pseudo_dest);
    // cout << "Find a path:"; for (int v : exp_path -> path) cout << v << " "; cout << "\n";
}

void Kth::KSP(int k, vector<Path*> &container, const vector<vector<Edge*>> &adj, const vector<vector<Edge*>> &radj) {
    /*
        Algorithm: Eppistein's k shortest path algorithm
            dist: Shortest distance to "dest".
            successor: Parent in shortest path tree.
            bc_to_kth: A hash table that maps vertices in search space to [0, N-1],
                        N is number of vertex in search space.
            bc_map -> threshold: Bounding condition, the path is considered possibly critical only if it is shorter than threshold
    */
	if (!build_SDSP_tree(radj)) return;
	container.resize(k);

	Prefix_node *root = new Prefix_node();
	if (dist[ LUT[pseudo_src] ] < bc_map -> threshold) this->pq.push(root);
	for (int i=0; i<k; i++) {
		if (pq.empty()) {
			container.resize(i);
			break;
		}
		Prefix_node *next_path = pq.top();
		pq.pop();
		this -> trash_can.emplace_back(next_path);

        if (next_path->delta + dist[ LUT[pseudo_src] ] >= bc_map -> threshold) {
            container.resize(i);
            break;
        }
        container[i] = new Path();
		get_explicit_path(container[i], next_path);
        if (i+1 < k) this -> extend(next_path, adj);
	}

	/* Clean nodes */
	for (Prefix_node *it : trash_can) {
		delete it;
	}
	trash_can.clear();
	while (!pq.empty()) {
		Prefix_node *next_path = pq.top();
		pq.pop();
		delete next_path;
	}
}

// *************************************
// *            Prefix_node            *
// *************************************

Kth::Prefix_node::Prefix_node(Kth *father, Prefix_node *p, int from, int to, float delay) {
	this -> parent = p;
	this -> last.from = from;
    this -> last.to = to;
    this -> last.delay = delay;
	this -> delta = p -> delta + father -> get_delta(last);
}

Kth::Prefix_node::Prefix_node() {
	/* An empty node is SP itself */
	this -> parent = NULL;
    this -> delta = 0;
}

bool Kth::Prefix_node::Compare::operator()(const Prefix_node *n1, const Prefix_node *n2) {
	/* This function is written for comparison in std::priority_queue
	   If we want to pick "worst" node, this comparison should return "is n1 better than n2" */

	return n1 -> delta > n2 -> delta;
}

/**********************************
 *      Output Formatting         *
 **********************************/

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
	/*
        Path are using BC id.
        The path are in normal order:
        source -> v1 -> v2 -> v3 -> v4 -> ... -> psuedo destination
    */
    BC_map *bc = graph->get_bc_map();

    int width = 8, n = path.size();
    float rat = delay[n - 1], slack = this->dist, at = rat - slack, total = 0;
    const char *tab = "      ", *spline = "----------------------------------------", *type_ch[2] = {"^   ", "v   "};

    // path[0] is SuperSrc
    if (path.empty()) return;
	fout << "Endpoint:   ";
    print_name(fout, graph->nodes[bc->get_graph_id(path[n-2])].name);
    fout << '\n';
	fout << "Beginpoint: ";
    print_name(fout, graph->nodes[bc->get_graph_id(path[0])].name);
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
    << "   " << type_ch[bc->get_graph_id_type(path[0])] << "  ";
    print_name(fout, graph->nodes[bc->get_graph_id(path[0])].name);
    // if(mark[n-2]) fout << " ->";
    fout << '\n';

    /* Output remaining */
	for (int i = 1; i<n-1; i++) {
		total -= delay[i]; // Delay is negative
		fout << tab << std::left << std::fixed << std::setprecision(OUTPUT_PRECISION) << std::setw(width) << -delay[i] << "  ";
		fout        << std::left << std::fixed << std::setprecision(OUTPUT_PRECISION) << std::setw(width) << total << "   ";
		fout << type_ch[bc->get_graph_id_type(path[i])] << "  ";
        print_name(fout, graph->nodes[bc->get_graph_id(path[i])].name);
        // if(mark[i] ) fout << " ->";
        fout << '\n';
	}
	fout << tab << spline << '\n' << '\n';
}

void Path::print() {
	LOG(CERR) << "Path:\nEnd at " << path[path.size()-2] << '\n';
	for (int i=0; i<(int)path.size(); i++) {
		LOG(CERR) << "delay " << std::setw(6) << delay[i-1] << " from " << this->path[i] << '\n';
	}
	LOG(CERR) << "Length: " << this->dist << "\n\n";
}
