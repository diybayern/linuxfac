
#include <alsa/asoundlib.h>
#include <pthread.h>

#include "SoundTest.h"

#define DEFAULT_SAMPLERATE     (44100)
#define DEFAULT_CHANNEL        (2)

#define DEFAULT_FORMAT         (SND_PCM_FORMAT_S16_LE)
#define PLAYBACK               (SND_PCM_STREAM_PLAYBACK)
#define RECORD                 (SND_PCM_STREAM_CAPTURE)

#define RECORD_FRAME_SIZE      (64)
#define PLAYBACK_FRAME_SIZE    (1024)    // It is a experienced value
#define SOUND_RECORD_FILE      ("/tmp/sound.wav")
#define DEFAULT_CARD_NAME      ("default")

static pthread_mutex_t gMutex;
//SoundTest* SoundTest::_mInstance = NULL;

SndInfo* SoundTest::g_record_info = new SndInfo();
SndInfo* SoundTest::g_playback_info = new SndInfo();
SndStatus SoundTest::gStatus = SOUND_UNKNOW;

SoundTest::~SoundTest()
{
    LOG_INFO("~SoundTest()");
    if (g_record_info != NULL) {
        delete g_record_info;
        g_record_info = NULL;
    }
    if (g_playback_info != NULL) {
        delete g_playback_info;
        g_playback_info = NULL;
    }
}

void SoundTest::init_volume()
{
    int ret = 0;
    string name = "";
    long minVolume = 0;
    long maxVolume = 0;
    snd_mixer_t *mixer_fd = NULL;
    snd_mixer_elem_t *elem = NULL;

    ret = snd_mixer_open(&mixer_fd, 0);
    if (ret < 0) {
        LOG_ERROR("open mixer failed ret=%d\n", ret);
        return;
    }

    ret = snd_mixer_attach(mixer_fd, g_record_info->card);
    if (ret < 0) {
        snd_mixer_close(mixer_fd);
        LOG_ERROR("attach mixer failed ret=%d\n", ret);
        return;
    }

    ret = snd_mixer_selem_register(mixer_fd, NULL, NULL);
    if (ret < 0) {
        snd_mixer_close(mixer_fd);
        LOG_ERROR("register mixer failed ret=%d \n", ret);
        return;
    }

    ret = snd_mixer_load(mixer_fd);
    if (ret < 0) {
        snd_mixer_close(mixer_fd);
        LOG_ERROR("load mixer failed ret=%d \n", ret);
        return;
    }

    for (elem = snd_mixer_first_elem(mixer_fd); elem; elem = snd_mixer_elem_next(elem)) {
        name = snd_mixer_selem_get_name(elem);
        LOG_INFO("name=%s \n", name.c_str());

        if ((name.compare("Master") == 0) || (name.compare("Headphone") == 0) || (name.compare("Speaker") == 0)
                || (name.compare("PCM") == 0) || (name.compare("Mic") == 0)) {
            //set maxvolume
            ret = snd_mixer_selem_get_playback_volume_range(elem, &minVolume, &maxVolume);
            LOG_INFO("name=%s get_playback_volume_range min=%ld max=%ld\n", name.c_str(), minVolume, maxVolume);
            snd_mixer_selem_set_playback_switch_all(elem, 1);
            if (ret == 0) {
                snd_mixer_selem_set_playback_volume_all(elem, maxVolume);
            } else {
                snd_mixer_selem_set_playback_volume_all(elem, 100);
            }
        } else if (name.compare("Mic Boost") == 0) {
            //set minvolume
            ret = snd_mixer_selem_get_playback_volume_range(elem, &minVolume, &maxVolume);
            LOG_INFO("name=%s get_playback_volume_range min=%ld max=%ld\n", name.c_str(), minVolume, maxVolume);
            snd_mixer_selem_set_capture_switch_all(elem, 1);
            if (ret == 0) {
                snd_mixer_selem_set_capture_volume_all(elem, minVolume);
            } else {
                snd_mixer_selem_set_capture_volume_all(elem, 0);
            }
        } else if (name.compare("Capture") == 0) {
            ret = snd_mixer_selem_get_capture_volume_range(elem, &minVolume, &maxVolume);
            LOG_INFO("name=%s get_capture_volume_range min=%ld max=%ld\n", name.c_str(), minVolume, maxVolume);
            snd_mixer_selem_set_capture_switch_all(elem, 1);

            if (ret == 0) {
                snd_mixer_selem_set_capture_volume_all(elem, maxVolume);
            } else {
                snd_mixer_selem_set_capture_volume_all(elem, 100);
            }
        }
    }

    snd_mixer_close(mixer_fd);
}


