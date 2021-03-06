// Copyright (c) Mathias Kaerlev 2012-2015.
//
// This file is part of Anaconda.
//
// Anaconda is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Anaconda is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Anaconda.  If not, see <http://www.gnu.org/licenses/>.

#include "objects/stringparser.h"
#include "stringcommon.h"
#include "common.h"
#include "fileio.h"

StringParser::StringParser(int x, int y, int id)
: FrameObject(x, y, id), has_split(false)
{

}

void StringParser::add_delimiter(const chowstring & v)
{
    if (v.size() != 1) {
        std::cout << "Delimiter size " << v.size() << " not supported"
            << std::endl;
        return;
    }
    delimiters += v;
}

void StringParser::reset_delimiters()
{
    delimiters.clear();
    has_split = false;
}

void StringParser::load(const chowstring & filename)
{
    if (filename[0] == '[')
        // work around HFA bug
        return;
    read_file(filename.c_str(), value);
}

void StringParser::split()
{
    if (has_split)
        return;
    elements.clear();
    split_string(value, delimiters, elements);
    has_split = true;
}

void StringParser::set(const chowstring & v)
{
    value = v;
    has_split = false;
}

int StringParser::get_count()
{
    split();
    return int(elements.size());
}

chowstring StringParser::set_element(const chowstring & value, int index)
{
    if (delimiters.size() <= 0)
        return value;
    index--;
    split();
    chowstring ret;
    vector<chowstring>::const_iterator it;

    for (int i = 0; i < int(elements.size()); i++) {
        if (i == index)
            ret += value;
        else
            ret += elements[i];
        // XXX not entirely correct behaviour, but good enough
        if (i < int(elements.size()) - 1)
            ret += delimiters[0];
    }
    return ret;
}

const chowstring & StringParser::get_element(int i)
{
    i--;
    split();
    if (i < 0 || i >= int(elements.size()))
        return empty_string;
    return elements[i];
}

const chowstring & StringParser::get_last_element()
{
    split();
    if (elements.empty())
        return empty_string;
    return elements[elements.size()-1];
}

chowstring StringParser::replace(const chowstring & from,
                                  const chowstring & to)
{
    chowstring ret = value;
    replace_substring(ret, from, to);
    return ret;
}

chowstring StringParser::remove(const chowstring & sub)
{
    chowstring ret = value;
    replace_substring(ret, sub, empty_string);
    return ret;
}

chowstring StringParser::get_md5()
{
    return ::get_md5(value);
}
