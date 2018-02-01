#ifndef GRAPH_H
#define GRAPH_H

#include "debug.h"
#include "logger.h"
#include "header.h"
#include "func.h"
#include "verilog.h"

class Graph {
	
	struct Node {
		Edge *from;            // Where is this node's value from
		vector<Edge*> *to;     // Connect to which Node / RC tree
	};
	
	struct Edge {
		Edge_type type;
		float delay;
	};
	
};

#endif
