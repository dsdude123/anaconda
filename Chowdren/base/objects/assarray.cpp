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

#include "objects/assarray.h"
#include "objects/blowfish.cpp"
#include "fileio.h"
#include <iostream>
#include "stringcommon.h"
#include "datastream.h"

#define ARRAY_MAGIC "ASSBF1.0"

static vector<AssociateArray*> arrays;

AssociateArray::AssociateArray(int x, int y, int type_id)
: FrameObject(x, y, type_id), store()
{
}

void AssociateArray::init_global()
{
    arrays.push_back(this);
}

AssociateArray::~AssociateArray()
{
    if (map != &global_map) {
        delete map;
        return;
    }
    arrays.erase(std::remove(arrays.begin(), arrays.end(), this),
                 arrays.end());
}

void AssociateArray::set_key(const chowstring & key)
{
    cipher.set_key(key);
}

void AssociateArray::load_encrypted(const chowstring & filename,
                                    int method)
{
    chowstring src;
    if (!read_file(filename.c_str(), src))
        return;

    clear();
    chowstring dst;
    cipher.decrypt(&dst, src);

    if (dst.compare(0, sizeof(ARRAY_MAGIC)-1,
                    ARRAY_MAGIC, sizeof(ARRAY_MAGIC)-1) != 0) {
        std::cout << "Invalid magic for " << filename << std::endl;
        return;
    }

    load_data(dst.substr(sizeof(ARRAY_MAGIC)-1), method);
}

void AssociateArray::load(const chowstring & filename, int method)
{
    std::cout << "Load: " << filename << std::endl;
    chowstring data;

    if (!read_file(filename.c_str(), data))
        return;

    clear();

#ifdef CHOWDREN_IS_NL2
    // we are just getting some raw data here. this is probably the right
    // format.
    load_data(data, method);
#else
    if (data.compare(0, sizeof(ARRAY_MAGIC)-1,
                     ARRAY_MAGIC, sizeof(ARRAY_MAGIC)-1) != 0) {
        std::cout << "Invalid magic for " << filename << std::endl;
        return;
    }

    load_data(data.substr(sizeof(ARRAY_MAGIC)-1), method);
#endif

    std::cout << "Done" << std::endl;
}

inline void decode_method(chowstring & str, int method)
{
    for (chowstring::iterator i = str.begin(); i != str.end(); ++i)
        *i = char(*i - method);
}

inline void encode_method(chowstring & str, int method)
{
    for (chowstring::iterator i = str.begin(); i != str.end(); ++i)
        *i = char(*i + method);
}

void AssociateArray::load_data(const chowstring & data, int method)
{
    unsigned int pos = 0;

    while (pos < data.size()) {
        if (data[pos] == '\x00')
            // probably EOF, NULL-byte due to encryption padding
            break;
        int start = pos;

        while (data[pos] != ' ')
            pos++;
        int len = string_to_int(data.substr(start, pos-start));
        pos++;

        // read key
        start = pos;
        while (data[pos] != '\x00')
            pos++;
        chowstring key = data.substr(start, pos-start);
        decode_method(key, method);
        pos++;
        len -= key.size();

        // read string
        start = pos;
        while (data[pos] != '\x00')
            pos++;
        chowstring string = data.substr(start, pos-start);
        decode_method(string, method);
        pos++;
        len -= string.size();

        // read value
        chowstring value = data.substr(pos, len);
        decode_method(value, method);
        pos += len;

        AssociateArrayItem & item = (*map)[key];
        item.value = string_to_int(value);
        item.string = string;
    }
}

void AssociateArray::set_value(const chowstring & key, int value)
{
    (*map)[key].value = value;
}

void AssociateArray::add_value(const chowstring & key, int value)
{
    (*map)[key].value += value;
}

void AssociateArray::sub_value(const chowstring & key, int value)
{
    (*map)[key].value -= value;
}

void AssociateArray::set_string(const chowstring & key,
                                const chowstring & value)
{
    (*map)[key].string = value;
}

int AssociateArray::get_value(const chowstring & key)
{
    ArrayMap::const_iterator it = map->find(key);
    if (it == map->end())
        return 0;
    return it->second.value;
}

const chowstring & AssociateArray::get_string(const chowstring & key)
{
    ArrayMap::const_iterator it = map->find(key);
    if (it == map->end())
        return empty_string;
    return it->second.string;
}

void AssociateArray::clear()
{
    for (int i = 0; i < CHOWDREN_ASSARRAY_STORE; ++i)
        store[i] = NULL;
    map->clear();
    if (map != &global_map)
        return;
    vector<AssociateArray*>::iterator it;
    for (it = arrays.begin(); it != arrays.end(); ++it) {
        AssociateArray * array = *it;
        for (int i = 0; i < CHOWDREN_ASSARRAY_STORE; ++i)
            array->store[i] = NULL;
    }
}

bool AssociateArray::has_key(const chowstring & key)
{
    return map->find(key) != map->end();
}

bool AssociateArray::count_prefix(const chowstring & key, int count)
{
    return count_prefix(key) >= count;
}

int AssociateArray::count_prefix(const chowstring & key)
{
    int n = 0;
    ArrayMap::const_iterator it;
    for (it = map->begin(); it != map->end(); it++) {
        const chowstring & other = it->first;
        if (other.compare(0, key.size(), key) == 0)
            n++;
    }
    return n;
}

