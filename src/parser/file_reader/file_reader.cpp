#include "file_reader.h"

void File_Reader::open(string filename){
    ifstream in(filename, std::ios::in);

    /* may using log*/
    if(!in.good()) cout << "[*] Fail to open " << filename << "\n";

    in.seekg(0, in.end);
    length = in.tellg();
    in.seekg(0, in.beg);

    buffer = new char[length+10];

    in.read(buffer, length);
    cout << buffer << endl;
}

bool File_Reader::is_special(char c){

}

string File_Reader::next_token(){

}

/* --- Test --- */

#ifdef TEST_FILE_READER

int main()
{
    File_Reader reader;
    reader.open("testcase_v1.2\\s1196\\s1196.v");
    return 0;
}

#endif
