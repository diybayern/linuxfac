#include "../../inc/BrightTest.h"
#include "../../inc/fac_log.h"
#include "../../inc/fac_utils.h"

#include <sys/inotify.h>

static int inotify_fd;
static int wd;
const int BRIGHTNESS_VALUE[6] = 
{
    7,17,27,37,47,57,
};

BrightTest::BrightTest()
{
}

inline int BrightTest::brightness_is_set(const int* const array, int array_cout, int value)
{
    int i = 0;
    for(i = 0; i < array_cout; i ++)
    {
        if(value == *(array + i))
        {
            return i;
        }
    }
    return -1;
}

void BrightTest::bright_test_all(string bright_level)
{
	Control* control = Control::get_control();
	int bright_num = get_int_value(bright_level);
    int actual_brightness_fd = 0;
    inotify_fd = inotify_init();
    char buf[4096];
    wd = inotify_add_watch(inotify_fd, "/sys/class/backlight/intel_backlight/actual_brightness", IN_MODIFY);
    int bright_cnt= 0;
    int bright_set_mask = 0;
    int bright_value = 0;
    int bright_set = 0;
    int ret = 0;
	
    pthread_detach(pthread_self());

    
    for(bright_cnt = 0; bright_cnt < bright_num; bright_cnt++)
    {
        bright_set_mask |= 1<<bright_cnt;
    }
    bright_cnt = 0;
    LOG_INFO("begin inotify brightness trigger\n");
	control->update_screen_log("begin inotify brightness trigger\n");
	bright_set = 0;
    for(bright_cnt = 0; bright_cnt < bright_num; bright_cnt++)
    {
        bright_value = 0;
        ret = read(inotify_fd, buf, 64);
        if(ret <= 0)
        {
            LOG_ERROR("inotify read error\n");
            goto error_return;
        }
        memset(buf, 0, 4096);
        if((actual_brightness_fd = open("/sys/class/backlight/intel_backlight/actual_brightness", O_RDONLY)) <= 0)
        {
            LOG_ERROR("actual_brightness_fd open error\n");
            goto error_return;
        }
        if (read(actual_brightness_fd, buf, 4096) < 0) {
            LOG_ERROR("actual_brightness_fd read error\n");
        }
        close(actual_brightness_fd);
        bright_value = atoi(buf);
        ret = brightness_is_set(BRIGHTNESS_VALUE, bright_num, bright_value);
        if(ret != -1)
        {
        	bright_set |= 1<<ret;
            LOG_INFO("PRESS %d:now the brightness is %d, brightness level %d\n", bright_cnt+1, bright_value, ret+1);
			control->update_screen_log("PRESS " + to_string(bright_cnt+1) + ":now the brightness is "
					+ to_string(bright_value) + ", brightness level " + to_string(ret+1));
        } else {
            LOG_ERROR("PRESS %d: brightness value is not set, brightness is %d\n",bright_cnt+1, bright_value);
			control->update_screen_log("PRESS " + to_string(bright_cnt+1) + ": brightness value is not set, brightness is "
					+ to_string(bright_value) + "\n");
            goto error_return;
        }
	}
    bright_set &= bright_set_mask;
    if(bright_set != bright_set_mask)
    {
        LOG_ERROR("all the brightness value cannot be corvered within 6 presses\n");
		control->update_screen_log("all the brightness value cannot be corvered within 6 presses\n");
        goto error_return;
    }
error_return:

    inotify_rm_watch (inotify_fd, wd);
    close(inotify_fd);
    
    return;
}

void* BrightTest::test_all(void *arg)
{
	Control::get_control()->update_screen_log("==================== bright test ====================\n");
	BaseInfo* baseInfo = (BaseInfo *)arg;
    Control::get_control()->confirm_test_result(BRIGHT_TEST_NAME);
	bright_test_all(baseInfo->bright_level);
	return NULL;
}

void BrightTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}


