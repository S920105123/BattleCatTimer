#ifndef FILE_READER_H
#define FILE_READER_H

#include "header.h"
#include "debug.h"
#include "logger.h"
#include "file_reader.h"

class File_Reader
{
public:
    File_Reader(){ buffer = NULL; }
    ~File_Reader(){
        if(buffer) delete buffer;
    }
    string next_token();

    void open(const string &filename);
    void put_back(const string& s);

private:
    bool is_normal(char c);
    bool is_specail(char c);
    bool is_useful(char c);

    char* buffer;
    stack<string> buf;
    int position;     // now position
    int length;       // file size
};
#endif /* end FILE_READER_H */
