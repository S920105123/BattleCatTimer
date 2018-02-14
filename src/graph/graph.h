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
#include "cppr.h"
#include "bc_map.h"
#include "kth.h"

typedef enum { RC_TREE, IN_CELL } Edge_type;
typedef enum { PRIMARY_IN, PRIMARY_OUT, INTERNAL} Node_type;
const string INPUT_PREFIX  = "";
const string OUTPUT_PREFIX = "";

class CPPR;
class BC_map;
class Kth;
class Graph {

public:

	struct Node {
		bool exist, is_clock, in_cppr;
		Transition_Type clk_edge; // Only set when is_clock==true
		int index;
		string name;
		Node_type node_type;
		RCTree *tree;

		float at[2][2];  // Arrival time. e.g, at[EARLY][RISE]
		float rat[2][2]; // Required arrival time.
		float slew[2][2];
		float slack[2][2];
		int launching_clk[2][2];
		int constrained_clk;

		Node(int index, const string& name, Node_type type);
	};

	struct Edge {
		// If "type"==IN_CELL, refer "arcs" for delay, "tree" otherwise.
		Edge_type type;
		int from ,to;
		RCTree *tree;
		vector<TimingArc*> arcs[2];

		Edge(int src, int dest, Edge_type type);
	};

	struct Wire_mapping {
		int src;
		vector<int> sinks;
		Wire_mapping();
	};

	struct Constraint {
		Mode mode;
		int from, to;
		TimingArc *arc;
		Constraint(int from, int to, TimingArc *arc, Mode mode);
	};

	Graph();
	~Graph();
	// Node related
	int get_index(const string &name);                 // "name" should follow this format: <Cell name>:<Pin name>, insert one if not found.
	const string& get_name(int index) const;           // Get name from index.
	bool in_graph(int index) const;                    // Check whether a node with "index" in graph
	bool in_graph(const string &name) const;           // Check whether a node with "name" in graph (name: <cell_name>:<pin_name>)
	void set_at(const string &pin_name, float early_at[], float late_at[]); // Set arrival time of given pin, only primary input is allowed to be set by this function
	void set_rat(const string &pin_name, float early_rat[2], float late_rat[2]);    // Only primary output can be set by this function
	void set_at(const string &pin_name, Mode mode, Transition_Type transition, float val);
	void set_rat(const string &pin_name, Mode mode, Transition_Type transition, float val);
	void set_slew(const string &pin_name, Mode mode, Transition_Type transition, float val);
	void set_slew(const string &pin_name, float early_rat[2], float late_rat[2]);

	float get_at(const string &pin_name, Mode mode, Transition_Type transition);
	float get_rat(const string &pin_name, Mode mode, Transition_Type transition);
	float get_slew(const string &pin_name, Mode mode, Transition_Type transition);
	float get_slack(const string &pin_name, Mode mode, Transition_Type transition);
	float get_cppr_credit(const string& pin1, const string& pin2, Transition_Type type1, Transition_Type typ2, Mode mode);

	/* unimplemented */
	void set_load(const string& pin_name, float cap);
	void set_clock(const string& pin_name,float period, float low);
	void report_worst_paths(const string& pin, int num_path);
	void disconnect_pin(const string& pin);
	void connect_pin(const string& pin, const string& net);
	void remove_net(const string& net);
	void update_spef(const string& filename);
	void remove_gate(const string& inst_name);
	void insert_net(const string& net_name);
	void insert_gate(const string& inst_name, const string& cell_type);
	void repower_gate(const string& inst_name, const string& cell_type);
	void report_timing(const vector<pair<Transition_Type,string>>&from,
					   const vector<pair<Transition_Type,string>>&through,
					   const vector<pair<Transition_Type,string>>&to, int max_paths, int nworst);

	int add_node(const string &name, Node_type type);

	// Edge related
	const vector< unordered_map<int, Edge*> >& adj_list() const;      // Return adjacency list
	const vector< unordered_map<int, Edge*> >& rev_adj_list() const;  // Return reverse adjacency list
	Edge* add_edge(int src, int dest, Edge_type type);                // Add an empty edge, with neither arc nor rc tree.
	Edge* get_edge(int src, int dest) const;                          // Return NULL if not exist
	void add_arc(int src, int dest, TimingArc *arc, Mode mode);       // Append one arc to edge from src to dest, add edge if doesn't exist.
	void add_constraint(int src, int dest, TimingArc *arc, Mode mode);// Add a constraint, src must be a clock

	// Wire related
	Wire_mapping* get_wire_mapping(const string &wire_name);

	// Graph related
	void build(Verilog &vlog, Spef &spef, CellLib &early_lib, CellLib &late_lib); // Build this graph from a verilog file.
	void calculate_at();   // Calculate arrival time of all vertices in the graph
	void calculate_rat();  // Calculate required arrival time of all vertices.
	                       // You MUST call calculate_at before calling calculate_rat function.
	                       // That is, required arrival time requires arrival time to be calculated first.

	void init_graph();    // initial graph information,  call it after build a graph
	void print_graph();
	void gen_test(string type, string filename);

private:

	int next_id;
	int clock_id;
	int clock_T;                                 // clock period from .timing
	unordered_map<string, float> out_load;		 // out_load from .timing
	unordered_map<string,int> trans;             // Transform name to index.
	vector<Node> nodes;                          // nodes[i]: Node with index i.
	vector< unordered_map<int, Edge*> > adj;     // Adjacency list of all nodes.
	vector< unordered_map<int, Edge*> > rev_adj; // Reverse adjacency list.
	vector< Constraint > constraints;            // Conatraint edges
	vector< int > clocks;                        // Clock nodes
	vector<pair<float,int>> nodes_slack;         // slack of ff:d of primary_out <slack, map_id>



	// Graph related
	void at_arc_update(int from, int to, TimingArc *arc, Mode mode);
	void at_update(Edge *eptr);
	void at_dfs(int index, vector<bool> &visit);
	void rat_arc_update(int from, int to, TimingArc *arc, Mode mode);
	void rat_update(Edge *eptr);
	void rat_dfs(int index, vector<bool> &visit);
	void init_rat_from_constraint();
	void rat_relax(float &target, float new_rat, Mode mode);

	// "wire_mapping":
	// key  = wire name
	// src  = output pin index (source pin)
	// sink = vector of input pin index (sink pins)
	unordered_map<string, Wire_mapping*> wire_mapping;
	CPPR *cppr;
	BC_map* bc_map;

	friend class CPPR;
	friend class BC_map;
	friend class Kth;
};

#endif
