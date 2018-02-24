#include "spef.h"

void Spef::open(const string& filename){
    File_Reader in;
    in.open(filename);

    string token;
    string net_name, type, dir, val, name, name2;
    float tot_cap;
    SpefNet* spef_net = NULL;
    do{
        token = in.next_token();

        if(token=="D_NET"){
            if(spef_net != NULL) LOG(ERROR) << "[Spef][open] " << net_name << " don't have 'END' \n";
            spef_net = new SpefNet();

            net_name = in.next_token();
            tot_cap  = stof( in.next_token() );
            spef_net->set_name( net_name );
            spef_net->set_total_cap( tot_cap );
            if(in.next_token()!="*" or (token=in.next_token())!="CONN")
                LOG(ERROR) << "[Spef][open] " << net_name << " don't have `CONN` \n";

            /* CONN */
            if(token == "CONN"){
                do{
                    type = in.next_token(); // *
                    type = in.next_token(); // type
                    if(type!="P" and type!="I"){
                        if((token=type)!="CAP")
                        LOG(ERROR) << "[Spef][open] " << net_name << " don't have `CAP` \n";
                        break;
                    }
                    name = in.next_token();
                    dir = in.next_token();
                    if(dir == ":"){
                        name += dir;
                        name += in.next_token();
                        dir = in.next_token();
                    }
                    spef_net->add_conn(name, type, dir);
                }while(true);

            }

            /* CAP */
            if(token == "CAP"){
                do{
                    type = in.next_token(); // id
                    if(type=="*"){
                        if((token=in.next_token())!="RES")
                            LOG(ERROR) << "[Spef][open] " << net_name << " don't have 'RES'\n";
                        break;
                    }
                    name = in.next_token();
                    val = in.next_token();
                    if(val == ":"){
                        name += val;
                        name += in.next_token();
                        val = in.next_token();
                    }
                    spef_net->add_cap(name, stof(val) );
                }while(true);
            }

            /* RES */
            if(token=="RES"){
                do{
                    type = in.next_token(); // pin_id
                    if(type=="*"){
                        if((token=in.next_token())!="END")
                            LOG(ERROR) << "[Spef][open] " << net_name << " don't have 'END'\n";
                        break;
                    }
                    name = in.next_token();
                    name2 = in.next_token();
                    if(name2==":"){
                        name += name2;
                        name += in.next_token();
                        name2 = in.next_token();
                    }
                    val = in.next_token();
                    if(val==":"){
                        name2 += val;
                        name2 += in.next_token();
                        val = in.next_token();
                    }
                    spef_net->add_res(name, name2, stof( val) );
                }while(true);
            }

            /* END */
            if(token=="END"){
                if(spef_net == NULL)
                    LOG(ERROR) << "[Spef][open] there exists a NULL spef_net at END block\n";
                else add_net( net_name, spef_net );
                spef_net = NULL;
            }
        }
    }while(!token.empty());
    LOG(NORMAL) << "[Spef][open] done.\n";
}

void Spef::add_net(const string& name, SpefNet* net){        // map name to net pointer
    if(nets.find( name )==nets.end()){
        nets[name] = net;
    }
    else LOG(ERROR) << "[Spef][add_net] " << name << " appear twice!\n";
}

void Spef::print_net(const string& name){
    if(nets.find(name) == nets.end()) cout << "no such net " << name << '\n';
    else nets[name]->print_net();
}

int Spef::size(){
    return nets.size();
}

SpefNet* Spef::get_spefnet_ptr(const string &name, bool logged){
    if(nets.find(name)==nets.end()){
        if(logged) LOG(ERROR) << "[Spef][get_spef_net_ptr] no such net: " << name << '\n';
    }else return nets[name];
    return NULL;
}

/*    SpefNet   */
void SpefNet::set_total_cap(float f){
    total_cap = f;
}

void SpefNet::set_name(const string& _name){
    net_name = _name;
}

void SpefNet::add_conn(const string& name,const string& type, const string& dir){
    conn.emplace_back(name, type, dir);
}

void SpefNet::add_cap(const string& name, float f){
    if(pin_id.find( name ) == pin_id.end()){
        pin_cap.emplace_back(f);
        pin_name.emplace_back(name);
        pin_id[name] = pin_cap.size()-1;
    }else LOG(ERROR) << "[SpefNet][add_cap] " << net_name << " cap " << name << " appear twice!\n";
}

void SpefNet::add_res(const string& pin1, const string& pin2, float f){
    int p1 = get_pin_id(pin1);
    int p2 = get_pin_id(pin2);

    pin_res.emplace_back(p1, p2, f);
    // pin_res[p1][p2] = f;
}

int SpefNet::get_pin_id(const string& name){
    if(pin_id.find( name )==pin_id.end()){
        // LOG(WARNING) << "[SpefNet] Net:" << net_name << " : " << name << " don't appear at CAP, so it's CAP is 0\n";
        add_cap( name , 0);
    }
    return pin_id[ name ];
}

void SpefNet::print_net(){
    cout << "net_name:" << net_name << '\n';
    cout << "CONN:\n" ;
    for(auto& x:conn){
        string name, type, dir;
        tie(name, type, dir) = x;
        cout << name << " " << type << " " << dir << " pin id = " << pin_id[name] << '\n';
    }

    cout << "CAP:\n";
    for(size_t i=0; i<pin_cap.size(); i++)
        cout << pin_name[i] << " cap = " << pin_cap[i] << " pinid = " << i << '\n';

    cout << "RES:\n";
    // for(auto& x:pin_res){
    //     for(auto& y:x.second)
    //         cout << x.first << "(" << pin_name[x.first] << ")" << " " <<
    //                 y.first << "(" << pin_name[y.first] << ")" << " = " << y.second << '\n';
    // }
    for(auto& t:pin_res){
        int from, to;
        float res;
        tie(from ,to, res) = t;
        cout << from << "(" << pin_name[from] << ")" << " " <<
                to << "(" << pin_name[to] << ")" << " = " << res << '\n';
    }
}

/* --- Test --- */

#ifdef TEST_SPEF

int main()
{
    string filename;
	Spef spef;
    cout << "Enter spef path : ";
    cin >> filename;
    time_t start = clock();
    spef.open(filename);
    time_t end = clock();

	cout << "open ok spend : " << end-start << "\n";
    cout << "total : " << spef.size() << '\n';
	string name;
    do{
        cout << "Enter Net name or exit : ";
        cin >> name;
        if(name=="exit") break;

        SpefNet *spefnet = spef.get_spefnet_ptr( name );
        if(spefnet==NULL){
            cout << "no such net: " << name << '\n';
            continue;
        }
        spefnet->print_net();
        const auto & pin_name = spefnet->get_total_pins_name();
        for(const auto& t:spefnet->get_pin_res()){
            int from, to;
            float res;
            tie(from ,to, res) = t;
            cout << from << "(" << pin_name[from] << ")" << " " <<
                    to << "(" << pin_name[to] << ")" << " = " << res << '\n';
            // cout << from << " " << to << " = " << res << '\n';
        }

        // spef.print_net(name);
    }while(true);

	Logger* logger = Logger::create();
    logger->~Logger();
    return 0;
}
#endif
