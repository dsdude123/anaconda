#include <string>
#include <sndfile.h>
#include <set>
#include <algorithm>
#include <al.h>
#include <alc.h>
#include <tinythread/tinythread.h>
#include <math.h>

namespace ChowdrenAudio {

// extension function pointers
typedef ALvoid (AL_APIENTRY*PFNALBUFFERSUBDATASOFTPROC)(ALuint, ALenum, 
    const ALvoid*, ALsizei, ALsizei);
PFNALBUFFERSUBDATASOFTPROC alBufferSubDataSOFT;

void _al_check(const std::string& file, unsigned int line)
{
    // Get the last error
    ALenum error_num = alGetError();

    if (error_num != AL_NO_ERROR)
    {
        std::string error, description;

        // Decode the error code
        switch (error_num)
        {
            case AL_INVALID_NAME:
                error = "AL_INVALID_NAME";
                description = "an unacceptable name has been specified";
                break;
            case AL_INVALID_ENUM:
                error = "AL_INVALID_ENUM";
                description = "an unacceptable value has been specified for an "
                              "enumerated argument";
                break;
            case AL_INVALID_VALUE:
                error = "AL_INVALID_VALUE";
                description = "a numeric argument is out of range";
                break;
            case AL_INVALID_OPERATION:
                error = "AL_INVALID_OPERATION";
                description = "the specified operation is not allowed in the "
                              "current state";
                break;
            case AL_OUT_OF_MEMORY:
                error = "AL_OUT_OF_MEMORY";
                description = "there is not enough memory left to execute the "
                              "command";
                break;
        }

        // Log the error
        std::cerr << "An internal OpenAL call failed in "
            << file.substr(file.find_last_of("\\/") + 1) << " (" << line <<
            ") : " << error << ", " << description << std::endl;
#ifdef _WIN32
        // let's debug on Windows
        __debugbreak();
#endif
    }
}

#define al_check(Func) ((Func), _al_check(__FILE__, __LINE__))

class SoundStream;

class AudioDevice
{
public:
    ALboolean direct_channels_ext, sub_buffer_data_ext;
    tthread::thread * streaming_thread;
    std::vector<SoundStream*> streams;
    tthread::recursive_mutex stream_mutex;

    AudioDevice();
    static void _stream_update(void * data);
    void stream_update();
    void add_stream(SoundStream*);
    void remove_stream(SoundStream*);
};

AudioDevice * global_device = NULL;

void open_audio()
{
    if (global_device != NULL)
        return;
    global_device = new AudioDevice;
}

class SoundFile
{
public:
    SNDFILE * file;
    std::size_t samples;
    unsigned int channels;
    unsigned int sample_rate;

    SoundFile(const std::string & filename)
    : file(NULL)
    {
        SF_INFO info;
        file = sf_open(filename.c_str(), SFM_READ, &info);
        if (!file) {
            std::cerr << "Failed to open sound file \"" << filename << "\" (" 
                << sf_strerror(file) << ")" << std::endl;
            return;
        }

        channels = info.channels;
        sample_rate = info.samplerate;
        samples = static_cast<std::size_t>(info.frames) * info.channels;

        // if (info.format & SF_FORMAT_VORBIS)
        //     sf_command(file, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE);
    }

    std::size_t read(signed short * data, std::size_t samples)
    {
        if (data && samples)
            return static_cast<std::size_t>(sf_read_short(file, data, samples));
        else
            return 0;
    }

    void seek(double value)
    {
        sf_count_t offset = static_cast<sf_count_t>(value * sample_rate);
        sf_seek(file, offset, SEEK_SET);
    }

    ~SoundFile()
    {
        if (!file)
            return;
        sf_close(file);
    }
};

ALenum get_format(unsigned int channels)
{
    switch (channels)
    {
        case 1:
            return AL_FORMAT_MONO16;
        case 2:
            return AL_FORMAT_STEREO16;
        case 4:
            return alGetEnumValue("AL_FORMAT_QUAD16");
        case 6:
            return alGetEnumValue("AL_FORMAT_51CHN16");
        case 7:
            return alGetEnumValue("AL_FORMAT_61CHN16");
        case 8:
            return alGetEnumValue("AL_FORMAT_71CHN16");
        default:
            return 0;
    }
}

class SoundBuffer
{
public:
    ALuint buffer;
    std::size_t samples_size;
    signed short * samples;
    std::size_t sample_count;
    unsigned int sample_rate;
    double left_gain, right_gain;
    unsigned int channels;
    ALenum format;

