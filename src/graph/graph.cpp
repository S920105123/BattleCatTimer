#include "graph.h"

// ******************************************************
// ***                 Node related                   ***
// ******************************************************

Graph::Node::Node(int index, const string &name, Node_type type) {
	this->exist     = true;
	this->name      = name;
	this->index     = index;
	this->tree      = NULL;
	this->node_type = type;
	this->is_clock  = false; // Assume it is false at first, manually set later.

	// These undefined values is defined in header.h
	for (int i=0; i<2; i++) {
		for (int j=0; j<2; j++) {
			Mode mode = MODES[i];
			Transition_Type type = TYPES[j];

			this->slack[mode][type] = UNDEFINED_SLACK[mode];
			this->slew[mode][type]  = UNDEFINED_SLEW[mode];
			this->rat[mode][type]   = UNDEFINED_RAT[mode];
			this->at[mode][type]    = UNDEFINED_AT[mode];
			this->launching_clk[mode][type] = -1; // Undefined
		}
	}
}

int Graph::add_node(const string &name, Node_type type) {
	int id = this->next_id++;
	this->trans.insert(make_pair(name, id));
	this->nodes.emplace_back(id, name, type);
	this->adj.emplace_back();
	this->rev_adj.emplace_back();
	return id;
}

int Graph::get_index(const string &name) {
	// Get index of a cell:pin name
	// If it is not exist, automatically add one
	auto it = this->trans.find(name);
	if (it == this->trans.end()) {
		return this->add_node(name, INTERNAL);
	} else {
		return it->second;
	}
}

bool Graph::in_graph(int index) const {
	return index < (int)this->nodes.size() && this->nodes[index].exist;
}

bool Graph::in_graph(const string &name) const {
	auto it = this->trans.find(name);
	return it != this->trans.end() && this->in_graph(it->second);
}

const string& Graph::get_name(int index) const {
	static const string EMPTY = string("");
	if (index > (int)this->nodes.size() || this->nodes[index].name.empty()) {
		LOG(WARNING) << "[Graph] Illegal index " << index << " when get name. (Node does not exist)" << endl;
		return EMPTY;
	}
	return this->nodes[index].name;
}

void Graph::set_at(const string &pin_name, float early_at[], float late_at[]) {
	 // LOG(CERR) << "set_at(timing) " << pin_name << " " << early_at[RISE] << " " << early_at[FALL]
	 // << " " << late_at[RISE] << " " << late_at[FALL] << endl;
	// string handle = cell_pin_concat( INPUT_PREFIX, pin_name );
	string handle = pin_name;
	if (!in_graph(handle)) {
		LOG(ERROR) << "Try to modify arrival time of a node which is not primary input. (" << pin_name << ")" << endl;
		ASSERT_NOT_REACHED();
	}
	int index = this->get_index(handle);
	this->nodes[index].at[EARLY][RISE] = early_at[RISE];
	this->nodes[index].at[EARLY][FALL] = early_at[FALL];
	this->nodes[index].at[LATE][RISE]  = late_at[RISE];
	this->nodes[index].at[LATE][FALL]  = late_at[FALL];
}

void Graph::set_rat(const string &pin_name, float early_rat[2], float late_rat[2]) {
	 // LOG(CERR) << "set_rat " << pin_name << " " << early_rat[RISE] << " " << early_rat[LATE]
	 // << " " << late_rat[RISE] << " " << late_rat[LATE] << endl;

	// string handle = cell_pin_concat( OUTPUT_PREFIX, pin_name );
	string handle = pin_name;
	if (!in_graph(handle)) {
		LOG(ERROR) << "Try to modify required arrival time of a node which is not primary output. (" << pin_name << ")"  << endl;
		ASSERT_NOT_REACHED();
	}
	int index = this->get_index(handle);
	this->nodes[index].rat[EARLY][RISE] = early_rat[RISE];
	this->nodes[index].rat[EARLY][FALL] = early_rat[FALL];
	this->nodes[index].rat[LATE][RISE]  = late_rat[RISE];
	this->nodes[index].rat[LATE][FALL]  = late_rat[FALL];
}

void Graph::set_at(const string &pin_name, Mode mode, Transition_Type transition, float val){
	 // LOG(CERR) << "set_at " << pin_name << " " << get_mode_string(mode) << " "
	 // << get_transition_string(transition) << " " << val << endl;

	// string handle = cell_pin_concat( INPUT_PREFIX, pin_name );
	string handle = pin_name;
	if (!in_graph(handle)) {
		LOG(ERROR) << "Try to modify arrival time of a node which is not primary input. (" << pin_name << ")" << endl;
		ASSERT_NOT_REACHED();
	}
	int index = this->get_index(handle);
	this->nodes[index].at[mode][transition] = val;
}

