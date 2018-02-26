#include "file_reader.h"
#include "header.h"

#include <bits/stdc++.h>
using namespace std;

string read_pin(File_Reader& in){
    string a = in.next_token();
    string b = in.next_token();
    if(b=="/"){
        string c = in.next_token();
        return a + b + c;
    }
    else{
        in.put_back(b);
        return a;
    }
}

struct Path
{
    vector<string> pin;
    vector<float>  delay;
    vector<float>  total;
    vector<Transition_Type> type;
    string endpoint, beginpoint;
    float rat, at, slack;
    int lineno;

    void read(File_Reader &in){
        string token;
        lineno = in.get_lineno();
        while(true){
            token = in.next_token();

            if(token == "Endpoint"){
                EXPECT(in.next_token(), ":");
                endpoint = read_pin(in);
            }
            else if(token == "Beginpoint"){
                EXPECT(in.next_token(), ":");
                beginpoint = read_pin(in);
            }
            else if(token == "="){
                EXPECT(in.next_token(), "Required");
                EXPECT(in.next_token(), "Time");
                rat = stof(in.next_token());
                // cout << rat << endl;

                EXPECT(in.next_token(), "-");
                EXPECT(in.next_token(), "Arrival");
                EXPECT(in.next_token(), "Time");
                at = stof(in.next_token());
                // cout << at << endl;

                EXPECT(in.next_token(), "=");
                EXPECT(in.next_token(), "Slack");
                EXPECT(in.next_token(), "Time");
                slack = stof(in.next_token());
                // cout << slack << endl;

                token = in.next_token();
                if(token=="+"){
                    EXPECT(in.next_token(), "Input");
                    EXPECT(in.next_token(), "Delay");
                    in.next_token();
                }else in.put_back(token);
            }
            else if(token[0]=='-' and token[1]=='-'){
                while(true){
                    token = in.next_token();
                    if(token[0]=='-' and token[1]=='-') break;
                }
                EXPECT(in.next_token(), "-");
                delay.push_back(0);
                total.push_back( stof(in.next_token()) );
                // cout << total.back() << endl;
                if(in.next_token()=="^") type.push_back(RISE);
                else type.push_back(FALL);
                pin.push_back( read_pin(in) );

                token = in.next_token();
                if(token=="-"){
                    EXPECT( in.next_token(), ">");
                }
                else in.put_back(token);

                while(true){
                    token = in.next_token();
                    if(token[0]=='-' and token[1]=='-') break;
                    in.put_back( token );
                    delay.push_back( stof(in.next_token()) );
                    total.push_back( stof(in.next_token()) );

                    if(in.next_token()=="^") type.push_back(RISE);
                    else type.push_back(FALL);
                    pin.push_back( read_pin(in) );

                    token = in.next_token();
                    if(token=="-"){
                        EXPECT( in.next_token(), ">");
                    }
                    else in.put_back(token);
                }
                break;
            }
        }
    }

    void print(){
        cout << "line: " << lineno << endl;
        cout << "Beginpoint: " << beginpoint << endl;
        cout << "Endpoint: " << endpoint << endl;
        cout << "Rat: " << rat << endl;
        cout << "at: " << at << endl;
        cout << "Slack: " << slack << endl;

        for(int i=0; i<(int)pin.size(); i++){
            cout << delay[i] << " " << total[i] << " " << get_transition_string(type[i]) << pin[i] << endl;
        }
        cout << endl;
    }
};

void read_file(string filename, vector<Path*>& res){
    File_Reader in;
    in.open( filename );
    string token;
    while( (token=in.next_token()) != "" ){
        in.put_back(token);
        Path* path = new Path();
        path->read( in );
        res.push_back( path );
    }
}

