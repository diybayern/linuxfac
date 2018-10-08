
#ifndef __SOUND_TEST_H
#define __SOUND_TEST_H

#include <alsa/asoundlib.h>
#include "Control.h"
#include "FuncBase.h"

using namespace std;

struct SndInfo {
    int samplearate;
    int channels;
    int period_size;
    snd_pcm_format_t format;
    snd_pcm_stream_t direction;
    const char* card;

    snd_pcm_t* pcm;

    int status;
};

enum SndStatus {
    SOUND_RECORD_START = 0,
    SOUND_RECORD_STOP,
    SOUND_PLAYBACK_START,
    SOUND_PLAYBACK_STOP,
    SOUND_UNKNOW
};


class Control;

class SoundTest : public FuncBase
{
public:
    SoundTest();
    static bool start_playback();
    static bool stop_playback();

    static bool start_record();
    static bool stop_record();
    
    static void* test_all(void*);
    void start_test(BaseInfo* baseInfo);

    static bool init();

private:
    Control* _control;
    static int open_sound_card(SndInfo *info);
    static void close_sound_card(SndInfo *info);
    static void* record_loop(void *arg);
    static void* playback_loop(void *arg);
    static void init_volume();
};

#endif // __SOUND_TEST_H














