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
#include <iostream>
#include <android/log.h>
#include "platform.h"
#include <SDL_rwops.h>

namespace ChowdrenAudio
{
    void pause_audio();
    void resume_audio();
}

static chowstring current_log_line;

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
            __android_log_print(ANDROID_LOG_INFO, "Chowdren",
                                current_log_line.c_str());
            current_log_line.clear();
        } else {
            current_log_line += c;
        }
        return c;
    }
};

#ifdef USE_ASSET_MANAGER
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/configuration.h>
#include <SDL_system.h>
static jobject java_asset_manager;
AAssetManager * global_asset_manager;
static chowstring internal_path;
static chowstring android_language("English");
chowstring obb_path;

extern "C" JNIEnv *Android_JNI_GetEnv(void);

static jobject java_activity_context;

#ifdef CHOWDREN_USE_GOOGLEPLAY
#include "gpg/gpg.h"
std::unique_ptr<gpg::GameServices> game_services;
static bool in_auth = true;
#endif

void init_asset_manager()
{
    jmethodID mid;

    JNIEnv *env = Android_JNI_GetEnv();
    const int capacity = 16;
    env->PushLocalFrame(capacity);

    jclass mActivityClass = env->FindClass("org/libsdl/app/SDLActivity");

    /* context = SDLActivity.getContext(); */
    mid = env->GetStaticMethodID(mActivityClass,
            "getContext","()Landroid/content/Context;");
    jobject context = env->CallStaticObjectMethod(mActivityClass, mid);
    java_activity_context = env->NewGlobalRef(context);

#ifdef CHOWDREN_USE_GOOGLEPLAY
    gpg::AndroidPlatformConfiguration platform_configuration;
    platform_configuration.SetActivity(java_activity_context);
    gpg::GameServices::Builder builder;

    game_services = builder
        .SetOnAuthActionStarted([](gpg::AuthOperation op) {
            if (op == gpg::AuthOperation::SIGN_IN)
                in_auth = true;
            std::cout << "on auth started: " << op << std::endl;
        }).SetOnAuthActionFinished([](gpg::AuthOperation op,
                                   gpg::AuthStatus status) {
            std::cout << "on auth fin: " << op << " " << status << std::endl;
            if (op != gpg::AuthOperation::SIGN_IN)
                return;
            in_auth = false;
            if (!game_services->IsAuthorized())
                game_services->StartAuthorizationUI();
        }).Create(platform_configuration);
#endif

    /* assetManager = context.getAssets(); */
    mid = env->GetMethodID(env->GetObjectClass(context),
            "getAssets", "()Landroid/content/res/AssetManager;");
    jobject asset_manager = env->CallObjectMethod(context, mid);
    java_asset_manager = env->NewGlobalRef(asset_manager);
    global_asset_manager = AAssetManager_fromJava(env, java_asset_manager);

#ifdef CHOWDREN_USE_GOOGLEPLAY
    /* String path = SDLActivity.getObbPath(); */
    mid = env->GetStaticMethodID(mActivityClass,
            "getObbPath", "()Ljava/lang/String;");
    jstring obb;
    while (true) {
        obb = (jstring)env->CallStaticObjectMethod(mActivityClass, mid);
        if (obb != NULL)
            break;
        platform_sleep(0.1);
    }

    const char* obb_utf8 = env->GetStringUTFChars(obb, 0);
    obb_path = std::string(obb_utf8, env->GetStringUTFLength(obb));
    env->ReleaseStringUTFChars(obb, obb_utf8);
#endif

    env->PopLocalFrame(NULL);

    internal_path = SDL_AndroidGetInternalStoragePath();

    AConfiguration * config = AConfiguration_new();
    AConfiguration_fromAssetManager(config, global_asset_manager);
    char lang[2];
    AConfiguration_getLanguage(config, lang);
    AConfiguration_delete(config);
    #define MAKE_LANG(a, b) (a | (b << 8))
    unsigned int lang_id = MAKE_LANG(lang[0], lang[1]);

    switch (lang_id) {
        case MAKE_LANG('e', 'n'):
            android_language = "English";
            break;
        case MAKE_LANG('i', 't'):
            android_language = "Italian";
            break;
        case MAKE_LANG('e', 's'):
            android_language = "Spanish";
            break;
        case MAKE_LANG('d', 'e'):
            android_language = "German";
            break;
        case MAKE_LANG('f', 'r'):
            android_language = "French";
            break;
        default:
            android_language = "English";
            break;
    }
    #undef MAKE_LANG
}
#endif

