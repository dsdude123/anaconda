#ifndef CHOWDREN_STRINGPARSER_H
#define CHOWDREN_STRINGPARSER_H

#include <string>
#include "types.h"
#include "frameobject.h"

class StringParser : public FrameObject
{
public:
    vector<std::string> elements;
    std::string delimiters;
    std::string value;
    bool has_split;

    StringParser(int x, int y, int id);
    void split();
    void load(const std::string & filename);
    void set(const std::string & value);
    void add_delimiter(const std::string & delim);
    const std::string & get_element(int index);
    const std::string & get_last_element();
    std::string replace(const std::string & from, const std::string & to);
    std::string remove(const std::string & sub);
};

#endif // CHOWDREN_STRINGPARSER_H
