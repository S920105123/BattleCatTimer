#include "cache.h"


//Cache::Cache(const vector<int>& through,const vector<int>& disable, BC_map *bc):
Cache::Cache(BC_map *bc, Kth *k)
	//through(through), disable(disable){
{
	bc_map = bc;
	timestamp = 0;
	kth = k;
}

Cache::~Cache() {
	//delete kth;
}

bool Cache::set_vert_valid(int id, bool val) {
	if(val==false) {
		auto it = vert_valid.find(id);
		if(it!=vert_valid.end()) vert_valid.erase(it);
		return false;
	}
	else return vert_valid[id] = val;
}

bool Cache::set_edge_valid(int id, bool val) {
	if(val==false) {
		auto it = edge_valid.find(id);
		if(it!=edge_valid.end()) edge_valid.erase(it);
		return false;
	}
	return edge_valid[id] = val;
}

void Cache::set_timestamp(int val) {
	timestamp = val;
}

void Cache::set_through(const vector<int>& thr) {
	through.clear();
	for(auto& x:thr) through.push_back(x);
}

void Cache::set_disable(const vector<int>& dis) {
	disable.clear();
	for(auto& x:dis) disable.push_back(x);
}

bool Cache::get_vert_valid(int id) {
	auto is_found = vert_valid.find(id);
	if(is_found == vert_valid.end()) return false;
	return is_found->second;
}

bool Cache::get_edge_valid(int id) {
	auto is_found = edge_valid.find(id);
	if(is_found == edge_valid.end()) return false;
	return is_found->second;
}

int Cache::get_timestamp() {
	return timestamp;
}

const vector<int>& Cache::get_through() {
	return through;
}

const vector<int>& Cache::get_disable() {
	return disable;
}

void Cache::clear() {
	edge_valid.clear();
	vert_valid.clear();
	through.clear();
	disable.clear();
}

int Cache::cal_difference(const vector<int>& through, const vector<int>& disable) {
	return 0;
}
