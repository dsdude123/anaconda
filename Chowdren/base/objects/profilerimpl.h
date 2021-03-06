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

#include "chowstring.h"
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>

#ifdef COXSDK
#include <windows.h>
#include <fstream>
#else
#include "platform.h"
#include "fileio.h"
#endif

class ProfilerEntry
{
public:
    chowstring name;
    double start, dt;
};

class Profiler
{
public:
    std::vector<ProfilerEntry> entries;
    std::vector<ProfilerEntry> stack;
    ProfilerEntry current;

#ifdef COXSDK
    double qpc_freq;
    __int64 qpc_start;
#endif

    Profiler()
    {
        init_timer();
    }

    void start(const chowstring & name)
    {
        stack.push_back(current);
        current.name = name;
        current.dt = 0.0;
        current.start = get_timer();
    }

    void start_additive(const chowstring & name)
    {
        if (name == current.name)
            return;
        std::vector<ProfilerEntry>::iterator it;
        for (it = entries.begin(); it != entries.end(); it++) {
            if (it->name != name)
                continue;
            current = *it;
            entries.erase(it);
            current.start = get_timer();
            return;
        }

        start(name);
    }

    void stop()
    {
        current.dt += get_timer() - current.start;
        entries.push_back(current);
        current = stack.back();
        stack.pop_back();
    }

    void save(const chowstring & path)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(15);
        ss << "Profiler results:\n\n";

        std::vector<ProfilerEntry>::iterator it;
        for (it = entries.begin(); it != entries.end(); it++) {
            ss << it->name << ": " << it->dt << " seconds\n\n";
        }
#ifdef COXSDK
        std::ofstream fp(path.c_str());
#else
        FSFile fp(path.c_str(), "w");
#endif
        chowstring str = ss.str();

        fp.write(str.data(), str.size());
        fp.close();
    }

#ifdef COXSDK
    void init_timer()
    {
        // just expect that QPC is working
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        qpc_freq = double(li.QuadPart);
        QueryPerformanceCounter(&li);
        qpc_start = li.QuadPart;
    }

    double get_timer()
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        return double(li.QuadPart - qpc_start) / qpc_freq;
    }
#else
    static void init_timer()
    {
    }

    static double get_timer()
    {
        return platform_get_time();
    }
#endif
};