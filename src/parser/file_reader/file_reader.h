#ifndef FILE_READER_H
#define FILE_READER_H

#include "header.h"

#define BUFFER_SIZE 100

class File_Reader
{
public:
    string next_token();

    void open(string filename);

private:
    bool is_special(char c);

    char* buffer;
    int position;     // now position
    int length;       // file size
};
#endif /* end FILE_READER_H */
