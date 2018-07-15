#ifndef BCT_BITSET_H
#define  BCT_BITSET_H

#include "header.h"

struct BitSet {
	int N;
	int num_box;
	unsigned int *val;
	BitSet(int n);
	BitSet();
	~BitSet();

	bool set_val(int x,int v);
	bool get_val(int x);
	void clear();
	void create(int n);
};

#endif
