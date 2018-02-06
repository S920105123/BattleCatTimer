#include "graph.h"

// ------------------ Node related ----------------------

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

const string& Graph::get_name(int index) const {
	static const string EMPTY = string("");
	if (index > (int)this->nodes.size() || this->nodes[index].name.empty()) {
		LOG(WARNING) << "[Graph] Illegal index " << index << " when get name. (Node does not exist)" << endl;
		return EMPTY;
	}
	return this->nodes[index].name;
}

Graph::Node::Node(int index, const string &name) {
	this->name = name;
	this->index = index;
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
//	LOG(CERR) << "An edge built from " << this->get_name(src)<< " to " << this->get_name(dest);
//	if (type == IN_CELL) {
//		LOG(CERR) << " (In cell edge).\n";
//	} else {
//		LOG(CERR) << " (RC tree edge).\n";
//	}
	return eptr;
}

Graph::Edge* Graph::get_edge(int src, int dest) {
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

Graph::Wire_mapping* Graph::get_wire_mapping(const string &wire_name) {
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
		int src = this->get_index( cell_pin_concat( input_prefix, in_pin ) );
		Wire_mapping *mapping = this->get_wire_mapping(in_pin);
		ASSERT(mapping != NULL);
		ASSERT(mapping->src == -1);
		mapping->src = src;
	}

	/* Create nodes for primary output */
	for (const string &out_pin : vlog.output) {
		int sink = this->get_index( cell_pin_concat( output_prefix, out_pin ) );
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
				if(is_prefix(root, input_prefix)) root = root.substr( input_prefix.size()+1 ), type="P", dir="I";
				if(is_prefix(root, output_prefix)) root = root.substr( output_prefix.size()+1 ), type="P", dir="O";

				net->set_name(wire_name);
				net->add_conn(root, type, dir);
				for(auto &it: (*wire_mapping[wire_name]).sinks){
					string name = get_name(it), type="I", dir="I";
					if(is_prefix(name, input_prefix))
						name = name.substr( input_prefix.size()+1 ), type="P", dir="I";
					if(is_prefix(name, output_prefix))
						name = name.substr( output_prefix.size()+1 ), type="P", dir="O";

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
