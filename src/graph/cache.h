#ifndef CACHE_H
#define CACHE_H

#include "header.h"
#include "debug.h"

//This class is the cache of the report timing
class BC_map;
class Kth;
class Cache {

public:
	//Cache(const vector<int>& through,const vector<int>& disable, BC_map *bc);
	Cache(BC_map *bc, Kth *k);
	~Cache();

	void set_through(const vector<int>& thr);
	void set_disable(const vector<int>& dis);
	bool set_vert_valid(int id, bool val);
	bool set_edge_valid(int id, bool val);
	void set_timestamp(int val);

	bool get_vert_valid(int id);
	bool get_edge_valid(int id);
	const vector<int>& get_through();
	const vector<int>& get_disable();
	int get_timestamp();

	void clear();

	int cal_difference(const vector<int>& through, const vector<int>& disable);

private:
	int timestamp;
	BC_map *bc_map;
	Kth* kth;
	unordered_map<int, bool> edge_valid;
	unordered_map<int, bool> vert_valid;

	vector<int> through;
	vector<int> disable;

friend BC_map;
};


#endif