bool compare(Path* ans, Path* out,int show=0){
    if(ans->beginpoint != out->beginpoint){
        if(show) cout << "wa: begin, ans: " << ans->beginpoint << ", out: " << out->beginpoint << endl;
        return false;
    }
    if(ans->endpoint   != out->endpoint){
        if(show) cout << "wa: end, ans: " << ans->endpoint << ", out: " << out->endpoint << endl;
        return false;
    }

    float lim = 0.15;
    if(ans->slack > out->slack and  std::abs(ans->slack - out->slack)>lim){ // special judge
        // if(show) cout << "wa: slack: " << abs(ans->slack-out->slack) << endl;
        // out should contain all pins in ans
        int j = 0;
        for(int i=0; i<(int)ans->pin.size(); i++){
            while(true){
                if(out->pin[j] == ans->pin[i] and out->type[j]==ans->type[i]){
                    break;
                }
                j++;
                if(j==out->pin.size()){
                    if(show) cout << "wa: special judge , out don't go through " << ans->pin[i] << endl;
                    return false;
                }
            }
            j++;
        }
        return true;
    }
    else if( std::abs(ans->slack-out->slack)<=lim){
        if(std::abs(ans->rat-out->rat)>lim){
            if(show) cout << "wa: rat\n";
            return false;
        }
        if(std::abs(ans->at-out->at)>lim)  {
            if(show) cout << "wa: at\n";
            return false;
        }
        if(ans->pin.size() != out->pin.size()){
            if(show) cout << "wa: pin size , ans: " << ans->pin.size() << ", out: " << out->pin.size() << endl;
            return false;
        }
        for(int i=0; i<(int)ans->pin.size(); i++){
            if(ans->pin[i] != out->pin[i]){
                if(show) cout << "wa pin name, ans: " << ans->pin[i] << ", out: " << out->pin[i] << endl;
                return false;
            }
            if(abs(ans->delay[i] - out->delay[i]) > lim){
                if(show) cout << "wa delay, ans: " << ans->delay[i] << ", out: " << out->delay[i] << endl;
                return false;
            }

            if(abs(ans->total[i] - out->total[i]) > lim){
                if(show) cout << "wa total, ans: " << ans->total[i] << ", out: " << out->total[i] << endl;
                return false;
            }

            if(ans->type[i] != out->type[i]){
                return false;
            }

        }
        return true;
    }
    else{
        cout << "wa slack , ans: " <<  ans->slack << ", out: " << out->slack << endl;
        return false;
    }
}

int main(int argc, char** argv)
{
    if(argc<2){
        cout << " no argument\n";
    }

    string testcase = argv[1];
    string base = "testcase_v1.2/";
    string myans_path    = base + testcase + "/" + testcase + ".myans";
    string myoutput_path = base + testcase + "/" + testcase + ".myoutput";

    vector<Path*> ans, out;
    read_file(myans_path, ans);
    read_file(myoutput_path, out);

    // if(ans.size() != out.size()){
    //     cout << ans.size() << " " << out.size() << endl;
    //     cout << "Error: ans.size() != out.size() " << endl;
    //     return 0;
    // }

    vector<int> ans_match, out_match;
    ans_match.resize( ans.size() );
    out_match.resize( out.size() );
    fill(ans_match.begin(), ans_match.end(), -1);
    fill(out_match.begin(), out_match.end(), -1);

    cout << "ans paths: " << ans.size() << " , out paths: " << out.size() << endl;
    for(int i=0; i<ans.size(); i++) if(ans_match[i]==-1){
        for(int j=0; j<out.size(); j++) if(out_match[j]==-1){
            if(compare(ans[i], out[j])){
                ans_match[i] = j;
                out_match[j] = i;
                break;
            }
        }
    }

    cout << "ans not match: \n";
    for(int i=0; i<ans.size(); i++){
        if(ans_match[i]==-1){
            cout << i+1 << ", line: " << ans[i]->lineno <<" ";
            if(ans.size() >= out.size()) compare(ans[i], out[i], 1);
            cout << endl;
        }
    }
    cout << endl;

    cout << "out not match: \n";
    for(int i=0; i<out.size(); i++){
        if(out_match[i]==-1){
            cout << i+1 << " ";
            if(ans.size() <= out.size()) compare(ans[i], out[i], 1);
        }
    }
    cout << endl;
    // ans.back()->print();
    // out.back()->print();
    // cout << compare(ans.back(), out.back());
    return 0;
}
