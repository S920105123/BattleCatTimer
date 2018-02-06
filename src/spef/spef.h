#ifndef SPEF_H
#define SPEF_H

#include "header.h"
#include "file_reader.h"
#include "logger.h"

class SpefNet;

class Spef {
public:
    void open(const string& filename);

    void add_net(const string& name, SpefNet* net);        // map name to net pointer

    void print_net(const string& name);

    SpefNet* get_spefnet_ptr(const string& name, bool log=true);

    const map<string, SpefNet*>& get_total_nets(){
        return nets;
    }

    int size();
private:

    map<string, SpefNet*> nets;
};

class SpefNet{
public:
    SpefNet(){}

    void set_total_cap(float f);

    void set_name(const string& _name);

    void add_conn(const string& name,const string& type, const string& dir);
    void add_cap(const string& name, float f);
    void add_res(const string& pin1, const string& pin2, float f);

    int get_pin_id(const string& name);

    const vector<tuple<int,int,float>>& get_pin_res(){ // for iterator
        return pin_res;
    }

    const vector<tuple<string,string,string>>& get_total_conn(){
        return conn;
    }

    const vector<string>& get_total_pins_name(){
        return pin_name;
    }

    string get_pin_name(int pin_id){
        if((size_t)pin_id >= pin_name.size())
            LOG(ERROR) << "[SpefNet][get_pin_name] id over size\n";
        else return pin_name[ pin_id ];
        return "";
    }

    float get_pin_cap(const string & pin_name){
        return pin_cap[ get_pin_id(pin_name) ];
    }

    float get_pin_cap(int id){
        if((size_t)id >= pin_cap.size())
            LOG(ERROR) << "[SpefNet][get_pin_cap] id over size\n";
        else return pin_cap[ id ];
        return 0;
    }

    float get_total_cap(){
        return total_cap;
    }

    string get_name(){
        return net_name;
    }

    int get_num_pins(){
        return pin_id.size();
    }

    const unordered_map<string,int>& get_total_pins(){
        return pin_id;
    }

    void print_net();

private:

    string net_name;
    float total_cap;
    unordered_map<string, int> pin_id;                  // map pin name to pinid
    vector<tuple<string,string,string> > conn;          // name, type, dir
    vector<string>   pin_name;                          // save pinid's name
    vector<float>    pin_cap;                           // save pinid's cap
    // map<int, map<int, float>> pin_res;               // save res between two pinid
    vector<tuple<int,int,float>> pin_res;
};

#endif /* end SPEF_H */
