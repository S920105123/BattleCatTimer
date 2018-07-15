#include "bct_bitset.h"

BitSet::BitSet() {
	val = nullptr;
}

BitSet::BitSet(int n) {
	create(n);
}

BitSet::~BitSet() { 
	delete[] val; 
}

void BitSet::create(int n) {
	N = n;
	num_box = ceil(n/32.0);
	val = new unsigned int[num_box];
}

bool BitSet::set_val(int x,int v) {
	int which = x/32;
	int pos = x%32;

	if(v != 0) val[which] |= (1<<pos);
	else val[which] &= ~(1<<pos);

	return (bool)v;
}

bool BitSet::get_val(int x) {
	int which = x/32;
	int pos = x%32;
	int tmp = val[which];

	if(pos!=0) tmp >>= pos;
	if(pos!=31) tmp <<= (31);
	return tmp;
}

void BitSet::clear() {
	memset(val, 0, sizeof(unsigned int)*num_box);
}