void* SoundTest::record_loop(void *arg)
{
    if (arg == NULL) {
        LOG_ERROR("arg is NULL");
        return NULL;
    }
//    FILE * outfile = NULL;
    bool ret = false;
    SndInfo *info = (SndInfo *)arg;
//    int retry_cnt = 80;
    char *buf = NULL;
    int buffer_size = 0;
    snd_pcm_sframes_t recv_len = 0;

    pthread_detach(pthread_self());

    ret = open_sound_card(info);
    if (ret != true) {
        LOG_ERROR("openSoundCardit fail\n");
        return NULL;
    }

    buffer_size = snd_pcm_frames_to_bytes(info->pcm, info->period_size);
    buf = (char *)malloc(buffer_size);
    if (buf == NULL) {
        LOG_ERROR("malloc sound buffer failed\n");
        goto err_record;
    }

    while (gStatus == SOUND_RECORD_START) {
        //LOG_INFO("in the record_loop");
        memset(buf, 0x00, buffer_size);
        recv_len = snd_pcm_readi(info->pcm, buf, info->period_size);
        if (recv_len < 0) {
            LOG_ERROR("pcm readi errno=%s \n", snd_strerror(errno));
            if (recv_len == -EPIPE) {
                snd_pcm_prepare(info->pcm);
            } else {
                snd_pcm_recover(info->pcm, recv_len, 1);
            }
            continue;
        }
        if (recv_len == info->period_size) {
#if 0
            if ((outfile = fopen(SOUND_RECORD_FILE, "a+")) == NULL) {
                LOG_ERROR("can't open %s\n", SOUND_RECORD_FILE);
                break;
            }

            count = fwrite(buf, buffer_size, 1, outfile);
            if (count != 1) {
                LOG_ERROR("write data failed file=%s count=%d size=%d\n", SOUND_RECORD_FILE, count, buffer_size);
                fclose(outfile);
                break;
            }
            fflush(outfile);
            fclose(outfile);
#endif
            ret = write_local_data(SOUND_RECORD_FILE, "a+", buf, buffer_size);
            if (ret == false) {
                LOG_ERROR("write data to %s failed", SOUND_RECORD_FILE);
                break;
            }
        } else {
            LOG_ERROR("read size not period_size:%ld", recv_len);
        }
    }

    LOG_INFO("exit the record_loop");
err_record:
    if (buf != NULL) {
        free(buf);
        buf = NULL;
    }
    close_sound_card(info); // must waiting for while loop stopped

    return NULL;
}

void* SoundTest::playback_loop(void *arg)
{
    if (arg == NULL) {
        LOG_ERROR("arg is NULL");
        return NULL;
    }
    
    bool flag = false;
    int ret = 0;
    SndInfo *info = (SndInfo *)arg;
    FILE *infile = NULL;
    snd_pcm_sframes_t write_frame;
    char *buf = NULL;
    int buffer_size = 0;

    pthread_detach(pthread_self());

    flag = open_sound_card(info);
    if (flag != true) {
        LOG_ERROR("open Sound Cardit fail \n");
        return NULL;
    }

    if ((info->samplearate != g_record_info->samplearate)
            || (info->channels != g_record_info->channels)
            || (info->format != g_record_info->format)) {
        LOG_ERROR("playback params is different from record params! \n");
        goto err_playback;
    }

    buffer_size = snd_pcm_frames_to_bytes(info->pcm, info->period_size);

    buf = (char *)malloc(buffer_size);
    if (buf == NULL) {
        LOG_ERROR("malloc sound buffer failed! \n");
        goto err_playback;
    }

    if ((infile = fopen(SOUND_RECORD_FILE, "r")) == NULL) {
        LOG_ERROR("can't open %s\n", SOUND_RECORD_FILE);
        goto err_playback;
    }

    while (gStatus == SOUND_PLAYBACK_START) {

        //LOG_INFO("in the playback_loop \n");
        memset(buf, 0x00, buffer_size);
        ret = fread(buf, buffer_size, 1, infile);
        if (ret == 0) {
            LOG_INFO("read end of file \n");
            break;
        }

        if (!info->pcm) {
            LOG_ERROR("sound pcm not init! \n");
            break;
        }

        write_frame = snd_pcm_writei(info->pcm, buf, info->period_size);
        if (write_frame < 0) {
            LOG_ERROR("pcm write errno=%s \n", snd_strerror(errno));
            if (write_frame == -EPIPE) {
                snd_pcm_prepare(info->pcm);
            } else {
                snd_pcm_recover(info->pcm, write_frame, 1);
            }
            continue;
        }
    }

err_playback:
    if (infile != NULL) {
        fclose(infile);
        infile = NULL;
    }

    if (buf != NULL) {
        free(buf);
        buf = NULL;
    }
    remove(SOUND_RECORD_FILE);

    return NULL;
}

