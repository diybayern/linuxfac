
#include <alsa/asoundlib.h>
#include <pthread.h>

#include "../../inc/SoundTest.h"
#include "../../inc/fac_log.h"
#include "../../inc/fac_utils.h"


#define DEFAULT_SAMPLERATE     (44100)
#define DEFAULT_CHANNEL        (2)
#define DEFAULT_FORMAT         (SND_PCM_FORMAT_S16_LE)

#define PLAYBACK               (SND_PCM_STREAM_PLAYBACK)
#define RECORD                 (SND_PCM_STREAM_CAPTURE)

#define RECORD_FRAME_SIZE      (64)
#define PLAYBACK_FRAME_SIZE    (1024)

#define SOUND_RECORD_FILE      ("/tmp/sound.wav")
#define DEFAULT_CARD_NAME      ("default")

static SndInfo *g_record_info = NULL;
static SndInfo *g_playback_info = NULL;
static SndStatus gStatus =  SOUND_UNKNOW;
static pthread_mutex_t gMutex;
//SoundTest* SoundTest::_mInstance = NULL;

void SoundTest::init_volume()
{
    int ret = 0;
    const char *name = NULL;
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

    for (elem = snd_mixer_first_elem(mixer_fd); elem;
            elem = snd_mixer_elem_next(elem)) {
        name = snd_mixer_selem_get_name(elem);
        LOG_INFO("name=%s \n", name);

        if ((strcmp(name, "Master") == 0) || (strcmp(name, "Headphone") == 0)
            || (strcmp(name, "Speaker") == 0) || (strcmp(name, "PCM") == 0)
            || (strcmp(name, "Mic") == 0)) {
            //set maxvolume
            ret = snd_mixer_selem_get_playback_volume_range(elem, &minVolume, &maxVolume);
            LOG_INFO("name=%s get_playback_volume_range min=%ld max=%ld\n", name, minVolume, maxVolume);
            snd_mixer_selem_set_playback_switch_all(elem, 1);
            if (ret == 0) {
                snd_mixer_selem_set_playback_volume_all(elem, maxVolume);
            } else {
                snd_mixer_selem_set_playback_volume_all(elem, 100);
            }
        } else if (strcmp(name, "Mic Boost") == 0) {
            //set minvolume
            ret = snd_mixer_selem_get_playback_volume_range(elem, &minVolume, &maxVolume);
            LOG_INFO("name=%s get_playback_volume_range min=%ld max=%ld\n", name, minVolume, maxVolume);
            snd_mixer_selem_set_capture_switch_all(elem, 1);
            if (ret == 0) {
                snd_mixer_selem_set_capture_volume_all(elem, minVolume);
            } else {
                snd_mixer_selem_set_capture_volume_all(elem, 0);
            }
        } else if(strcmp(name, "Capture") == 0) {
            ret = snd_mixer_selem_get_capture_volume_range(elem, &minVolume, &maxVolume);
            LOG_INFO("name=%s get_capture_volume_range min=%ld max=%ld\n", name, minVolume, maxVolume);
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
    int count = 0;
//    FILE * outfile = NULL;

    int ret = 0;
    SndInfo *info = (SndInfo *) arg;
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
			count = write_local_data(SOUND_RECORD_FILE, "a+", buf, buffer_size);
			if(count == false)
				break;
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
    close_sound_card(info);

    return NULL;
}

void* SoundTest::playback_loop(void *arg)
{
    int ret = 0;
    SndInfo *info = (SndInfo *)arg;
    FILE *infile = NULL;
    snd_pcm_sframes_t write_frame;
    char *buf = NULL;
    int buffer_size = 0;

    pthread_detach(pthread_self());

    ret = open_sound_card(info);
    if (ret != true) {
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
    close_sound_card(info);
    remove(SOUND_RECORD_FILE);

    return NULL;
}


SoundTest::SoundTest()
{
}

int SoundTest::open_sound_card(SndInfo *info)
{
    int err = -1;
    bool result = false;

    int direction   = info->direction;
    int sample_rate = info->samplearate;
    int channels    = info->channels;
    snd_pcm_format_t format       = info->format;;
    snd_pcm_uframes_t period_size = info->period_size;
    snd_pcm_hw_params_t *hw_params = NULL;

    LOG_INFO("open audio device %s\n", info->card);

    err = snd_pcm_open(&info->pcm, info->card, info->direction, 0);
    if (err < 0) {
        LOG_ERROR("cannot open audio device %s \n", snd_strerror(err));
        return result;
    }

    err = snd_pcm_hw_params_malloc(&hw_params);
    if (err < 0) {
        LOG_ERROR("cannot allocate hardware parameter structure %s \n",
                  snd_strerror(err));
        goto err;
    }

    err = snd_pcm_hw_params_any(info->pcm, hw_params);
    if (err < 0) {
        LOG_ERROR("cannot initialize hardware parameter structure %s \n",
                  snd_strerror(err));
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
    pthread_create(&pid_t, NULL, record_loop, g_record_info);

    return true;
}

bool SoundTest::stop_record()
{
    if (SOUND_RECORD_STOP == gStatus) {
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
    if (!g_record_info || SOUND_PLAYBACK_START == gStatus) {
        LOG_ERROR("it is not ready to playback \n");
        return false;
    }
    pthread_t pid_t;

    pthread_mutex_lock(&gMutex);
    gStatus = SOUND_PLAYBACK_START;
    pthread_mutex_unlock(&gMutex);

    LOG_INFO("sound test playback start\n");
    pthread_create(&pid_t, NULL, playback_loop, g_playback_info);

    return true;
}

bool SoundTest::stop_playback()
{
    if (SOUND_PLAYBACK_STOP == gStatus) {
        return false;
    }

    pthread_mutex_lock(&gMutex);
    gStatus = SOUND_PLAYBACK_STOP;
    pthread_mutex_unlock(&gMutex);
    LOG_INFO("sound test playback stop");

    return true;
}

bool SoundTest::init()
{
    g_record_info = (SndInfo *) malloc(sizeof(SndInfo));
    if (!g_record_info) {
		LOG_ERROR("malloc SndInfo failed \n");
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

    g_playback_info = (SndInfo *) malloc(sizeof(SndInfo));
    if (!g_playback_info) {
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
	

    if (Control::get_control()->get_is_idv()) {
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
	Control *control = Control::get_control();
	UiHandle* uihandle = UiHandle::get_uihandle();
	uihandle->start_audio_progress_dialog();
	usleep(200000);
    start_record();
    sleep(3);
    stop_record();
    sleep(1);
    start_playback();
    sleep(3);
    stop_playback();
    sleep(1);		
	control->confirm_test_result(SOUND_TEST_NAME);
	
	return NULL;
}

void SoundTest::start_test(BaseInfo* baseInfo)
{	
    pthread_t tid;
	init();
    pthread_create(&tid,NULL,test_all,baseInfo);
}


