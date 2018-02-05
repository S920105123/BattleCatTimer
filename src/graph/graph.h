#ifndef GRAPH_H
#define GRAPH_H

#include "debug.h"
#include "logger.h"
#include "header.h"
#include "func.h"
#include "verilog.h"
#include "cell_lib.h"
#include "timing_arc.h"
#include "rc_tree.h"

typedef enum { RC_TREE, IN_CELL } Edge_type;
const string input_prefix  = "primary_in";
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
		// If "type"==IN_CELL, refer "arcs" for delay, "tree" otherwise.
		Edge_type type;
		int from ,to;
		RCTree *tree;
		vector<TimingArc*> arcs[2];
		
		Edge(int src, int dest, Edge_type type);
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
	const vector< unordered_map<int, Edge*> >& adj_list() const;// Return adjacency list of node i.
	Edge* add_edge(int src, int dest, Edge_type type);          // Add an empty edge, with neither arc nor rc tree.
	Edge* get_edge(int src, int dest);                          // Return NULL if not exist
	void add_arc(int src, int dest, TimingArc *arc, Mode mode); // Append one arc to edge from src to dest, add edge if doesn't exist.
	
	// Wire related
	Wire_mapping* get_wire_mapping(const string &wire_name);
	
	// Graph related
	void build(Verilog &vlog, Spef &spef, CellLib &early_lib, CellLib &late_lib); // Build this graph from a verilog file.
	
private:
	int next_id;
	unordered_map<string,int> trans;         // Transform name to index.
	vector<Node> nodes;                      // nodes[i]: Node with index i.
	vector< unordered_map<int, Edge*> > adj; // Adjacency list of all nodes.
	
	// Used when building graph, not sure if this will be used in the future.
	// Key  = wire name
	// src  = output pin index (source pin)
	// sink = vector of input pin index (sink pins)
	unordered_map<string, Wire_mapping*> wire_mapping;
};

#endif