void AssociateArray::remove_key(const chowstring & key)
{
    ArrayMap::iterator it = map->find(key);
    if (it == map->end())
        return;
    AssociateArrayItem * search = &it->second;
    for (int i = 0; i < CHOWDREN_ASSARRAY_STORE; i++) {
        if (store[i] != search)
            continue;
        store[i] = NULL;
        break;
    }
    map->erase(it);
}

ArrayAddress AssociateArray::get_first()
{
    ArrayMap::const_iterator it;
    it = map->begin();
    if (it == map->end())
        return ArrayAddress();
    return ArrayAddress(it);
}

ArrayAddress AssociateArray::get_prefix(const chowstring & prefix, int index,
                                        ArrayAddress start)
{
    ArrayMap::const_iterator it;
    if (start.null)
        it = map->begin();
    else
        it = start.it;

    for (; it != map->end(); it++) {
        const chowstring & other = it->first;
        if (other.compare(0, prefix.size(), prefix) != 0)
            continue;
        if (index == 0)
            return ArrayAddress(it);
        index--;
    }
    return ArrayAddress();
}

const chowstring & AssociateArray::get_key(ArrayAddress addr)
{
    if (addr.null)
        return empty_string;
    return addr.it->first;
}

template <typename T, bool magic>
inline void save_assarray(AssociateArray & array, T & stream, int method)
{
    if (magic)
        stream.write(ARRAY_MAGIC, sizeof(ARRAY_MAGIC)-1);

    ArrayMap::iterator it;
    for (it = array.map->begin(); it != array.map->end(); it++) {
        AssociateArrayItem & item = it->second;
        chowstring key = it->first;
        encode_method(key, method);
        chowstring string = item.string;
        encode_method(string, method);
        chowstring value = number_to_string(item.value);
        encode_method(value, method);

        int len = key.size() + string.size() + value.size();
        stream.write_string(number_to_string(len));
        stream.write_int8(' ');
        stream.write_string(key);
        stream.write_int8('\x00');
        stream.write_string(string);
        stream.write_int8('\x00');
        stream.write_string(value);
    }
}

void AssociateArray::save(const chowstring & path, int method)
{
    FSFile fp(path.c_str(), "w");
    if (!fp.is_open()) {
        std::cout << "Could not save associate array: " << path << std::endl;
        return;
    }
    WriteStream stream;
    save_assarray<WriteStream, false>(*this, stream, method);
    stream.save(fp);
    fp.close();
}

void AssociateArray::save_encrypted(const chowstring & path, int method)
{
    std::stringstream ss;
    DataStream stream(ss);
    save_assarray<DataStream, true>(*this, stream, method);
    chowstring src = ss.str();
    chowstring dst;
    cipher.encrypt(&dst, src);

    FSFile fp(path.c_str(), "w");
    if (!fp.is_open()) {
        std::cout << "Could not save file " << path << std::endl;
        return;
    }
    fp.write(dst.data(), dst.size());
    fp.close();
}

// set/get with store

void AssociateArray::set_value(int index, const chowstring & key, int value)
{
    AssociateArrayItem * item = store[index];
    if (item == NULL) {
        item = &((*map)[key]);
        store[index] = item;
    }
    item->value = value;
}

void AssociateArray::add_value(int index, const chowstring & key, int value)
{
    AssociateArrayItem * item = store[index];
    if (item == NULL) {
        item = &((*map)[key]);
        store[index] = item;
    }
    item->value += value;
}

void AssociateArray::sub_value(int index, const chowstring & key, int value)
{
    AssociateArrayItem * item = store[index];
    if (item == NULL) {
        item = &((*map)[key]);
        store[index] = item;
    }
    item->value -= value;
}

void AssociateArray::set_string(int index, const chowstring & key,
                                const chowstring & value)
{
    AssociateArrayItem * item = store[index];
    if (item == NULL) {
        item = &((*map)[key]);
        store[index] = item;
    }
    item->string = value;
}

int AssociateArray::get_value(int index, const chowstring & key)
{
    AssociateArrayItem * item = store[index];
    if (item != NULL)
        return item->value;
    ArrayMap::iterator it = map->find(key);
    if (it == map->end())
        return 0;
    store[index] = &it->second;
    return it->second.value;
}

const chowstring & AssociateArray::get_string(int index,
                                               const chowstring & key)
{
    AssociateArrayItem * item = store[index];
    if (item != NULL)
        return item->string;
    ArrayMap::iterator it = map->find(key);
    if (it == map->end())
        return empty_string;
    store[index] = &it->second;
    return it->second.string;
}

bool AssociateArray::has_key(int index, const chowstring & key)
{
    if (store[index] != NULL)
        return true;
    ArrayMap::iterator it = map->find(key);
    if (it == map->end())
        return false;
    store[index] = &it->second;
    return true;
}

ArrayMap AssociateArray::global_map;

static ArrayMap default_map;

class DefaultArray : public AssociateArray
{
public:
    DefaultArray()
    : AssociateArray(0, 0, 0)
    {
        map = &default_map;
        setup_default_instance(this);
    }

    ~DefaultArray()
    {
        map = &global_map;
    }
};

static DefaultArray default_assarray;
FrameObject * default_assarray_instance = &default_assarray;
