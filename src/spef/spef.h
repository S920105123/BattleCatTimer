#ifndef SPEF_H
#define SPEF_H

#include "header.h"
#include "file_reader.h"
#include "logger.h"

class SpefNet;

class Spef {
public:
    void open(string filename);

    void add_net(string name, SpefNet* net);        // map name to net pointer

    void print_net(string name);

    int size();
private:

    map<string, SpefNet*> nets;
};

class SpefNet{
public:
    SpefNet(){}

    void set_total_cap(float f);

    void set_name(string _name);

    void add_conn(string name,string type, string dir);

    void add_cap(string name, float f);

    void add_res(string pin1, string pin2, float f);

    int get_pin_id(string name);

    void print_net();

private:

    string net_name;
    float total_cap;
    map<string, int> pin_id;                             // map pin name to pinid
    vector<string>   conn_name, conn_type, conn_dir;     // save conn
    vector<string>   pin_name;                           // save pinid's name
    vector<float>    pin_cap;                            // save pinid's cap
    map<int, map<int, float>> pin_res;                   // save res between two pinid
};

#endif /* end SPEF_H */