void Graph::set_rat(const string &pin_name, Mode mode, Transition_Type transition, float val){
	// LOG(CERR) << "set_rat " << pin_name << " " << get_mode_string(mode) << " "
	// << get_transition_string(transition) << " " << val << endl;

	// string handle = cell_pin_concat( OUTPUT_PREFIX, pin_name );
	string handle = pin_name;
	if (!in_graph(handle)) {
		LOG(ERROR) << "Try to modify rat of a node which is not primary ouput. (" << pin_name << ")" << endl;
		ASSERT_NOT_REACHED();
	}
	int index = this->get_index(handle);
	this->nodes[index].rat[mode][transition] = val;
}

void Graph::set_slew(const string &pin_name, Mode mode, Transition_Type transition, float val){
	// LOG(CERR) << "set_slew " << pin_name << " " << get_mode_string(mode) << " "
	// << get_transition_string(transition) << " " << val << endl;

	// string handle = cell_pin_concat( INPUT_PREFIX, pin_name );
	string handle = pin_name;
	if (!in_graph(handle)) {
		LOG(ERROR) << "Try to modify slew of a node which is not primary input. (" << pin_name << ")" << endl;
		ASSERT_NOT_REACHED();
	}
	int index = this->get_index(handle);
	this->nodes[index].slew[mode][transition] = val;
}

void Graph::set_slew(const string &pin_name, float early_slew[2], float late_slew[2]){
	// LOG(CERR) << "set_slew " << pin_name << " " << early_slew[RISE] << " " << early_slew[FALL]
	// << " " << late_slew[RISE] << " " << late_slew[FALL] << endl;

	// string handle = cell_pin_concat( INPUT_PREFIX, pin_name );
	string handle = pin_name;
	if (!in_graph(handle)) {
		LOG(ERROR) << "Try to modify slew time of a node which is not primary output. (" << pin_name << ")"  << endl;
		ASSERT_NOT_REACHED();
	}
	int index = this->get_index(handle);
	this->nodes[index].slew[EARLY][RISE] = early_slew[RISE];
	this->nodes[index].slew[EARLY][FALL] = early_slew[FALL];
	this->nodes[index].slew[LATE][RISE]  = late_slew[RISE];
	this->nodes[index].slew[LATE][FALL]  = late_slew[FALL];
}

void Graph::set_clock(const string& pin_name,float period, float low){
	// LOG(CERR) << "set_clock " << pin_name << " " << period << " " << low << endl;

	clock_T = period;
	clock_id = get_index(pin_name);
}

void Graph::set_load(const string& pin_name, float cap){
	// LOG(CERR) << "set_load " << pin_name << " " << cap << endl;

	int id = get_index(pin_name);
	if(!in_graph(id)){
		LOG(CERR) << "[Graph][set_load] no such pin: " << pin_name << endl;
		return;
	}
	out_load[pin_name] = cap;
	RCTree* tree = nodes[id].tree;
	if(tree==NULL) return;
	tree->add_pin_cap(pin_name, cap);
	tree->cal();
}

float Graph::get_at(const string &pin_name, Mode mode, Transition_Type transition){
//	 LOG(CERR) << "report_at " << pin_name << " " << get_mode_string(mode) << " " <<
//	 get_transition_string(transition) << endl;

	if (!in_graph(pin_name)){
		LOG(WARNING) << "[Graph][get_at] pin no exist , pin: " << pin_name << endl;
		return 0;
	}
	int index = this->get_index(pin_name);
	return this->nodes[index].at[mode][transition];
}

float Graph::get_rat(const string &pin_name, Mode mode, Transition_Type transition){
	// LOG(CERR) << "report_rat " << pin_name << " " << get_mode_string(mode) << " " <<
	// get_transition_string(transition) << endl;

	if (!in_graph(pin_name)){
		LOG(WARNING) << "[Graph][get_rat] pin no exist , pin: " << pin_name << endl;
		return 0;
	}
	int index = this->get_index(pin_name);
	return this->nodes[index].rat[mode][transition];
}

float Graph::get_slew(const string &pin_name, Mode mode, Transition_Type transition){
	// LOG(CERR) << "report_slew " << pin_name << " " << get_mode_string(mode) << " " <<
	// get_transition_string(transition) << endl;

	if (!in_graph(pin_name)){
		LOG(WARNING) << "[Graph][get_slew] pin no exist , pin: " << pin_name << endl;
		return 0;
	}
	int index = this->get_index(pin_name);
	return this->nodes[index].slew[mode][transition];
}

float Graph::get_slack(const string &pin_name, Mode mode, Transition_Type transition){
	// LOG(CERR) << "report_slack " << pin_name << " " << get_mode_string(mode) << " " <<
	// get_transition_string(transition) << endl;

	if (!in_graph(pin_name)){
		LOG(WARNING) << "[Graph][get_slack] pin no exist , pin: " << pin_name << endl;
		return 0;
	}
	int index = this->get_index(pin_name);
	return this->nodes[index].slack[mode][transition];
}

