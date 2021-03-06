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

#ifndef CHOWDREN_DIALOGEXT_H
#define CHOWDREN_DIALOGEXT_H

#include "chowstring.h"

class DialogObject
{
public:
    static void set_title(const chowstring & title);
    static void set_text(const chowstring & text);
    static void create(unsigned int id);
    static void set_ok();
    static void set_yes_no();
    static void set_modal();
    static void set_topmost();
    static void set_filter(const chowstring & filter, int count);
    static void set_default_filename(const chowstring & filename);
    static void set_default_extension(const chowstring & filename);
    static void set_default_directory(const chowstring & filename);
    static void open_load_selector(const chowstring & start);
    static void open_save_selector(const chowstring & start);
    static const chowstring & get_path();
    static bool is_file_success();
    static bool is_failure(unsigned int id);
    static bool is_success(unsigned int id);
};

#endif // CHOWDREN_DIALOGEXT_H
