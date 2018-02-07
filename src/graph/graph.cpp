#include "graph.h"

// ------------------ Node related ----------------------

Graph::Node::Node(int index, const string &name) {
	this->exist = true;
	this->name = name;
	this->index = index;

	// These undefined values is defined in header.h
	this->rat[EARLY][RISE] = this->rat[EARLY][FALL] = UNDEFINED_RAT[EARLY];
	this->rat[LATE][RISE]  = this->rat[LATE][FALL]  = UNDEFINED_RAT[LATE];
	this->at[EARLY][RISE]  = this->at[EARLY][FALL]  = UNDEFINED_AT[EARLY];
	this->at[LATE][RISE]   = this->at[LATE][FALL]   = UNDEFINED_AT[LATE];
}

int Graph::add_node(const string &name) {
	int id = this->next_id++;
	this->trans.insert(make_pair(name, id));
	this->nodes.emplace_back(id, name);
	this->adj.emplace_back();
	return id;
}

int Graph::get_index(const string &name) {
	auto it = this->trans.find(name);
	if (it == this->trans.end()) {
		return this->add_node(name);
	} else {
		return it->second;
	}
}

bool Graph::in_graph(int index) const {
	return this->nodes[index].exist;
}

bool Graph::in_graph(const string &name) const {
	auto it = this->trans.find(name);
	return it != this->trans.end();
}

const string& Graph::get_name(int index) const {
	static const string EMPTY = string("");
	if (index > (int)this->nodes.size() || this->nodes[index].name.empty()) {
		LOG(WARNING) << "[Graph] Illegal index " << index << " when get name. (Node does not exist)" << endl;
		return EMPTY;
	}
	return this->nodes[index].name;
}

void Graph::set_at(const string &pin_name, float early_at[2], float late_at[2]) {
	LOG(CERR) << "set_at " << pin_name << " " << early_at[RISE] << " " << early_at[LATE]
	<< " " << late_at[RISE] << " " << late_at[LATE] << endl;

	string handle = cell_pin_concat( INPUT_PREFIX, pin_name );
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
	LOG(CERR) << "set_rat " << pin_name << " " << early_rat[RISE] << " " << early_rat[LATE]
	<< " " << late_rat[RISE] << " " << late_rat[LATE] << endl;

	string handle = cell_pin_concat( OUTPUT_PREFIX, pin_name );
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
	LOG(CERR) << "set_at " << pin_name << " " << get_mode_string(mode) << " "
	<< get_transition_string(transition) << " " << val << endl;

	string handle = cell_pin_concat( INPUT_PREFIX, pin_name );
	if (!in_graph(handle)) {
		LOG(ERROR) << "Try to modify arrival time of a node which is not primary input. (" << pin_name << ")" << endl;
		ASSERT_NOT_REACHED();
	}
	int index = this->get_index(handle);
	this->nodes[index].at[mode][transition] = val;
}

void Graph::set_rat(const string &pin_name, Mode mode, Transition_Type transition, float val){
	LOG(CERR) << "set_rat " << pin_name << " " << get_mode_string(mode) << " "
	<< get_transition_string(transition) << " " << val << endl;

	string handle = cell_pin_concat( OUTPUT_PREFIX, pin_name );
	if (!in_graph(handle)) {
		LOG(ERROR) << "Try to modify rat of a node which is not primary ouput. (" << pin_name << ")" << endl;
		ASSERT_NOT_REACHED();
	}
	int index = this->get_index(handle);
	this->nodes[index].rat[mode][transition] = val;
}

void Graph::set_slew(const string &pin_name, Mode mode, Transition_Type transition, float val){
	LOG(CERR) << "set_slew " << pin_name << " " << get_mode_string(mode) << " "
	<< get_transition_string(transition) << " " << val << endl;

	string handle = cell_pin_concat( INPUT_PREFIX, pin_name );
	if (!in_graph(handle)) {
		LOG(ERROR) << "Try to modify slew of a node which is not primary input. (" << pin_name << ")" << endl;
		ASSERT_NOT_REACHED();
	}
	int index = this->get_index(handle);
	this->nodes[index].slew[mode][transition] = val;
}

