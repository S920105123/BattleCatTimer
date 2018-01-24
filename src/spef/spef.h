#ifndef SPEF_H
#define SPEF_H

#include "header.h"
#include "file_reader.h"
#include "logger.h"

class SpefNet;

class Spef {
public:
    void open(string filename);

private:

    void addNet();
    map<string, SpefNet*> nets;
};

class SpefNet{
public:
    SpefNet(){}
    SpefNet(string _name, vector<string> &conn, vector<pair<string, float>> &cap)

private:
    string name;
    float total_cap;
    map<string, int> pin_id;            // map pin name to pinid
    vector<int>      conn_id;           // keep conn id
    vector<string>   pin_name;          // save pinid's name
    vector<float>    pin_cap;           // pinid's cap
    map<int, map<int, float>> pin_res;  // tow
};

#endif /* end SPEF_H */