float Graph::get_cppr_credit(const string& pin1, const string& pin2, Transition_Type type1, Transition_Type type2, Mode mode){
	int id1 = get_index(pin1);
	int id2 = get_index(pin2);
	return cppr->cppr_credit(mode, id1, type1, id2, type2);
}

// ******************************************************
// ***                 Edge related                   ***
// ******************************************************

Graph::Edge::Edge(int src, int dest, Edge_type type) {
	this->from = src;
	this->to = dest;
	this->type = type;
}

Graph::Edge* Graph::add_edge(int src, int dest, Edge_type type) {
	Edge *eptr = new Edge(src, dest, type);
	this->adj[src].insert( {dest, eptr} );
	this->rev_adj[dest].insert( {src, eptr} );
//	LOG(CERR) << "An edge built from " << this->get_name(src)<< " to " << this->get_name(dest);
//	if (type == IN_CELL) {
//		LOG(CERR) << " (In cell edge).\n";
//	} else {
//		LOG(CERR) << " (RC tree edge).\n";
//	}
	return eptr;
}

Graph::Edge* Graph::get_edge(int src, int dest) const {
	auto it = this->adj[src].find(dest);
	if (it == this->adj[src].end()) {
		return NULL;
	}
	return it->second;
}

void Graph::add_arc(int src, int dest, TimingArc *arc, Mode mode) {
	// mode=0 -> eraly,    mode=1 -> late
	Edge *eptr = this->get_edge(src, dest);
	if (eptr == NULL) {
		eptr = this->add_edge(src, dest, IN_CELL);
	}
	eptr->arcs[mode].emplace_back(arc);
}

void Graph::add_constraint(int src, int dest, TimingArc *arc, Mode mode) {
	this->constraints.emplace_back(src, dest, arc, mode);

//	LOG(CERR) << "A constraint built from " << this->get_name(src)<< " to " << this->get_name(dest);
//	if (mode == EARLY) {
//		LOG(CERR) << " (Hold).\n";
//	} else {
//		LOG(CERR) << " (Setup).\n";
//	}
}

Graph::Constraint::Constraint(int from, int to, TimingArc *arc, Mode mode) {
	ASSERT(arc->is_constraint());
	this->from = from;
	this->to   = to;
	this->arc  = arc;
	this->mode = mode;
}

const vector< unordered_map< int, Graph::Edge* > >& Graph::adj_list() const {
	return this->adj;
}

// ******************************************************
// ***                 Wire related                   ***
// ******************************************************

Graph::Wire_mapping::Wire_mapping() {
	this->src=-1;
}

Graph::Wire_mapping* Graph::get_wire_mapping(const string &wire_name) {
	auto it = this->wire_mapping.find(wire_name);
	if (it == wire_mapping.end()) {
		Wire_mapping *mapping = new Wire_mapping();
		this->wire_mapping.emplace(wire_name, mapping);
		return mapping;
	}
	return it->second;
}

// ******************************************************
// ***                Graph related                   ***
// ******************************************************

Graph::Graph(){
	clock_id = -1;
	cppr = NULL;
}

Graph::~Graph(){
	if(cppr) delete cppr;
}