    SoundBuffer(unsigned int sample_rate, unsigned int channels, ALenum format)
    : left_gain(1.0), right_gain(1.0), sample_rate(sample_rate), format(format),
      samples_size(0), samples(NULL), channels(channels)
    {
        al_check(alGenBuffers(1, &buffer));
    }

    SoundBuffer(SoundFile & file, size_t sample_count)
    : left_gain(1.0), right_gain(1.0), samples_size(0), samples(NULL)
    {
        al_check(alGenBuffers(1, &buffer));
        channels = file.channels;
        format = get_format(channels);
        sample_rate = file.sample_rate;
        read(file, sample_count);
    }

    bool read(SoundFile & file, size_t read_samples)
    {
        if (samples_size < read_samples) {
            delete[] samples;
            samples = new signed short[read_samples];
            samples_size = read_samples;
        }
        sample_count = file.read(samples, read_samples);
        buffer_data();
        return sample_count == read_samples;
    }

    bool read(SoundFile & file)
    {
        return read(file, sample_rate * channels);
    }

    void buffer_data(bool updated = false)
    {
        ALsizei size = static_cast<ALsizei>(sample_count
            ) * sizeof(signed short);
        signed short * buffer_data = samples;
        bool del = false;
        if (format == AL_FORMAT_STEREO16 && 
                (left_gain != 1.0 || right_gain != 1.0)) {
            del = true;
            buffer_data = new signed short[sample_count];
            memcpy(buffer_data, samples, size);
            for (unsigned int i = 0; i < sample_count; i += 2) {
                buffer_data[i] *= left_gain;
                buffer_data[i+1] *= right_gain;
            }
        }
        if (updated)
            al_check(alBufferSubDataSOFT(buffer, format, buffer_data, 0,
                size));
        else
            al_check(alBufferData(
                buffer, format, buffer_data, size, sample_rate));
        if (del)
            delete[] buffer_data;
    }

    void set_pan(double left, double right)
    {
        if (format != AL_FORMAT_STEREO16 || samples == NULL)
            return;
        left_gain = left;
        right_gain = right;
        if (!global_device->sub_buffer_data_ext)
            // this is bad
            return;
        buffer_data(true);
    }

    ~SoundBuffer()
    {
        delete[] samples;
        samples = NULL;
        al_check(alDeleteBuffers(1, &buffer));
    }
};

class Sound;
typedef std::set<Sound*> SoundList;

class Sample
{
public:
    SoundBuffer * buffer;
    double duration; // duration in seconds
    unsigned int sample_rate;
    unsigned int channels;
    SoundList sounds;

    Sample(const std::string & filename);
    ~Sample();
    void add_sound(Sound* sound);
    void remove_sound(Sound* sound);
};

#define AL_DIRECT_CHANNELS_SOFT 0x1033

template <class T>
inline T clamp(T value)
{
    return std::min<T>(1, std::max<T>(0, value));
}

double get_pan_factor(double pan)
{
    if (pan == 1.0)
        return 1.0;
    else if (pan == 0.0)
        return 0.0;
    // directsound algorithmic scale, taken from wine sources
    return clamp(pow(2.0, (pan * 10000) / 600.0) / 65535.0);
}

class SoundBase
{
public:
    enum Status
    {
        Stopped,
        Paused,
        Playing
    };

    ALuint source;
    ALenum format;
    double pan, left_gain, right_gain;
    double volume;
    float pitch;
    bool closed;

    void initialize()
    {
        left_gain = right_gain = 1.0;
        al_check(alGenSources(1, &source));
        if (global_device->direct_channels_ext)
            al_check(alSourcei(source, AL_DIRECT_CHANNELS_SOFT, AL_TRUE));
        closed = false;
    }

    void set_pitch(float pitch)
    {
        this->pitch = pitch;
        al_check(alSourcef(source, AL_PITCH, pitch));
    }

    float get_pitch()
    {
        return pitch;
    }

    void set_volume(float value)
    {
        volume = value;
        al_check(alSourcef(source, AL_GAIN, volume));
    }

