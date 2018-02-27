#include "file_reader.h"

void File_Reader::open(const string &filename){
    ifstream in(filename, std::ios::binary);

    if(!in.good()) LOG(ERROR) << "[File_Reader] Fail to open " << filename << "\n";

    in.seekg(0, in.end);
    length = in.tellg();
    in.seekg(0, in.beg);

    buffer = new char[length+10];

    LOG(NORMAL) << "[File_Reader] open " << filename << " length = " << length << '\n';
    position = 0;
    lineno = 1;
    in.read(buffer, length);
    in.close();
}

bool File_Reader::is_normal(char c){
    return  (c>='a'&&c<='z') or
            (c>='A'&&c<='Z') or
            (c>='0'&&c<='9') or
            c=='_' or c=='-' or c=='.';
}

bool File_Reader::is_specail(char c){
    return c=='(' or c==')' or
           c=='[' or c==']' or
           c=='{' or c=='}' or
           c=='*' or c=='.' or c==':' or c=='\\' or c=='/'
           or c=='=' or c=='^' or c=='>' or c=='+';
}

bool File_Reader::is_useful(char c){
    if(c=='\n') lineno++;
    return c!=',' and c!=';' and c!=' ' and c!='\n' and isprint(c);
}

void File_Reader::put_back(const string& s){
    buf.push(s);
}

string File_Reader::next_token(){
    if(!buf.empty()){
        string val = buf.top();
        buf.pop();
        return val;
    }
    string res="";
    char *s = buffer;
    while(position<length){
        /* del comment */
        if(position+1<length && s[position]=='/' && s[position+1]=='/'){
            position+=2;
            while(position<length && s[position]!='\n') position++;
            lineno++;
        }
        else if(position+1<length && s[position]=='/' && s[position+1]=='*'){ // /* */
            position += 2;
            do{
                if(s[position]=='\n') lineno++;
                if(position+1<length && s[position]=='*' and s[position+1]=='/'){
                    position+=2;
                    break;
                }else position++;
            }while(position<length);
        }

        /* "res" */
        else if(s[position]=='\"'){
            position++;
            while(position<length && s[position]!='\"'){
                if(s[position]!='\n') res += s[position++];
                else LOG(WARNING) << "[*][File_Reader] there is a newline between \" \n", position++, lineno++;
            }
            position++;
            return res;
        }

        /* 'res' */
        else if(s[position]=='\''){
            position++;
            while(position<length && s[position]!='\'')  res += s[position++];
            position++;
            return res;
        }

        /* specail char */
        else if(is_specail(s[position])){
            res += s[position++];
            return res;
        }

        /* del ; , ' ', \n */
        else if(!is_useful(s[position])){
            position++;
        }

        /* res */
        else if(is_normal(s[position])){
            do{
                res += s[position++];
            }while(position<length && is_normal(s[position]));
            return res;
        }
        else if(s[position] == 0) position++;
        else if(position<length){
            res += s[position++];
            return res;
        }
        else{
            for(int i=position; i<length; i++){
                cout << s[i];
            }
            cout << std::endl;
            ASSERT_NOT_REACHED();
        }
    }
    return res;
}

/* --- Test --- */

#ifdef TEST_FILE_READER

int main()
{
    File_Reader reader;
    // reader.open("testcase_v1.2\\s1196\\s1196_Early.lib");
    // reader.open("testcase_v1.2\\s1196\\s1196.v");
    reader.open("src\\file_reader\\testcase.test");
    // reader.open("testcase_v1.2\\s1196\\s1196.spef");
    freopen("test_out.test","w",stdout);
    while(true){
        string token = reader.next_token();
        if(token.empty()) break;
        cout << token << '\n';
    }
    return 0;
}

#endif