void Graph::build(Verilog &vlog, Spef &spef, CellLib &early_lib, CellLib &late_lib) {
	/* Build a graph via above list of terrible files */

	/* Initilize */
	this->next_id = 0;
	CellLib &lib = early_lib; // Two lib has the same topological structure.
	CellLib *lib_arr[2] = {&early_lib, &late_lib};

	/* Construct wire mapping */
	for (const string &wire_name : vlog.wire) {
		Wire_mapping *mapping = new Wire_mapping;
		mapping->src=-1;
		this->wire_mapping.emplace( wire_name, mapping );
	}

	/* Append each pin to the wire and construct internal timing arc  */
	for (const auto &p : vlog.gates) {
		const Verilog::Verilog_Gate *gt = p.second;
		const string &cell_type = gt->cell_type, cell_name = gt->cell_name;
		for (const pair<string,string> &io_pair : gt->param) {
			const string &pin_name = io_pair.first, &wire_name = io_pair.second;
			// Check is clock.
			int sink = this->get_index( cell_pin_concat(cell_name, pin_name) );
			if (lib.get_pin_is_clock(cell_type, pin_name)) {
				this->nodes[sink].is_clock = true;
				this->nodes[sink].launching_clk[EARLY][RISE] = this->nodes[sink].launching_clk[EARLY][FALL] =
				this->nodes[sink].launching_clk[LATE][RISE]  = this->nodes[sink].launching_clk[LATE][FALL]  = sink;
				this->clocks.push_back(sink);
			}

			Direction_type direction = lib.get_pin_direction(cell_type, pin_name);
			if (direction == OUTPUT) {
				// Construct in-cell timing arc.
				for (int mode=EARLY; mode<=LATE; mode++) {
					vector<TimingArc*> *arcs = lib_arr[mode]->get_pin_total_TimingArc(cell_type, pin_name);
					for (TimingArc *arc : *arcs) {
						int src = this->get_index( cell_pin_concat(cell_name, arc->get_related_pin()) );
						this->add_arc(src, sink, arc, (Mode)mode);
					}
				}

				// Complete wire mapping. Here "sink" is the output pin.
				Wire_mapping *mapping = this->get_wire_mapping(wire_name);
				ASSERT(mapping != NULL);
				ASSERT(mapping->src == -1);
				mapping->src = sink;
			} else {
				// Check for constraint edges
				for (int mode=EARLY; mode<=LATE; mode++) {
					vector<TimingArc*> *arcs = lib_arr[mode]->get_pin_total_TimingArc(cell_type, pin_name);
					for (TimingArc *arc : *arcs) {
						int src = this->get_index( cell_pin_concat(cell_name, arc->get_related_pin()) );
						this->add_constraint(src, sink, arc, (Mode)mode);
					}
				}

				// Complete wire mapping.
				Wire_mapping *mapping = this->get_wire_mapping(wire_name);
				ASSERT(mapping != NULL);
				mapping->sinks.emplace_back(sink);
			}
		}
	}

	/* Create nodes for primary input */
	for (const string &in_pin : vlog.input) {
		// int src = this->get_index( cell_pin_concat( INPUT_PREFIX, in_pin ) );
		int src = this->get_index( in_pin  );
		nodes[src].node_type = PRIMARY_IN;
		Wire_mapping *mapping = this->get_wire_mapping(in_pin);
		ASSERT(mapping != NULL);
		ASSERT(mapping->src == -1);
		mapping->src = src;
	}

	/* Create nodes for primary output */
	for (const string &out_pin : vlog.output) {
		// int sink = this->get_index( cell_pin_concat( OUTPUT_PREFIX, out_pin ) );
		int sink = this->get_index( out_pin );
		nodes[sink].node_type = PRIMARY_OUT;  // set node type
		Wire_mapping *mapping = this->get_wire_mapping(out_pin);
		mapping->sinks.emplace_back(sink);
	}

	/* Construct external timing arc through wire mapping */
	for (const auto &wire_pair : this->wire_mapping) {
		const string &wire_name = wire_pair.first;
		Wire_mapping *mapping = wire_pair.second;
		int from = mapping->src;
		if (from == -1) continue; // Isolated node.

		SpefNet *net = spef.get_spefnet_ptr(wire_name, 0);
		RCTree *tree = NULL;
		if (net!=NULL) tree = new RCTree(net, &vlog, lib_arr);
		else{
			// add new spef
			SpefNet *net = new SpefNet();
			net->set_total_cap(0);
			string root = get_name ( wire_mapping[wire_name]->src ), type="I", dir="O";
			Node_type node_type = nodes[get_index(root)].node_type;
			if(node_type==PRIMARY_IN)  type="P", dir="I";
			if(node_type==PRIMARY_OUT) type="P", dir="O";

			net->set_name(wire_name);
			net->add_conn(root, type, dir);
			for(auto &it: (*wire_mapping[wire_name]).sinks) {
				string name = get_name(it), type="I", dir="I";
				Node_type node_type = nodes[it].node_type;
				if(node_type==PRIMARY_IN)  type="P", dir="I";
				if(node_type==PRIMARY_OUT) type="P", dir="O";

				net->add_conn(name, type , dir);
				net->add_cap(name, 0);
				net->add_res(root, get_name(it), 0);
			}
			spef.add_net( wire_name, net);
			tree = new RCTree(net, &vlog, lib_arr);
		}
		tree->build_tree();
		tree->cal();
		nodes[from].tree = tree;
		for (int to : mapping->sinks) {
			nodes[to].tree = tree;
			Edge *eptr = add_edge(from, to, RC_TREE);
			eptr->tree = tree;
		}
	}

	/* Now all the arcs are constructed, set the rising/falling edge of each clock nodes */
	std::sort(clocks.begin(), clocks.end());
	for (vector<int>::iterator it = this->clocks.begin(); it != this->clocks.end(); ++it) {
		int clk = *it;
		ASSERT(!this->adj[clk].empty());
		Edge *eptr = this->adj[clk].begin()->second;
		ASSERT(!eptr->arcs[1].empty());
		if (eptr->arcs[1][0]->is_rising_triggered()) {
			this->nodes[clk].clk_edge = RISE;
		} else {
			ASSERT(eptr->arcs[1][0]->is_falling_triggered());
			this->nodes[clk].clk_edge = FALL;
		}
	}
}

// ******************************************************
// ***                 Arrival Time                   ***
// ******************************************************