    float get_volume()
    {
        return volume;
    }

    void set_pan(double value)
    {
        if (value > 1.0)
            value = 1.0;
        else if (value < -1.0)
            value = -1.0;
        if (value == pan)
            return;
        pan = value;
        if (format == AL_FORMAT_STEREO16) {
            left_gain = get_pan_factor(clamp(1.0 - pan));
            right_gain = get_pan_factor(clamp(1.0 + pan));
            update_stereo_pan();

        } else
            al_check(alSource3f(source, AL_POSITION, pan, 
                                -sqrt(1.0 - pan * pan), 0));
    }

    virtual void update_stereo_pan()
    {

    }

    virtual Status get_status()
    {
        if (closed)
            return Stopped;

        ALint status;
        al_check(alGetSourcei(source, AL_SOURCE_STATE, &status));

        switch (status) {
            case AL_INITIAL:
            case AL_STOPPED: 
                return Stopped;
            case AL_PAUSED:
                return Paused;
            case AL_PLAYING:
                return Playing;
        }

        return Stopped;
    }

    void set_frequency(int value)
    {
        set_pitch(double(value) / get_sample_rate());
    }

    int get_frequency()
    {
        return get_pitch() * get_sample_rate();
    }

    virtual void play() = 0;
    virtual void stop() = 0;
    virtual int get_sample_rate() = 0;
    virtual void set_loop(bool) = 0;
    virtual void set_playing_offset(double) = 0;
    virtual double get_playing_offset() = 0;

    virtual ~SoundBase()
    {
        al_check(alSourcei(source, AL_BUFFER, 0));
        al_check(alDeleteSources(1, &source));
    }
};

class Sound : public SoundBase
{
public:
    Sample & sample;

    Sound(Sample & sample) : sample(sample)
    {
        initialize();
        al_check(alSourcei(source, AL_BUFFER, sample.buffer->buffer));
        sample.add_sound(this);
    }

    ~Sound()
    {
        sample.remove_sound(this);
        reset_buffer();
    }

    void play()
    {
        al_check(alSourcePlay(source));
    }

    void pause()
    {
        al_check(alSourcePause(source));
    }

    void stop()
    {
        al_check(alSourceStop(source));
    }

    bool get_loop()
    {
        ALint loop;
        al_check(alGetSourcei(source, AL_LOOPING, &loop));
        return loop != 0;
    }

    void set_loop(bool loop)
    {
        al_check(alSourcei(source, AL_LOOPING, loop));
    }

    double get_playing_offset()
    {
        ALfloat secs = 0.0;
        al_check(alGetSourcef(source, AL_SEC_OFFSET, &secs));
        return secs;
    }

    void set_playing_offset(double offset)
    {
        al_check(alSourcef(source, AL_SEC_OFFSET, offset));
    }

    int get_sample_rate()
    {
        return sample.sample_rate;
    }

    void update_stereo_pan()
    {
        sample.buffer->set_pan(left_gain, right_gain);
    }

    void reset_buffer()
    {
        if (closed)
            return;
        stop();
        al_check(alSourcei(source, AL_BUFFER, 0));
        closed = true;
    }
};

#define BUFFER_COUNT 3

#if defined(_MSC_VER)
    typedef signed __int64 Int64;
    typedef unsigned __int64 Uint64;
#else
    typedef signed long long Int64;
    typedef unsigned long long Uint64;
#endif

#define LOCK_STREAM global_device->stream_mutex.lock
#define UNLOCK_STREAM global_device->stream_mutex.unlock

class SoundStream : public SoundBase
{
public:
    SoundFile * file;
    bool playing;
    SoundBuffer * buffers[BUFFER_COUNT];
    unsigned int channels;
    unsigned int sample_rate;
    bool loop;
    Uint64 samples_processed;
    bool end_buffers[BUFFER_COUNT];
    bool stopping;

    SoundStream(const std::string & filename)
    : playing(false), loop(false), stopping(false)
    {
        file = new SoundFile(filename);
        format = get_format(file->channels);

        for (int i = 0; i < BUFFER_COUNT; ++i)
            buffers[i] = new SoundBuffer(file->sample_rate, file->channels,
                                         format);

        initialize();

        LOCK_STREAM();
        global_device->add_stream(this);
        UNLOCK_STREAM();
    }

