#ifndef GRAPH_H
#define GRAPH_H

#include "debug.h"
#include "logger.h"
#include "header.h"
#include "func.h"
#include "verilog.h"
#include "cell_lib.h"
#include "timing_arc.h"

typedef enum { RC_TREE, CELL } Edge_type;
typedef enum { EARLY, LATE } Graph_type;
const string input_prefix = "primary_in";
const string output_prefix = "primary_out";

class Graph {

public:
	
	struct Node {
		// Temporarily nothing to store!?
		// Maybe store dp information
		int index;
		string name;
		
		Node(int index, const string& name);
	};
	
	struct Edge {
		// If "type"==CELL, use "arc", "tree" otherwise.
		Edge_type type;
		int from ,to;
		// RC_tree*
		TimingArc *arc;
		
		Edge(int src, int dest, Edge_type type, void *delay_ref);
		float get_delay() const;
	};
	
	struct Wire_mapping {
		int src;
		vector<int> sinks;
		Wire_mapping();
	};
	
	// Node related
	int get_index(const string &name);                        // "name" should follow this format: <Cell name>:<Pin name>, insert one if not found.
	const string& get_name(int index) const;                  // Get name from index.
	int add_node(const string &name);
	
	// Edge related
	const vector<vector<Edge>>& adj_list() const;             // Return adjacency list of node i.
	void add_edge(int src, int dest, Edge_type type, void *delay_ref);
	
	// Wire related
	Wire_mapping* get_wire_mapping(const string &wire_name);
	
	// Graph related
	void build(Verilog &vlog, CellLib &lib, Graph_type type); // Build this graph from a verilog file.
	
private:
	int next_id;
	unordered_map<string,int> trans;  // Transform name to index.
	vector<Node> nodes;               // nodes[i]: Node with index i.
	vector< vector<Edge> > adj;       // Adjacency list of all nodes.
	
	// Used when building graph, not sure if this will be used in the future.
	// Key  = wire name
	// src  = output pin index (source pin)
	// sink = vector of input pin index (sink pins)
	unordered_map<string, Wire_mapping*> wire_mapping;
};

#endif