void Graph::set_slew(const string &pin_name, float early_slew[2], float late_slew[2]){
	LOG(CERR) << "set_slew " << pin_name << " " << early_slew[RISE] << " " << early_slew[FALL]
	<< " " << late_slew[RISE] << " " << late_slew[FALL] << endl;

	string handle = cell_pin_concat( INPUT_PREFIX, pin_name );
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

float Graph::get_at(const string &pin_name, Mode mode, Transition_Type transition){
	LOG(CERR) << "report_at " << pin_name << " " << get_mode_string(mode) << " " <<
	get_transition_string(transition) << endl;

	if (!in_graph(pin_name)){
		LOG(WARNING) << "[Graph][get_at] pin no exist , pin: " << pin_name << endl;
		return 0;
	}
	int index = this->get_index(pin_name);
	return this->nodes[index].at[mode][transition];
}

float Graph::get_rat(const string &pin_name, Mode mode, Transition_Type transition){
	LOG(CERR) << "report_rat " << pin_name << " " << get_mode_string(mode) << " " <<
	get_transition_string(transition) << endl;

	if (!in_graph(pin_name)){
		LOG(WARNING) << "[Graph][get_rat] pin no exist , pin: " << pin_name << endl;
		return 0;
	}
	int index = this->get_index(pin_name);
	return this->nodes[index].rat[mode][transition];
}

float Graph::get_slew(const string &pin_name, Mode mode, Transition_Type transition){
	LOG(CERR) << "report_slew " << pin_name << " " << get_mode_string(mode) << " " <<
	get_transition_string(transition) << endl;

	if (!in_graph(pin_name)){
		LOG(WARNING) << "[Graph][get_slew] pin no exist , pin: " << pin_name << endl;
		return 0;
	}
	int index = this->get_index(pin_name);
	return this->nodes[index].slew[mode][transition];
}

float Graph::get_slack(const string &pin_name, Mode mode, Transition_Type transition){
	LOG(CERR) << "report_slack " << pin_name << " " << get_mode_string(mode) << " " <<
	get_transition_string(transition) << endl;

	if (!in_graph(pin_name)){
		LOG(WARNING) << "[Graph][get_slack] pin no exist , pin: " << pin_name << endl;
		return 0;
	}
	int index = this->get_index(pin_name);
	return this->nodes[index].slack[mode][transition];
}
// ------------------ Edge related ----------------------

Graph::Edge::Edge(int src, int dest, Edge_type type) {
	this->from = src;
	this->to = dest;
	this->type = type;
}

Graph::Edge* Graph::add_edge(int src, int dest, Edge_type type) {
	Edge *eptr = new Edge(src, dest, type);
	this->adj[src].insert( {dest, eptr} );
	this->adj[dest].insert( {src, eptr} );
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

const vector< unordered_map< int, Graph::Edge* > >& Graph::adj_list() const {
	return this->adj;
}

// ------------------ Wire related ----------------------

Graph::Wire_mapping::Wire_mapping() {
	this->src=-1;
}

Graph::Wire_mapping* Graph::get_wire_mapping(const string &wire_name) const {
	auto it = this->wire_mapping.find(wire_name);
	if (it == wire_mapping.end()) return NULL;
	return it->second;
}

// ------------------ Graph related ----------------------

void Graph::build(Verilog &vlog, Spef &spef, CellLib &early_lib, CellLib &late_lib) {
	/* Initilize */
	this->next_id = 0;
	CellLib &lib = early_lib; // Two lib has the same topological structure.
	CellLib *lib_arr[2] = {&early_lib, &late_lib};

	/* Construct wire mapping */
	for (const string &wire_name : vlog.wire) {
		Wire_mapping *mapping = new Wire_mapping;
		mapping->src=-1;
		this->wire_mapping.insert( make_pair(wire_name, mapping) );
	}

	/* Append each pin to the wire and construct internal timing arc  */
	for (const auto &p : vlog.gates) {
		const Verilog::Verilog_Gate *gt = p.second;
		const string &cell_type = gt->cell_type, cell_name = gt->cell_name;
		for (const pair<string,string> &io_pair : gt->param) {
			const string &pin_name = io_pair.first, &wire_name = io_pair.second;
			// Following lines should be changed to enum value.
			Direction_type direction = lib.get_pin_direction(cell_type, pin_name);
			if (direction == OUTPUT) {
				// Construct in-cell timing arc. (early mode)
				int sink = this->get_index( cell_pin_concat(cell_name, pin_name) );
				vector<TimingArc*> *arcs = early_lib.get_pin_total_TimingArc(cell_type, pin_name);
				for (TimingArc *arc : *arcs) {
					int src = this->get_index( cell_pin_concat(cell_name, arc->get_related_pin()) );
					this->add_arc(src,sink, arc, EARLY);
				}

				// Construct in-cell timing arc. (late mode)
				arcs = late_lib.get_pin_total_TimingArc(cell_type, pin_name);
				for (TimingArc *arc : *arcs) {
					int src = this->get_index( cell_pin_concat(cell_name, arc->get_related_pin()) );
					this->add_arc(src,sink, arc, LATE);
				}

				// Complete wire mapping. Here "sink" is the output pin.
				Wire_mapping *mapping = this->get_wire_mapping(wire_name);
				ASSERT(mapping != NULL);
				ASSERT(mapping->src == -1);
				mapping->src = sink;
			} else {
				// Complete wire mapping.
				int sink = this->get_index( cell_pin_concat(cell_name, pin_name) );
				Wire_mapping *mapping = this->get_wire_mapping(wire_name);
				ASSERT(mapping != NULL);
				mapping->sinks.emplace_back(sink);
			}
		}
	}

	/* Create nodes for primary input */
	for (const string &in_pin : vlog.input) {
		int src = this->get_index( cell_pin_concat( INPUT_PREFIX, in_pin ) );
		Wire_mapping *mapping = this->get_wire_mapping(in_pin);
		ASSERT(mapping != NULL);
		ASSERT(mapping->src == -1);
		mapping->src = src;
	}

	/* Create nodes for primary output */
	for (const string &out_pin : vlog.output) {
		int sink = this->get_index( cell_pin_concat( OUTPUT_PREFIX, out_pin ) );
		Wire_mapping *mapping = this->get_wire_mapping(out_pin);
		ASSERT(mapping != NULL);
		mapping->sinks.emplace_back(sink);
	}

	/* Construct external timing arc through wire mapping */
	for (const auto &wire_pair : this->wire_mapping) {
		const string &wire_name = wire_pair.first;
		Wire_mapping *mapping = wire_pair.second;
		int from = mapping->src;
		ASSERT(from != -1);
		for (int to : mapping->sinks) {
			Edge *eptr = add_edge(from, to, RC_TREE);
			SpefNet *net = spef.get_spefnet_ptr(wire_name, 0);
			if(net!=NULL) eptr->tree = new RCTree(net, &vlog, lib_arr);
			else{
				SpefNet *net = new SpefNet();
				net->set_total_cap(0);
				string root = get_name ( wire_mapping[wire_name]->src ), type="I", dir="O";
				if(is_prefix(root, INPUT_PREFIX)) root = root.substr( INPUT_PREFIX.size()+1 ), type="P", dir="I";
				if(is_prefix(root, OUTPUT_PREFIX)) root = root.substr( OUTPUT_PREFIX.size()+1 ), type="P", dir="O";

				net->set_name(wire_name);
				net->add_conn(root, type, dir);
				for(auto &it: (*wire_mapping[wire_name]).sinks) {
					string name = get_name(it), type="I", dir="I";
					if(is_prefix(name, INPUT_PREFIX))
						name = name.substr( INPUT_PREFIX.size()+1 ), type="P", dir="I";
					if(is_prefix(name, OUTPUT_PREFIX))
						name = name.substr( OUTPUT_PREFIX.size()+1 ), type="P", dir="O";

					net->add_conn(name, type , dir);
					net->add_cap(name, 0);
					net->add_res(root, get_name(it), 0);
				}
				spef.add_net( wire_name, net);
				eptr->tree = new RCTree(net, &vlog, lib_arr);
			}
			eptr->tree->build_tree();
		}
	}
}

/* unimplement */

void Graph::set_clock(const string& pin_name,float period, float low){
	LOG(CERR) << "set_clock " << pin_name << " " << period << " " << low << endl;
	clock_T = period;
}

void Graph::set_load(const string& pin_name, float cap){
	LOG(CERR) << "set_load " << pin_name << " " << cap << endl;
}

void Graph::report_worst_paths(const string& pin, int num_path){
	LOG(CERR) << "report_worst_paths " << pin << " " << num_path << endl;
}

void Graph::disconnect_pin(const string& pin){
	LOG(CERR) << "disconnect_pin " << pin << endl;
}

void Graph::connect_pin(const string& pin, const string& net){
	LOG(CERR) << "connect_pin " << pin << " " << net << endl;
}

void Graph::remove_net(const string& net){
	LOG(CERR) << "remove_net " << net << endl;
}

void Graph::update_spef(const string& filename){
	cout << "update " << filename << endl;
}

void Graph::remove_gate(const string& inst_name){
	LOG(CERR) << "remove_gate " << inst_name << endl;
}

void Graph::insert_net(const string& net_name){
	LOG(CERR) << "inst_name " << net_name << endl;
}

void Graph::insert_gate(const string& inst_name, const string& cell_type){
	LOG(CERR) << "insert_gate " << inst_name << " " << cell_type << endl;
}

void Graph::repower_gate(const string& inst_name, const string& cell_type){
	LOG(CERR) << "repower_gate " << inst_name << " " << cell_type << endl;
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
