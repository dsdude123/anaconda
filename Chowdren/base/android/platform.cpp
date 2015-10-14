#include <string>
#include <iostream>
#include <android/log.h>
#include "platform.h"
#include <SDL_rwops.h>

static std::string current_log_line;

class LogBuffer : public std::streambuf
{
public:
    LogBuffer()
    {
        // no buffering, overflow on every char
        setp(0, 0);
    }

    virtual int_type overflow(int_type c = traits_type::eof())
    {
        if (c == '\n') {
            __android_log_print(ANDROID_LOG_INFO, "SDL",
                                current_log_line.c_str());
            current_log_line.clear();
        } else {
            current_log_line += c;
        }
        return c;
    }
};

void platform_init_android()
{
    static LogBuffer ob;
    std::streambuf * cout_sb = std::cout.rdbuf(&ob);
    std::streambuf * cerr_sb = std::cerr.rdbuf(&ob);
    __android_log_print(ANDROID_LOG_INFO, "SDL", "Initialized logbuffer");
}

void platform_walk_folder(const std::string & in_path,
                          FolderCallback & callback)
{
}

size_t platform_get_file_size(const char * filename)
{
    std::string path = convert_path(filename);
    SDL_RWops * rw = SDL_RWFromFile(path.c_str(), "rb");
    if (rw == NULL)
        return 0;
    size_t size = SDL_RWsize(rw);
    SDL_RWclose(rw);
    return size;
}

bool platform_is_directory(const std::string & value)
{
    return false;
}

bool platform_is_file(const std::string & value)
{
    std::string path = convert_path(value);
    SDL_RWops * rw = SDL_RWFromFile(path.c_str(), "rb");
    if (rw != NULL) {
        SDL_RWclose(rw);
        return true;
    }
    return false;
}

bool platform_path_exists(const std::string & value)
{
    return platform_is_file(value);
}

void platform_create_directories(const std::string & value)
{
}

const std::string & platform_get_appdata_dir()
{
    static std::string dir(".");
    return dir;
}

#include "../desktop/platform.cpp"