bool SoundTest::open_sound_card(SndInfo *info)
{
    if (info == NULL) {
        LOG_ERROR("sound info is NULL");
        return false;
    }
    
    int err = -1;
    bool result = false;

    int direction   = info->direction;
    int sample_rate = info->samplearate;
    int channels    = info->channels;
    snd_pcm_format_t format        = info->format;;
    snd_pcm_uframes_t period_size  = info->period_size;
    snd_pcm_hw_params_t *hw_params = NULL;

    LOG_INFO("open audio device %s\n", info->card);

    err = snd_pcm_open(&info->pcm, info->card, info->direction, 0);
    if (err < 0) {
        LOG_ERROR("cannot open audio device %s \n", snd_strerror(err));
        return result;
    }

    err = snd_pcm_hw_params_malloc(&hw_params);
    if (err < 0) {
        LOG_ERROR("cannot allocate hardware parameter structure %s \n", snd_strerror(err));
        goto err;
    }

    err = snd_pcm_hw_params_any(info->pcm, hw_params);
    if (err < 0) {
        LOG_ERROR("cannot initialize hardware parameter structure %s \n", snd_strerror(err));
        goto err;
    }

    err = snd_pcm_hw_params_set_access(info->pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        LOG_ERROR("cannot set access type %s \n", snd_strerror(err));
        goto err;
    }

    err = snd_pcm_hw_params_set_format(info->pcm, hw_params, format);
    if (err < 0) {
        LOG_ERROR("cannot set sample format %s \n", snd_strerror(err));
        goto err;
    }

    err = snd_pcm_hw_params_set_channels(info->pcm, hw_params, channels);
    if (err < 0) {
        LOG_ERROR("cannot set channel count %s \n", snd_strerror(err));
        goto err;
    }

    err = snd_pcm_hw_params_set_rate_near(info->pcm, hw_params, (unsigned int *)&sample_rate, 0);
    if (err < 0) {
        LOG_ERROR("cannot set sample rate %s \n", snd_strerror(err));
        goto err;
    }

    err = snd_pcm_hw_params_set_period_size_near(info->pcm, hw_params, &period_size, &direction);
    if (err < 0) {
        LOG_ERROR("cannot set period size near %s \n", snd_strerror(err));
        goto err;
    }

    if ((err = snd_pcm_hw_params(info->pcm, hw_params)) < 0) {
        LOG_ERROR("cannot set parameters %s \n", snd_strerror(err));
        goto err;
    }

    err = snd_pcm_hw_params_get_period_size(hw_params, &period_size, &direction);
    if (err < 0) {
        LOG_ERROR("get frame size failed %s \n", snd_strerror(err));
        goto err;
    }

    err = snd_pcm_hw_params_get_rate(hw_params, (unsigned int *)&sample_rate, &direction);
    if (err < 0) {
        LOG_ERROR("get rate failed %s \n", snd_strerror(err));
        goto err;
    }

    err = snd_pcm_hw_params_get_channels(hw_params, (unsigned int *)&channels);
    if (err < 0) {
        LOG_ERROR("get channel failed %s \n", snd_strerror(err));
        goto err;
    }

    err = snd_pcm_hw_params_get_format(hw_params, &format);
    if (err < 0) {
        LOG_ERROR("get format failed %s \n", snd_strerror(err));
        goto err;
    }

    info->samplearate = sample_rate;
    info->channels    = channels;
    info->format      = format;
    info->period_size = period_size;

    result = true;

    LOG_INFO("set hardware params success samplarate:%d,channels:%d,format:0x%x,period_size:%ld \n",
         sample_rate, channels, format, period_size);

err:
    if (result == false) {
        snd_pcm_close(info->pcm);
        info->pcm = NULL;
    }

    if (hw_params != NULL) {
        snd_pcm_hw_params_free(hw_params);
        hw_params = NULL;
    }

    return result;
}

void SoundTest::close_sound_card(SndInfo *info)
{
    if (info == NULL) {
        LOG_ERROR("sound info is NULL");
        return;
    }

    if (info->pcm) {
        snd_pcm_close(info->pcm);
        info->pcm = NULL;
    }
    LOG_INFO("close Sound Card \n");
}

bool SoundTest::start_record()
{
    if (!g_record_info || SOUND_RECORD_START == gStatus) {
        LOG_ERROR("it is not ready to record \n");
        return false;
    }
    pthread_t pid_t;

    pthread_mutex_lock(&gMutex);
    gStatus = SOUND_RECORD_START;
    pthread_mutex_unlock(&gMutex);

    LOG_INFO("sound test record start \n");
    int err = pthread_create(&pid_t, NULL, record_loop, g_record_info);
    if (err != 0) {
        LOG_ERROR("record loop create thread error: %s", strerror(err));
        return false;
    }

    return true;
}

