#ifndef FILE_READER_H
#define FILE_READER_H

#include "header.h"

class File_Reader
{
public:
    string next_token();

    void open(string filename);

private:
    bool is_normal(char c);
    bool is_specail(char c);
    bool is_useful(char c);

    char* buffer;
    int position;     // now position
    int length;       // file size
};
#endif /* end FILE_READER_H */