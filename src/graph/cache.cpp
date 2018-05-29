#include "cache.h"


Cache::Cache(const vector<int>& through,const vector<int>& disable):
	through(through), disable(disable){

}

bool Cache::set_vert(int id, bool val) {
	vert_valid[id] = val;
	return val;
}

bool Cache::set_edge(int id, bool val) {
	edge_valid[id] = val;
	return val;
}

bool Cache::get_vert(int id) {
	auto is_found = vert_valid.find(id);
	return is_found != vert_valid.end();
}

bool Cache::get_edge(int id) {
	auto is_found = edge_valid.find(id);
	return is_found != edge_valid.end();
}
