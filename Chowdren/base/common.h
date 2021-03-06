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

#ifndef CHOWDREN_COMMON_H
#define CHOWDREN_COMMON_H

#include "chowconfig.h"
#include "render.h"
#include "profiler.h"
#include "keydef.h"
#include "keyconv.h"
#include "manager.h"
#include "platform.h"
#include "chowstring.h"
#include <list>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <algorithm>
#include <string.h>
#include "stringcommon.h"
#include "datastream.h"
#include <ctype.h>
#include "globals.h"
#include "image.h"
#include "frameobject.h"
#include "collision.h"
#include "alterables.h"
#include "color.h"
#include "mathcommon.h"
#include "path.h"
#include "types.h"
#include "crossrand.h"
#include "utility.h"
#include "input.h"
#include "movement.h"
#include "intern.h"
#include "overlap.h"

// string helpers

inline int string_find(const chowstring & a, const chowstring & b,
                       int pos)
{
    if (pos == -1)
        pos = 0;
    size_t ret = a.find(b, pos);
    if (ret == chowstring::npos)
        return -1;
    return ret;
}

inline int string_rfind(const chowstring & a, const chowstring & b,
                        int pos)
{
    if (pos == -1)
        pos = 0;
    size_t ret = a.rfind(b, pos);
    if (ret == chowstring::npos)
        return -1;
    return ret;
}

inline int string_size(const chowstring & a)
{
    return a.size();
}

inline chowstring lowercase_string(const chowstring & str)
{
	chowstring v(str);
    std::transform(v.begin(), v.end(), v.begin(),
                   static_cast<int (*)(int)>(tolower));
    return v;
}

inline chowstring uppercase_string(const chowstring & str)
{
	chowstring v(str);
    std::transform(v.begin(), v.end(), v.begin(),
                   static_cast<int(*)(int)>(toupper));
    return v;
}

inline chowstring right_string(const chowstring & v, int count)
{
    count = clamp(count, 0, int(v.size()));
    int index = int(v.size()) - count;
    return v.substr(index, count);
}

inline chowstring mid_string(const chowstring & v, int index, int count)
{
    int size = int(v.size());
    index = clamp(index, 0, size);
    count = clamp(count, 0, size - index);
    return v.substr(index, count);
}

inline chowstring left_string(const chowstring & v, int count)
{
    count = clamp(count, 0, int(v.size()));
    return v.substr(0, count);
}

class Font
{
public:
    chowstring name;
    int size;
    bool bold;
    bool italic;
    bool underline;

    Font(const char * name, int size, bool bold, bool italic, bool underline);
};

// static objects

class FTTextureFont;
FTTextureFont * get_font(int size, int flags = 0);
void set_font_path(const char * path);
void set_font_path(const chowstring & path);
bool init_font();

#define NONE_GRADIENT 0
#define VERTICAL_GRADIENT 1
#define HORIZONTAL_GRADIENT 2

void draw_gradient(int x1, int y1, int x2, int y2, int gradient_type,
                   Color color, Color color2, int alpha);

#include "fileop.h"

#include "extensions.h"

inline void reset_global_data()
{
#ifdef CHOWDREN_USE_INIPP
    INI::reset_global_data();
#endif
}

// event helpers

#include "mathhelper.h"

// get_single for ObjectList

inline FrameObject * get_single(ObjectList & list)
{
    return list.back_selection();
}

inline FrameObject * get_single(ObjectList & list, int index)
{
    return list.get_wrapped_selection(index);
}

inline FrameObject * get_single(ObjectList & list, FrameObject * def)
{
    FrameObject * ret = list.back_selection();
    if (ret == NULL)
        return def;
    return ret;
}

inline FrameObject * get_single(ObjectList & list, int index,
                                FrameObject * def)
{
    FrameObject * ret = list.get_wrapped_selection(index);
    if (ret == NULL)
        return def;
    return ret;
}

// get_single for QualifierList

inline FrameObject * get_single(QualifierList & list)
{
    return list.back_selection();
}

inline FrameObject * get_single(QualifierList & list, int index)
{
    return list.get_wrapped_selection(index);
}

inline FrameObject * get_single(QualifierList & list, FrameObject * def)
{
    FrameObject * ret = list.back_selection();
    if (ret == NULL)
        return def;
    return ret;
}

inline FrameObject * get_single(QualifierList & list, int index,
                                FrameObject * def)
{
    FrameObject * ret = list.get_wrapped_selection(index);
    if (ret == NULL)
        return def;
    return ret;
}

// get_single for SavedSelection

inline FrameObject * get_single(SavedSelection & list)
{
    if (list.count <= 0)
        return NULL;
    return list.items[0];
}

inline FrameObject * get_single(SavedSelection & list, int index)
{
    if (list.count <= 0)
        return NULL;
    index %= list.count;
    return list.items[index];
}

inline FrameObject * get_single(SavedSelection & list, FrameObject * def)
{
    if (list.count <= 0)
        return def;
    return list.items[0];
}

inline FrameObject * get_single(SavedSelection & list, int index,
                                FrameObject * def)
{
    if (list.count <= 0)
        return def;
    index %= list.count;
    return list.items[index];
}

// pick_random

inline FrameObject * pick_random(ObjectList & instances)
{
    int size = 0;
    for (ObjectIterator it(instances); !it.end(); ++it) {
        if ((*it)->flags & (DISABLE_COL | DESTROYING)) {
            it.deselect();
            continue;
        }
        size++;
    }
    if (size == 0)
        return NULL;
    int index = randrange(size);
    for (ObjectIterator it(instances); !it.end(); ++it) {
        if (index == 0) {
            it.select_single();
            return *it;
        }
        index--;
    }
    return NULL;
}