void Graph::at_arc_update(int from, int to, TimingArc *arc, Mode mode) {
	/* Use "from" update "to" through "arc" */
	Node &node_from = this->nodes[from], &node_to = this->nodes[to];

	for (int i=0; i<2; i++) {
		for (int j=0; j<2; j++) {
			Transition_Type type_from = TYPES[i], type_to = TYPES[j];
			// No ++ in Transition_Type
			if (!arc->is_transition_defined(type_from, type_to)) continue;
			float cap_load = node_to.tree->get_downstream(mode, node_to.name);
			float input_slew = node_from.slew[mode][type_from];
			/* Try to update at */
			if (node_from.at[mode][type_from] != UNDEFINED_AT[mode]) {
				float delay = arc->get_delay(type_from, type_to, input_slew, cap_load);
				float new_at = node_from.at[mode][type_from] + delay;
				float &at = node_to.at[mode][type_to];
				if (at == UNDEFINED_AT[mode] || at_worse_than(new_at, at, mode)) {
					// Always choose worst at.
					at = new_at;
					if (!node_to.is_clock) {
						node_to.launching_clk[mode][type_to] = node_from.launching_clk[mode][type_from];
					}
				}
			}

			/* Try to update slew */
			if (input_slew != UNDEFINED_SLEW[mode]) {
				float new_slew = arc->get_slew(type_from, type_to, input_slew, cap_load);
				float &slew = node_to.slew[mode][type_to];
				if (slew == UNDEFINED_SLEW[mode] || slew_worse_than(new_slew, slew, mode)) {
					// Always choose worst slew.
					slew = new_slew;
				}
			}
		}
	}
}

void Graph::at_update(Edge *eptr) {
	/* Use eptr->from to relax eptr->to */
	int from = eptr->from, to = eptr->to;
	if (eptr->type == IN_CELL) {
		/* Update from in-cell timing arc */
		for (int i=0; i<2; i++) {
			Mode mode = MODES[i];
			for (TimingArc *arc : eptr->arcs[mode]) {
				at_arc_update(from, to, arc, mode);
			}
		}
	}
	else {
		/* Update from RC tree */
		Node &node_from = this->nodes[from], &node_to = this->nodes[to];

		for (int i=0; i<2; i++) {
			Mode mode = MODES[i];  // No + operator for Mode nor Transition_type
			float delay = eptr->tree->get_delay(mode, node_to.name);
			for (int j=0; j<2; j++) {
				Transition_Type type = TYPES[j];
				float new_slew = eptr->tree->get_slew(mode, node_to.name, node_from.slew[mode][type]);
				if (node_from.slew[mode][type] != UNDEFINED_SLEW[mode]) {
					float &slew = node_to.slew[mode][type];
					if ( slew == UNDEFINED_SLEW[mode] || slew_worse_than(new_slew, slew, mode) ) {
						// Always choose worst slew
						slew = new_slew;
					}
				}
				if (node_from.at[mode][type] != UNDEFINED_AT[mode]) {
					float &at = node_to.at[mode][type], new_at = node_from.at[mode][type] + delay;
					if ( at == UNDEFINED_AT[mode] || at_worse_than(new_at, at, mode) ) {
						// Always choose worst at
						at = new_at;
						if (!node_to.is_clock) {
							node_to.launching_clk[mode][type] = node_from.launching_clk[mode][type];
						}
					}
				}
			}
		}
	}
}

void Graph::at_dfs(int index, vector<bool> &visit) {
	// Top-down DP structrue
	// Assuming slew and at are both ready after calling this function

	for (const auto &adj_pair : this->rev_adj[index]) {
		Edge *eptr = adj_pair.second;
		int from = eptr->from, to = eptr->to;
		ASSERT(to == index);
		if (!visit[from]) {
			visit[from] = true;
			at_dfs(from, visit);
		}
		this->at_update(eptr);
	}
//	LOG(CERR) << get_name(index) << " " << get_mode_string(mode) << " "
//	 << nodes[index].at[mode][RISE] << " " <<  nodes[index].at[mode][FALL] << " "
//	 << nodes[index].slew[mode][RISE] << " " <<  nodes[index].slew[mode][FALL] << endl;
	// LOG(CERR) << "at rise: " << nodes[index].at[mode][RISE] << " " <<  "at fall: " << nodes[index].at[mode][LATE] << " " << endl;
	// LOG(CERR) << "slew rise: " << nodes[index].slew[mode][RISE] << " " <<  "slew fall: " << nodes[index].slew[mode][LATE] << " " << endl << endl;
}

void Graph::calculate_at() {
	/* Calculate arrival times and slews of all the vertices in this graph */
	vector<bool> visit(this->nodes.size());
	std::fill(visit.begin(), visit.end(), false);
	for (int i=0; i<(int)this->nodes.size(); i++) {
		if (!visit[i]) {
			visit[i] = true;
			this->at_dfs(i, visit);
		}
		if (nodes[i].launching_clk[0][0]==-1) {
			// cout<<nodes[i].name<<" launching from -1"<<endl;
		} else {
			// cout<<nodes[i].name<<" launching from "<<nodes[nodes[i].launching_clk[0][0]].name<<endl;
		}
	}
}