    ~SoundStream()
    {
        stop();

        LOCK_STREAM();
        global_device->remove_stream(this);
        UNLOCK_STREAM();
    }

    void play()
    {
        // If the sound is already playing (probably paused), just resume it
        if (playing) {
            al_check(alSourcePlay(source));
            return;
        }

        // Move to the beginning
        on_seek(0);

        samples_processed = 0;

        for (int i = 0; i < BUFFER_COUNT; ++i) {
            end_buffers[i] = false;
        }

        stopping = fill_queue();
        al_check(alSourcePlay(source));

        playing = true;
    }

    void pause()
    {
        al_check(alSourcePause(source));
    }

    void stop()
    {
        if (!playing) {
            return;
        }
        LOCK_STREAM();
        playing = false;
        UNLOCK_STREAM();
        al_check(alSourceStop(source));
        clear_queue();
        al_check(alSourcei(source, AL_BUFFER, 0));
        for (int i = 0; i < BUFFER_COUNT; i++)
            delete buffers[i];
    }

    Status get_status()
    {
        Status status = SoundBase::get_status();

        // To compensate for the lag between play() and alSourceplay()
        if ((status == Stopped) && playing)
            status = Playing;

        return status;
    }

    void set_playing_offset(double time)
    {
        LOCK_STREAM();
        al_check(alSourceStop(source));
        clear_queue();
        al_check(alSourcei(source, AL_BUFFER, 0));
        on_seek(time);
        samples_processed = static_cast<Uint64>(
            time * file->sample_rate * file->channels);
        for (int i = 0; i < BUFFER_COUNT; ++i)
            end_buffers[i] = false;
        stopping = fill_queue();    
        al_check(alSourcePlay(source));
        UNLOCK_STREAM();
    }

    double get_playing_offset()
    {
        ALfloat secs = 0.0f;
        al_check(alGetSourcef(source, AL_SEC_OFFSET, &secs));
        return secs + static_cast<float>(samples_processed
            ) / file->sample_rate / file->channels;
    }

    void set_loop(bool loop)
    {
        this->loop = loop;
    }

    bool get_loop()
    {
        return loop;
    }

    int get_sample_rate()
    {
        return file->sample_rate;
    }

    void update()
    {
        if (!playing)
            return;

        ALint status;
        al_check(alGetSourcei(source, AL_SOURCE_STATE, &status));

        // The stream has been interrupted!
        if (status == AL_STOPPED) {
            if (stopping) {
                stop();
                return;
            } else
                al_check(alSourcePlay(source));
        }

        // Get the number of buffers that have been processed (ie. ready for 
        // reuse)
        ALint processed = 0;
        al_check(alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed));

        while (processed--) {
            // Pop the first unused buffer from the queue
            ALuint buffer;
            al_check(alSourceUnqueueBuffers(source, 1, &buffer));

            // Find its number
            unsigned int buffer_num = 0;
            for (int i = 0; i < BUFFER_COUNT; ++i)
                if (buffers[i]->buffer == buffer) {
                    buffer_num = i;
                    break;
                }

            // Retrieve its size and add it to the samples count
            if (end_buffers[buffer_num]) {
                // This was the last buffer: reset the sample count
                samples_processed = 0;
                end_buffers[buffer_num] = false;
            }
            else {
                ALint size, bits;
                al_check(alGetBufferi(buffer, AL_SIZE, &size));
                al_check(alGetBufferi(buffer, AL_BITS, &bits));
                samples_processed += size / (bits / 8);
            }

            // Fill it and push it back into the playing queue
            if (!stopping) {
                if (fill_buffer(buffer_num))
                    stopping = true;
            }
        }
    }

    void on_seek(double offset)
    {
        // Lock lock(m_mutex);
        file->seek(offset);
    }