bool SoundTest::stop_record()
{
    if (gStatus == SOUND_RECORD_STOP) {
        LOG_ERROR("status is already SOUND_RECORD_STOP");
        return false;
    }

    pthread_mutex_lock(&gMutex);
    gStatus = SOUND_RECORD_STOP;
    pthread_mutex_unlock(&gMutex);
    LOG_INFO("sound test record stop \n");

    return true;
}

bool SoundTest::start_playback()
{
    if (!g_record_info || gStatus == SOUND_PLAYBACK_START) {
        LOG_ERROR("it is not ready to playback \n");
        return false;
    }
    pthread_t pid_t;

    pthread_mutex_lock(&gMutex);
    gStatus = SOUND_PLAYBACK_START;
    pthread_mutex_unlock(&gMutex);

    LOG_INFO("sound test playback start\n");
    int err = pthread_create(&pid_t, NULL, playback_loop, g_playback_info);
    if (err != 0) {
        LOG_ERROR("start playback create thread error: %s", strerror(err));
        return false;
    }

    return true;
}

bool SoundTest::stop_playback()
{
    if (gStatus == SOUND_PLAYBACK_STOP) {
        LOG_ERROR("status is already SOUND_PLAYBACK_STOP");
        return false;
    }

    pthread_mutex_lock(&gMutex);
    gStatus = SOUND_PLAYBACK_STOP;
    pthread_mutex_unlock(&gMutex);
    LOG_INFO("sound test playback stop");

    close_sound_card(g_playback_info); // must waiting for playback ended

    return true;
}

bool SoundTest::init(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is NULL");
        return false;
    }
    if (g_record_info == NULL) {
        LOG_ERROR("new record Info failed \n");
        return false;
    }
    memset(g_record_info, 0, sizeof(SndInfo));
    g_record_info->samplearate  = DEFAULT_SAMPLERATE;
    g_record_info->channels     = DEFAULT_CHANNEL;
    g_record_info->format       = DEFAULT_FORMAT;
    g_record_info->period_size  = RECORD_FRAME_SIZE;
    g_record_info->direction    = RECORD;
    g_record_info->pcm          = NULL;
    g_record_info->card         = DEFAULT_CARD_NAME;

    if (g_playback_info == NULL) {
        LOG_ERROR("new playback Info failed \n");
        return false;
    }
    memset(g_playback_info, 0, sizeof(SndInfo));
    g_playback_info->samplearate    = DEFAULT_SAMPLERATE;
    g_playback_info->channels       = DEFAULT_CHANNEL;
    g_playback_info->format         = DEFAULT_FORMAT;
    g_playback_info->period_size    = PLAYBACK_FRAME_SIZE;
    g_playback_info->direction      = PLAYBACK;
    g_playback_info->pcm            = NULL;
    g_playback_info->card           = DEFAULT_CARD_NAME;
    

    if (baseInfo->platform == "IDV") {
        if (system("if pulseaudio --check; then pulseaudio -k; else touch /tmp/no_pulseaudio; fi") < 0) {
            LOG_ERROR("pulseaudio -k error\n");
            return false;
        }

        if (system("if ! lsmod | grep -q \"snd_hda_intel\"; then modprobe snd_hda_intel; fi") < 0) {
            LOG_ERROR("modprobe snd_hda_intel error\n");
            return false;
        }

    }

    if (pthread_mutex_init(&gMutex, NULL) != 0) {
        return false;
    }
    gStatus = SOUND_UNKNOW;

    init_volume();
    
    return true;
}

void* SoundTest::test_all(void*)
{
    pthread_detach(pthread_self());
    Control *control = Control::get_control();
    UiHandle* uihandle = UiHandle::get_uihandle();
    control->update_color_screen_log("==================== " + FUNC_TEST_NAME[F_SOUND] + " ====================", "black");
    uihandle->start_audio_progress_dialog();
    usleep(200000);  //wait for Synchronize progress bar and recording
    start_record();
    sleep(3);        //recording 3 seconds
    stop_record();
    sleep(1);        //wait 1 seconds to start playback
    start_playback();
    sleep(3);        //playback 3 seconds
    stop_playback();
    sleep(1);        //wait 1 seconds to show result confirm box 
    control->show_test_confirm_dialog(FUNC_TEST_NAME[F_SOUND]);
    
    return NULL;
}

void SoundTest::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return;
    }
    pthread_t tid;
    int err = pthread_create(&tid, NULL, test_all, baseInfo);
    if (err != 0) {
        LOG_ERROR("sound test create thread error: %s", strerror(err));
    }
}

/* open pulseaudio when exiting software */
bool SoundTest::sound_record_restore(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return false;
    }
    if (baseInfo->platform == "IDV") {
        if (system("if [ ! -f /tmp/no_pulseaudio ]; then pulseaudio --start --log-target=syslog; else rm -f /tmp/no_pulseaudio; fi") < 0) {
            LOG_ERROR("pulseaudio --start error\n");
            return false;
        }
    }
    return true;
}