// ******************************************************
// ***              Required Arrival                  ***
// ******************************************************

void Graph::rat_arc_update(int from, int to, TimingArc *arc, Mode mode) {
	/* Use "to" update "from" through "arc" */
	Node &node_from = this->nodes[from], &node_to = this->nodes[to];

	for (int i=0; i<2; i++) {
		for (int j=0; j<2; j++) {
			Transition_Type type_from = TYPES[i], type_to = TYPES[j];
			// No ++ in Transition_Type
			if (!arc->is_transition_defined(type_from, type_to)) continue;
			float cap_load = node_to.tree->get_downstream(mode, node_to.name);
			float input_slew = node_from.slew[mode][type_from];
			/* Try to update rat */
			if (node_to.rat[mode][type_to] != UNDEFINED_RAT[mode]) {
				float delay = arc->get_delay(type_from, type_to, input_slew, cap_load);
				float new_rat = node_to.rat[mode][type_to] - delay;
				float &rat = node_from.rat[mode][type_from];
				if (rat == UNDEFINED_RAT[mode] || rat_worse_than(new_rat, rat, mode)) {
					// Always choose worst rat.
					rat = new_rat;
				}
			}
		}
	}
}

void Graph::rat_update(Edge *eptr) {
	// IMPORTANT. Use eptr->to to relax eptr->from
	int from = eptr->from, to = eptr->to;
	if (eptr->type == IN_CELL) {
		/* Update from in-cell timing arc */
		for (int i=0; i<2; i++) {
			Mode mode = MODES[i];
			for (TimingArc *arc : eptr->arcs[mode]) {
				rat_arc_update(from, to, arc, mode);
			}
		}
	}
	else {
		/* Update from RC tree */
		Node &node_from = this->nodes[from], &node_to = this->nodes[to];

		for (int i=0; i<2; i++) {
			Mode mode = MODES[i];  // No + operator for Mode nor Transition_type
			float delay = eptr->tree->get_delay(mode, node_to.name);
			for (int j=0; j<2; j++) {
				Transition_Type type = TYPES[j];
				if (node_to.rat[mode][type] != UNDEFINED_RAT[mode]) {
					float &rat = node_from.rat[mode][type], new_rat = node_to.rat[mode][type] - delay;
					if ( rat == UNDEFINED_RAT[mode] || rat_worse_than(new_rat, rat, mode) ) {
						// Always choose worst rat
						rat = new_rat;
					}
				}
			}
		}
	}
}

void Graph::rat_dfs(int index, vector<bool> &visit) {
	// This function defines top-down DP structure for rat calculation.
	// IMPORTANT. It assumes at, slew of all vertices are ready before called
	// And rat of target vertex (indicated by index) will be ready after this function return.
	for (const auto &adj_pair : this->adj[index]) {
		Edge *eptr = adj_pair.second;
		int from = eptr->from, to = eptr->to;
		ASSERT(from == index);
		if (!visit[to]) {
			visit[to] = true;
			rat_dfs(to, visit);
		}
		this->rat_update(eptr);
	}

	Node &nd = nodes[index];
	for (int i=0; i<2; i++) {
		Transition_Type type  = TYPES[i];
		if (nd.at[EARLY][type] != UNDEFINED_AT[EARLY] && nd.rat[EARLY][type] != UNDEFINED_RAT[EARLY]) {
			nd.slack[EARLY][type] = nd.at[EARLY][type] - nd.rat[EARLY][type];
		}
		if (nd.at[LATE][type] != UNDEFINED_AT[LATE] && nd.rat[LATE][type] != UNDEFINED_RAT[LATE]) {
			nd.slack[LATE][type] = nd.rat[LATE][type] - nd.at[LATE][type];
		}
	}
}

void Graph::rat_relax(float &target, float new_rat, Mode mode) {
	if (target == UNDEFINED_RAT[mode] || rat_worse_than(new_rat, target, mode)) {
		target = new_rat;
	}
}

