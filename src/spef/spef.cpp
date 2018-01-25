#include "spef.h"

void Spef::open(string filename){
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
            tot_cap  = string_to_float( in.next_token() );
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
                    spef_net->add_cap(name, string_to_float(val));
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
                    spef_net->add_res(name, name2, string_to_float(val));
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

void Spef::add_net(string name, SpefNet* net){        // map name to net pointer
    if(nets.find( name )==nets.end()){
        nets[name] = net;
    }
    else LOG(ERROR) << "[Spef][add_net] " << name << " appear twice!\n";
}

void Spef::print_net(string name){
    if(nets.find(name) == nets.end()) cout << "no such net " << name << endl;
    else nets[name]->print_net();
}

int Spef::size(){
    return nets.size();
}

/*    SpefNet   */
void SpefNet::set_total_cap(float f){
    total_cap = f;
}

void SpefNet::set_name(string _name){
    net_name = _name;
}

void SpefNet::add_conn(string name,string type, string dir){
    conn_name.push_back(name);
    conn_type.push_back(type);
    conn_dir.push_back(dir);
}

void SpefNet::add_cap(string name, float f){
    if(pin_id.find( name ) == pin_id.end()){
        pin_cap.push_back(f);
        pin_name.push_back(name);
        pin_id[name] = pin_cap.size()-1;
    }else LOG(ERROR) << "[SpefNet][add_cap] " << net_name << " cap " << name << " appear twice!\n";
}

void SpefNet::add_res(string pin1, string pin2, float f){
    int p1 = get_pin_id(pin1);
    int p2 = get_pin_id(pin2);
    pin_res[p1][p2] = f;

}

void SpefNet::print_net(){
    cout << "net_name:" << net_name << endl;
    cout << "CONN:\n" ;
    for(auto x:conn_name) cout << x << " its pinid = " << pin_id[x] << endl;

    cout << "CAP:\n";
    for(int i=0; i<pin_cap.size(); i++)
        cout << pin_name[i] << " cap = " << pin_cap[i] << " pinid = " << i << endl;

    cout << "RES:\n";
    for(auto& x:pin_res){
        for(auto& y:x.second)
            cout << x.first << "(" << pin_name[x.first] << ")" << " " <<
                    y.first << "(" << pin_name[y.first] << ")" << " = " << y.second << endl;
    }
}

int SpefNet::get_pin_id(string name){
    if(pin_id.find( name )==pin_id.end()){
        LOG(WARNING) << "[SpefNet] Net:" << net_name << " : " << name << " don't appear at CAP, so it's RES is 0\n";
        add_cap( name , 0);
    }
    return pin_id[ name ];
}

/* --- Test --- */

#ifdef TEST_SPEF

int main()
{
    string filename;
	Spef spef;
    cout << "Enter spef path : ";
    cin >> filename;
    spef.open(filename);

	cout << "open ok\n";
    cout << "total : " << spef.size() << endl;
	string name;
    do{
        cout << "Enter Net name or exit : ";
        cin >> name;
        if(name=="exit") break;
        spef.print_net(name);
    }while(true);

	Logger* logger = Logger::create();
    logger->~Logger();
    return 0;
}
#endif
