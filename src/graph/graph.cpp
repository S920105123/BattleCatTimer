#include "graph.h"

// ------------------ Node related ----------------------

int Graph::add_node(const string &name) {
	int id = this->next_id++;
	this->trans.insert(make_pair(name, id));
	this->nodes.push_back(Node(id, name));
	this->adj.push_back(vector<Edge>());
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

Graph::Edge::Edge(int src, int dest, Edge_type type, void *delay_ref) {
	this->from = src;
	this->to = dest;
	this->type = type;
	if (type == CELL) {
		this->arc = (TimingArc*)delay_ref;
//		this->tree = NULL;
	} else {
		this->arc = NULL;
//		this->tree = delay_ref;
	}
}

void Graph::add_edge(int src, int dest, Edge_type type, void *delay_ref) {
	this->adj[src].push_back(Edge(src, dest, type, delay_ref));
	LOG(CERR) << "An edge built from " << this->get_name(src)<< " to " << this->get_name(dest);
	if (type == CELL) {
		LOG(CERR) << " (In cell edge).\n"; 
	} else {
		LOG(CERR) << " (RC tree edge).\n"; 
	}
}

const vector< vector< Graph::Edge > >& Graph::adj_list() const {
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

void Graph::build(Verilog &vlog, CellLib &lib, Graph_type type) {
	/* Initilize */
	this->next_id=0;
	
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
			string direction = lib.get_pin_direction(cell_type, pin_name);
			if (direction == "output") {
				// Construct in-cell timing arc.
				int sink = this->get_index( cell_pin_concat(cell_name, pin_name) );
				vector<TimingArc*> *arcs = lib.get_pin_total_TimingArc(cell_type, pin_name);
				for (TimingArc *arc : *arcs) {
					int src = this->get_index( cell_pin_concat(cell_name, arc->get_related_pin()) );
					this->add_edge(src,sink,CELL,arc);
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
				mapping->sinks.push_back(sink);
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
		mapping->sinks.push_back(sink);
	}
	
	/* Construct external timing arc through wire mapping */
	for (const auto &wire_pair : this->wire_mapping) {
		Wire_mapping *mapping = wire_pair.second;
		int from = mapping->src;
		ASSERT(from != -1);
		for (int to : mapping->sinks) {
			// RC TREE NEED TO BE FILLED IN
			add_edge(from, to, RC_TREE, NULL);
		}
	}
}


// ------------ For Testing ----------------

#ifdef TEST_GRAPH

int main() {
	Verilog vlog;
	CellLib lib;
	Graph G;
	
	vlog.open("unit_test/graph/simple.v");
	lib.open("unit_test/graph/simple_Early.lib");
	G.build(vlog,lib,EARLY);
	
}

#endif