void Graph::init_rat_from_constraint() {
	// As its name, initialize rat from constraint, yield pre-CPPR rat
	// For each constraint timing arc, it defines how a clock imposes constraint to a data pin
	//     - rat-early (hold) = at(CLK,late) + hold
	//     - rat-late (setup) = T + at(CLK,early) - setup
	// Then get CPPR credit, and add/sub to/from rat

	for (auto it = this->constraints.begin(); it != this->constraints.end(); ++it) {
		const Constraint &cons = *it;
		ASSERT(cons.arc->is_constraint());

		for (int i=0; i<2; i++) {
			for (int j=0; j<2; j++) {
				// A timing arc defines how a clock pin imposes constraint to an data pin
				Node &clk = this->nodes[cons.from], &data_pin = this->nodes[cons.to];
				Transition_Type type_clk = TYPES[i], type_data = TYPES[j];
				Mode mode = cons.mode;
				if (!cons.arc->is_transition_defined(type_clk, type_data)) continue; // This also checks what clock edge to be used
				if (mode == EARLY) {
					// Hold test
					float delay = cons.arc->get_constraint(type_clk, type_data, clk.slew[LATE][type_clk], data_pin.slew[EARLY][type_data]);
					float &data_rat = data_pin.rat[EARLY][type_data];
					float &clk_rat = clk.rat[LATE][type_clk];
					float new_data_rat = clk.at[LATE][type_clk] + delay;
					float new_clk_rat = data_pin.at[EARLY][type_data] - delay;

					// CPPR credit, slack must < 0
					int lnch_clk = data_pin.launching_clk[EARLY][type_data];
					if (lnch_clk != -1 && data_pin.at[EARLY][type_data] - new_data_rat < 0) {
						float credit = this->cppr->cppr_credit(EARLY, lnch_clk, this->nodes[lnch_clk].clk_edge, clk.index, type_clk);
						new_data_rat -= credit;
					}
					if (lnch_clk != -1 && new_clk_rat - clk.at[LATE][type_clk] < 0) {
						float credit = this->cppr->cppr_credit(EARLY, lnch_clk, this->nodes[lnch_clk].clk_edge, clk.index, type_clk);
						new_clk_rat  += credit;
					}

					if (clk.at[LATE][type_clk] != UNDEFINED_AT[LATE]) rat_relax(data_rat, new_data_rat, EARLY);
					if (data_pin.at[EARLY][type_data] != UNDEFINED_AT[EARLY]) rat_relax(clk_rat, new_clk_rat, LATE);
				} else {
					// Setup test
					float delay = cons.arc->get_constraint(type_clk, type_data, clk.slew[EARLY][type_clk], data_pin.slew[LATE][type_data]);
					float &data_rat = data_pin.rat[LATE][type_data];
					float &clk_rat = clk.rat[EARLY][type_clk];
					float new_data_rat = this->clock_T + clk.at[EARLY][type_clk] - delay;
					float new_clk_rat = data_pin.at[LATE][type_data] + delay - clock_T;

					// CPPR credit, slack must < 0
					int lnch_clk = data_pin.launching_clk[LATE][type_data];
					if (lnch_clk != -1 && new_data_rat - data_pin.at[LATE][type_data] < 0) {
						float credit = this->cppr->cppr_credit(LATE, lnch_clk, this->nodes[lnch_clk].clk_edge, clk.index, type_clk);
						new_data_rat += credit;
					}
					if (lnch_clk != -1 && clk.at[EARLY][type_clk] - new_clk_rat < 0) {
						float credit = this->cppr->cppr_credit(LATE, lnch_clk, this->nodes[lnch_clk].clk_edge, clk.index, type_clk);
						new_clk_rat  -= credit;
					}

					if (clk.at[EARLY][type_clk] != UNDEFINED_AT[EARLY]) rat_relax(data_rat, new_data_rat, LATE);
					if (data_pin.at[LATE][type_data] != UNDEFINED_AT[LATE]) rat_relax(clk_rat, new_clk_rat, EARLY);
				}
			}
		}
	}
}

void Graph::calculate_rat() {
	// Calculate arrival times and slews of all the vertices in this graph
	// You MUST call calculate_at before calling calculate_rat function.
	// That is, required arrival time requires arrival time to be calculated first.

	/* Set the value of basic case in DP */
	this->init_rat_from_constraint();

	/* DFS each point if it hasn't been visited */
	vector<bool> visit(this->nodes.size());
	std::fill(visit.begin(), visit.end(), false);
	for (int i=0; i<(int)this->nodes.size(); i++) {
		if (!visit[i]) {
			visit[i] = true;
			this->rat_dfs(i, visit);
		}
	}
}

void Graph::init_graph(){
    calculate_at();
	Logger::add_timestamp("at ok");
	if(clock_id == -1){
		LOG(ERROR) << "[Graph][init_graph] don't set clock pin\n";
	}
	cppr = new CPPR(this, clock_id);
	cppr->build_tree();
	Logger::add_timestamp("cppr ok");
	bc_map = new BC_map(this);
	bc_map->build();
	Logger::add_timestamp("bc ok");

    calculate_rat();
	Logger::add_timestamp("rat ok");
}

// ******************************************************
// ***                Unimplemented                   ***
// ******************************************************

void Graph::report_worst_paths(const string& pin, int num_path){
	//LOG(CERR) << "report_worst_paths " << pin << " " << num_path << endl;
}

void Graph::disconnect_pin(const string& pin){
	//LOG(CERR) << "disconnect_pin " << pin << endl;
}

void Graph::connect_pin(const string& pin, const string& net){
	//LOG(CERR) << "connect_pin " << pin << " " << net << endl;
}

void Graph::remove_net(const string& net){
	//LOG(CERR) << "remove_net " << net << endl;
}

void Graph::update_spef(const string& filename){
	// cout << "update " << filename << endl;
}