// JNI handlers
extern "C"
{
    jint SDL_JNI_OnLoad(JavaVM* vm, void* reserved);
    jint JNI_OnLoad(JavaVM* vm, void* reserved)
    {
    #ifdef CHOWDREN_USE_GOOGLEPLAY
        gpg::AndroidInitialization::JNI_OnLoad(vm);
    #endif
        return SDL_JNI_OnLoad(vm, reserved);
    }

    void Java_org_libsdl_app_SDLActivity_onNativeActivityResult(
                                        JNIEnv* env, jobject thiz,
                                        jobject activity, jint requestCode,
                                        jint resultCode, jobject data)
    {
    #ifdef CHOWDREN_USE_GOOGLEPLAY
        gpg::AndroidSupport::OnActivityResult(env, activity, requestCode,
                                              resultCode, data);
    #endif
    }
}

void android_check_auth()
{
#ifdef CHOWDREN_USE_GOOGLEPLAY
    static int count = 0;
        count++;

    if (!in_auth)
        return;

    ChowdrenAudio::pause_audio();
    while (in_auth) {
        platform_sleep(0.1);
    }
    ChowdrenAudio::resume_audio();
#endif
}

void platform_minimize()
{
    jmethodID mid;
    const int capacity = 16;
    JNIEnv *env = Android_JNI_GetEnv();
    env->PushLocalFrame(capacity);
    mid = env->GetMethodID(env->GetObjectClass(java_activity_context),
            "moveTaskToBack", "(Z)Z");
    jboolean ret = env->CallBooleanMethod(java_activity_context, mid, true);
    env->PopLocalFrame(NULL);
}

void platform_init_android()
{
    static LogBuffer ob;
    std::streambuf * cout_sb = std::cout.rdbuf(&ob);
    std::streambuf * cerr_sb = std::cerr.rdbuf(&ob);
    __android_log_print(ANDROID_LOG_INFO, "Chowdren", "Initialized logbuffer");
#ifdef USE_ASSET_MANAGER
    init_asset_manager();
#endif

    android_check_auth();
}

const chowstring & platform_get_language()
{
    return android_language;
}

#ifdef USE_ASSET_MANAGER

void platform_walk_folder(const chowstring & in_path,
                          FolderCallback & callback)
{
}

size_t platform_get_file_size(const char * filename)
{
    FSFile fp(filename, "r");
    if (!fp.is_open())
        return 0;
    return fp.get_size();
}

bool platform_is_directory(const chowstring & value)
{
    return false;
}

bool platform_is_file(const chowstring & value)
{
    FSFile fp(value.c_str(), "r");
    if (!fp.is_open())
        return 0;
    return fp.get_size();
}

bool platform_path_exists(const chowstring & value)
{
    return platform_is_file(value);
}

void platform_create_directories(const chowstring & value)
{
}

#else

void platform_walk_folder(const chowstring & in_path,
                          FolderCallback & callback)
{
}

size_t platform_get_file_size(const char * filename)
{
    chowstring path = convert_path(filename);
    SDL_RWops * rw = SDL_RWFromFile(path.c_str(), "rb");
    if (rw == NULL)
        return 0;
    size_t size = SDL_RWsize(rw);
    SDL_RWclose(rw);
    return size;
}

bool platform_is_directory(const chowstring & value)
{
    return false;
}

bool platform_is_file(const chowstring & value)
{
    chowstring path = convert_path(value);
    SDL_RWops * rw = SDL_RWFromFile(path.c_str(), "rb");
    if (rw != NULL) {
        SDL_RWclose(rw);
        return true;
    }
    return false;
}

bool platform_path_exists(const chowstring & value)
{
    return platform_is_file(value);
}

void platform_create_directories(const chowstring & value)
{
}
#endif

const chowstring & platform_get_appdata_dir()
{
    static chowstring dir(".");
    return dir;
}

#ifdef CHOWDREN_USE_GAMECIRCLE

#include "AchievementsClientInterface.h"

void platform_unlock_achievement(const chowstring & name)
{
    AmazonGames::AchievementsClientInterface::updateProgress(
        name.c_str(), 100.0f, 0);
}

#elif CHOWDREN_USE_GOOGLEPLAY

void platform_unlock_achievement(const chowstring & name)
{
    if (!game_services->IsAuthorized())
        return;
    game_services->Achievements().Unlock(name);
}

void platform_open_achievements()
{
    if (!game_services->IsAuthorized())
        return;
    game_services->Achievements().ShowAllUI();
}

#else

void platform_unlock_achievement(const chowstring & name)
{
}

#endif

#include "../desktop/platform.cpp"