inline FrameObject * pick_random(QualifierList & instances)
{
    int size = 0;
    for (QualifierIterator it(instances); !it.end(); ++it) {
        if ((*it)->flags & (DISABLE_COL | DESTROYING)) {
            it.deselect();
            continue;
        }
        size++;
    }
    if (size == 0)
        return NULL;
    int index = randrange(size);
    for (QualifierIterator it(instances); !it.end(); ++it) {
        if (index == 0) {
            it.select_single();
            return *it;
        }
        index--;
    }
    return NULL;
}

#ifdef CHOWDREN_USE_VALUEADD

inline void spread_value(QualifierList & instances, int key, int start,
                         int step)
{
    for (QualifierIterator it(instances); !it.end(); it++) {
        (*it)->get_extra_alterables().set_value(key, start);
        start += step;
    }
}

inline void spread_value(ObjectList & instances, int key, int start, int step)
{
    for (ObjectIterator it(instances); !it.end(); it++) {
        (*it)->get_extra_alterables().set_value(key, start);
        start += step;
    }
}

#endif

inline void spread_value(FrameObject * obj, int alt, int start)
{
    obj->alterables->values.set(alt, start);
}

inline void spread_value(ObjectList & instances, int alt, int start)
{
    for (ObjectIterator it(instances); !it.end(); ++it) {
        (*it)->alterables->values.set(alt, start);
        start++;
    }
}

inline void spread_value(QualifierList & instances, int alt, int start)
{
    for (QualifierIterator it(instances); !it.end(); ++it) {
        (*it)->alterables->values.set(alt, start);
        start++;
    }
}

inline int objects_in_zone(ObjectList & instances,
                           int x1, int y1, int x2, int y2)
{
    int count = 0;
    for (ObjectIterator it(instances); !it.end(); ++it) {
        FrameObject * obj = *it;
        if (obj->flags & (FADEOUT | DESTROYING))
            continue;
        int x = obj->get_x();
        int y = obj->get_y();
        if (x < x1 || x >= x2 || y < y1 || y >= y2)
            continue;
        count++;
    }
    return count;
}

inline int objects_in_zone(QualifierList & instances,
                           int x1, int y1, int x2, int y2)
{
    int count = 0;
    for (QualifierIterator it(instances); !it.end(); ++it) {
        FrameObject * obj = *it;
        if (obj->flags & (FADEOUT | DESTROYING))
            continue;
        int x = obj->get_x();
        int y = obj->get_y();
        if (x < x1 || x >= x2 || y < y1 || y >= y2)
            continue;
        count++;
    }
    return count;
}

inline void pick_objects_in_zone(ObjectList & instances,
                                 int x1, int y1, int x2, int y2)
{
    for (ObjectIterator it(instances); !it.end(); ++it) {
        FrameObject * obj = *it;
        if (obj->flags & (FADEOUT | DESTROYING)) {
            it.deselect();
            continue;
        }
        int x = obj->get_x();
        int y = obj->get_y();
        if (x < x1 || x >= x2 || y < y1 || y >= y2)
            it.deselect();
    }
}

inline void pick_objects_in_zone(QualifierList & instances,
                                 int x1, int y1, int x2, int y2)
{
    for (QualifierIterator it(instances); !it.end(); ++it) {
        FrameObject * obj = *it;
        if (obj->flags & (FADEOUT | DESTROYING)) {
            it.deselect();
            continue;
        }
        int x = obj->get_x();
        int y = obj->get_y();
        if (x < x1 || x >= x2 || y < y1 || y >= y2)
            it.deselect();
    }
}

inline void set_random_seed(int seed)
{
    cross_srand(seed);
}

inline void open_process(const chowstring & exe, const chowstring & cmd,
                         int pad)
{

}

inline void transform_pos(int & x, int & y, FrameObject * parent)
{
    double c, s;
    get_dir(parent->direction, c, s);
    int new_x = int(double(x) * c - double(y) * s);
    int new_y = int(double(x) * s + double(y) * c);
    x = new_x;
    y = new_y;
}

void swap_position(const FlatObjectList & value);

inline void set_cursor_visible(bool value)
{
    if (value)
        platform_show_mouse();
    else
        platform_hide_mouse();
}

inline chowstring get_command_arg(const chowstring & arg)
{
    // XXX implement, maybe
    return "";
}

chowstring get_md5(const chowstring & value);

template <typename T, typename T1>
inline T get_event_dummy(T value, T1 other)
{
    return value;
}

template <typename T, typename T1, typename T2>
inline T get_event_dummy(T value, T1 a, T2 b)
{
    return value;
}

inline int get_zero_dummy(const chowstring value)
{
    return 0;
}

chowstring get_joytokey_name(int value);

void create_joystick_rumble(int n, float delay, float duration,
                            float l, float r, const chowstring & name);
void start_joystick_rumble(int n, const chowstring & name, int times);

inline int get_ascii(const chowstring & value)
{
    if (value.empty())
        return 0;
    return int((unsigned char)value[0]);
}

inline int reverse_color(int value)
{
    Color color(value);
    color.r = 255 - color.r;
    color.g = 255 - color.g;
    color.b = 255 - color.b;
    color.a = 0;
    return color.get_int();
}

const chowstring & get_platform();

#ifdef CHOWDREN_EMULATE_MENU

// menu emulation

bool is_menu_checked(unsigned int id);
void check_menu(unsigned int id);
void uncheck_menu(unsigned int id);
void activate_menu(unsigned int id);
void deactivate_menu(unsigned int id);

#endif

#ifdef CHOWDREN_IS_NL2
int nl2_get_reload_pos();
#endif

#endif // CHOWDREN_COMMON_H