void Graph::remove_gate(const string& inst_name){
	//LOG(CERR) << "remove_gate " << inst_name << endl;
}

void Graph::insert_net(const string& net_name){
	//LOG(CERR) << "inst_name " << net_name << endl;
}

void Graph::insert_gate(const string& inst_name, const string& cell_type){
	//LOG(CERR) << "insert_gate " << inst_name << " " << cell_type << endl;
}

void Graph::repower_gate(const string& inst_name, const string& cell_type){
	//LOG(CERR) << "repower_gate " << inst_name << " " << cell_type << endl;
}


void Graph::report_timing(const string& from, vector<pair<Transition_Type, string>>& through,
					const string& to, int max_paths, int nworst)
{
	LOG(CERR) << "report_timing " << "-from " << from << " ";
	LOG(CERR) << "-to " << to << " ";
	LOG(CERR) << "-max_pahts " << max_paths << " ";
	LOG(CERR) << "-nworst " << nworst << " ";
	for(auto i:through){
		if(i.first==Transition_Type::FALL){
			LOG(CERR) << "-fall_through " << i.second << " ";
		}
		if(i.first==Transition_Type::RISE){
			LOG(CERR) << "-rise_through " << i.second << " ";
		}
	}
	LOG(CERR) << endl;
}

void Graph::print_graph(){
	for(size_t i=0; i<nodes.size(); i++){
		for(auto it : adj[i]){
			int to = it.first;
			Edge *e = it.second;
			if(e->type==IN_CELL){
				for(size_t j=0; j<e->arcs[LATE].size(); j++){
					TimingArc* arc = e->arcs[LATE][j];
					for(int ii=0; ii<2; ii++){
						for(int jj=0; jj<2; jj++){
							if(!arc->is_transition_defined(TYPES[ii], TYPES[jj])) continue;

							cout << get_name(i) << ":" << get_transition_string(TYPES[ii]) << " -> ";
							cout << get_name(to) << ":" << get_transition_string(TYPES[jj]) << " ";
							cout << arc->get_delay(TYPES[ii], TYPES[jj], nodes[i].slew[LATE][TYPES[ii]], nodes[to].tree->get_downstream(LATE, get_name(to))) << endl;
						}
					}
				}
			}
			else {
				cout << get_name(i) << ":Rise -> " << get_name(to) << ":R " << e->tree->get_delay(LATE, get_name(to)) << endl;
				cout << get_name(i) << ":Fall -> " << get_name(to) << ":F " << e->tree->get_delay(LATE, get_name(to)) << endl;
			}
		}
	}
}

void Graph::gen_test(string type, string filename){
	if(type=="cppr"){
		vector<int> clocks;
		for(size_t i=0; i<nodes.size(); i++) if(nodes[i].is_clock){
			clocks.push_back(i);
		}
		cout << "total clocks = " << clocks.size() << endl;

		ofstream fout;
		fout.open(filename);

		string base = "report_cppr_credit ";
		string option[][2] = { {"-fall1", ""}, {"-fall2",""}, {"", "-late"}};
		int numcase = 100000;
		srand(time(NULL));
		while(numcase--){
			string cmd2 = base;
			cmd2 += " -pin1 " + get_name( clocks[ rand()%clocks.size() ] );
			cmd2 += " -pin2 " + get_name( clocks[ rand()%clocks.size() ] );
			for(int i=0; i<2; i++){
				string cmd3;
				cmd3 = cmd2 + " " + option[0][i];
				for(int j=0; j<2; j++){
					string cmd4;
					cmd4 = cmd3 + " " + option[1][j];
					for(int k=0; k<2; k++){
						string cmd5;
						cmd5 += cmd4 + " " + option[2][k] + "\n";
						fout << cmd5;
					}
				}
			}
		}
		fout.close();
	}
	else{
		ofstream fout;
		fout.open(filename);
		string cmds[] = {"report_at ", "report_rat ", "report_slack ", "report_slew "};
		string nonesense = "-pin ";
		string modes[] = {"-early ", "-late "};
		string types[] = {"-rise ", "-fall "};

		cout << " total nodes = " << nodes.size() << endl;
		for (auto &nd : nodes) {
			for (auto &cmd : cmds) {
				for (auto &type : types) {
					for (auto mode : modes) {
						fout<<cmd<<nonesense<<nd.name<<" "<<mode<<type<<endl;
					}
				}
			}
		}
		fout.close();
	}
}

// ------------ For Testing ----------------

#ifdef TEST_GRAPH

int main() {
	Verilog vlog;
	CellLib early_lib(EARLY), late_lib(LATE);
	Spef spef;
	Graph G;

	spef.open("unit_test/graph/simple.spef");
	vlog.open("unit_test/graph/simple.v");
	early_lib.open("unit_test/graph/simple_Early.lib");
	late_lib.open("unit_test/graph/simple_Late.lib");
	G.build(vlog, spef, early_lib, late_lib);

	Logger::create()->~Logger();
}

#endif
