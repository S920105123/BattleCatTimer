#include "cppr.h"

CPPR::CPPR(Graph *_graph, int clock_root){
    graph = _graph;
    root = clock_root;
    num = graph->nodes.size();

    in = new int[num+10];
    out = new int[num+10];
}

CPPR::~CPPR(){
    if(in) delete in;
    if(out) delete out;
}

void CPPR::build(){

}