    bool fill_buffer(unsigned int buffer_num)
    {
        bool stopping = false;

        SoundBuffer & buffer = *buffers[buffer_num];

        if (!buffer.read(*file)) {
            // Mark the buffer as the last one (so that we know when to reset 
            // the playing position)
            end_buffers[buffer_num] = true;

            // Check if the stream must loop or stop
            if (loop) {
                // Return to the beginning of the stream source
                on_seek(0);

                // If we previously had no data, try to fill the buffer once 
                // again
                if (buffer.sample_count == 0) {
                    return fill_buffer(buffer_num);
                }
            }
            else {
                // Not looping: request stop
                stopping = true;
            }
        }

        if (buffer.sample_count != 0)
            al_check(alSourceQueueBuffers(source, 1, &buffer.buffer));

        return stopping;
    }

    bool fill_queue()
    {
        // Fill and enqueue all the available buffers
        bool stopping = false;
        for (int i = 0; (i < BUFFER_COUNT) && !stopping; ++i) {
            if (fill_buffer(i))
                stopping = true;
        }

        return stopping;
    }

    void clear_queue()
    {
        // Get the number of buffers still in the queue
        ALint queued;
        al_check(alGetSourcei(source, AL_BUFFERS_QUEUED, &queued));

        // Unqueue them all
        ALuint buffer;
        for (ALint i = 0; i < queued; ++i)
            al_check(alSourceUnqueueBuffers(source, 1, &buffer));
    }

    void update_stereo_pan()
    {
        LOCK_STREAM();
        for (int i = 0; i < BUFFER_COUNT; i++)
            buffers[i]->set_pan(left_gain, right_gain);
        UNLOCK_STREAM();
    }
};

// audio device implementation

AudioDevice::AudioDevice()
{
    ALCdevice *device = alcOpenDevice(NULL);
    if(!device) {
        std::cerr << "Device open failed" << std::endl;
        return;
    }

    ALCcontext *context = alcCreateContext(device, NULL);
    if(!context || alcMakeContextCurrent(context) == ALC_FALSE) {
        if(context)
            alcDestroyContext(context);
        alcCloseDevice(device);
        std::cerr << "Context setup failed" << std::endl;
        return;
    }

    std::cout << "Audio initialized: " << alGetString(AL_VERSION) <<
        ", " << alGetString(AL_RENDERER) << ", " << alGetString(AL_VENDOR)
        << std::endl;

    // OpenAL-Soft specific extensions
    direct_channels_ext = alIsExtensionPresent("AL_SOFT_direct_channels");
    sub_buffer_data_ext = alIsExtensionPresent("AL_SOFT_buffer_sub_data");
    if (sub_buffer_data_ext) {
        alBufferSubDataSOFT = (PFNALBUFFERSUBDATASOFTPROC)alGetProcAddress(
            "alBufferSubDataSOFT");
    }
    streaming_thread = new tthread::thread(_stream_update, NULL);
}

void AudioDevice::stream_update()
{
    while (true) {
        stream_mutex.lock();
        std::vector<SoundStream*>::const_iterator it;
        for (it = streams.begin(); it != streams.end(); it++)
            (*it)->update();
        stream_mutex.unlock();
        tthread::this_thread::sleep_for(tthread::chrono::milliseconds(125));
    }
}

void AudioDevice::_stream_update(void * data)
{
    global_device->stream_update();
}

void AudioDevice::add_stream(SoundStream * stream)
{
    streams.push_back(stream);
}

void AudioDevice::remove_stream(SoundStream * stream)
{
    streams.erase(std::remove(streams.begin(), streams.end(), stream), 
                  streams.end());
}

class Listener
{
public:
    static void set_volume(float volume)
    {
        al_check(alListenerf(AL_GAIN, volume));
    }

    static float get_volume()
    {
        float volume = 0.0f;
        al_check(alGetListenerf(AL_GAIN, &volume));
        return volume;
    }
};

// Sample implementation

Sample::Sample(const std::string & filename)
{
    SoundFile file(filename);
    std::size_t sample_count = file.samples;
    channels = file.channels;
    sample_rate = file.sample_rate;
    buffer = new SoundBuffer(file, file.samples);
    duration = double(sample_count) / sample_rate / channels;
}

Sample::~Sample()
{
    for (SoundList::const_iterator it = sounds.begin(); it != sounds.end();
         ++it)
        (*it)->reset_buffer();

    delete buffer;
}

void Sample::add_sound(Sound* sound)
{
    sounds.insert(sound);
}

void Sample::remove_sound(Sound* sound)
{
    sounds.erase(sound);
}

} // namespace ChowdrenAudio