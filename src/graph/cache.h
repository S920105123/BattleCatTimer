#ifndef CACHE_H
#define CACHE_H

#include "header.h"
#include "debug.h"

//This class is the cache of the report timing
class Cache {

public:
	Cache(const vector<int>& through,const vector<int>& disable);
	bool set_vert(int id, bool val);
	bool set_edge(int id, bool val);
	bool get_vert(int id);
	bool get_edge(int id);

private:
	unordered_map<int, bool> edge_valid;
	unordered_map<int, bool> vert_valid;

	vector<int> through;
	vector<int> disable;
};


#endif
