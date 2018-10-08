#include "../../inc/FuncTest.h"
#include "../../inc/fac_log.h"

string usb_screen_log = "";

UsbTest::UsbTest()
{
    
}

bool UsbTest::usb_num_test(string total_num, string num_3)
{	
    string real_total_num = execute_command("lsusb -t | grep \"Mass Storage\" | wc -l");
    string real_num_3 = execute_command("lsusb -t | grep \"Mass Storage\" | grep \"5000M\" | wc -l");
    if (real_total_num == total_num) {
        if (real_num_3 == num_3) {
            return true;
        } else {
            usb_screen_log += "usb3.0 num is " + real_num_3 + ",which need " + num_3 + "\n";
            LOG_INFO("usb3.0 num is %s,which need %s!",real_num_3.c_str(),num_3.c_str());
            return false;
        }
    } else {
    	usb_screen_log += "usb num is " + real_total_num + ",which need " + total_num + "\n";
        LOG_INFO("usb num is %s,which need %s!",real_total_num.c_str(),total_num.c_str());
        return false;
    }
}

bool UsbTest::get_dev_mount_point(struct udev_device* dev, char* dst) {
	int len = 0;
	DIR* dir = NULL;
	struct dirent *ptr = NULL;
	const char* sys_path = NULL;
	const char* sys_name = NULL;

	sys_path = udev_device_get_syspath(dev);
	sys_name = udev_device_get_sysname(dev);

	dir = opendir(sys_path);
	if (NULL == dir) {
		LOG_INFO("open dir=%s\n", sys_path);
		return false;
	}

	len = strlen(sys_name);

	while (NULL != (ptr = readdir(dir))) {

		if (0 == strncmp(ptr->d_name, sys_name, len)) {
			snprintf(dst, USB_BLOCK_LEN, "/dev/%s", ptr->d_name);
			break;
		}
	}

	closedir(dir);

	return true;
}

struct udev_device*
UsbTest::get_child(struct udev* udev, struct udev_device* parent, const char* subsystem) {

	struct udev_device* child = NULL;
	struct udev_list_entry* entry = NULL;
	struct udev_list_entry *devices = NULL;

	struct udev_enumerate* enumerate = udev_enumerate_new(udev);

	udev_enumerate_add_match_parent(enumerate, parent);
	udev_enumerate_add_match_subsystem(enumerate, subsystem);
	udev_enumerate_scan_devices(enumerate);

	devices = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry_foreach(entry, devices)
	{
		const char *path = udev_list_entry_get_name(entry);
		child = udev_device_new_from_syspath(udev, path);
		break;
	}

	udev_enumerate_unref(enumerate);
	return child;
}

void UsbTest::get_usb_mass_storage(USB_INFO_T* info) {

	int ret = false;
	int index = 0;
	struct udev* udev = NULL;
	struct udev_list_entry *entry = NULL;
	struct udev_list_entry *devices = NULL;

	udev = info->udev;
	struct udev_enumerate* enumerate = udev_enumerate_new(udev);

	udev_enumerate_add_match_subsystem(enumerate, "scsi");
	udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");
	udev_enumerate_scan_devices(enumerate);

	devices = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry_foreach(entry, devices)
	{
		const char* path = udev_list_entry_get_name(entry);
		struct udev_device* scsi = udev_device_new_from_syspath(udev, path);

		struct udev_device* block = get_child(udev, scsi, "block");
		struct udev_device* scsi_disk = get_child(udev, scsi, "scsi_disk");

		struct udev_device* usb = udev_device_get_parent_with_subsystem_devtype(
				scsi, "usb", "usb_device");

		if (block && scsi_disk && usb) {
			ret = get_dev_mount_point(block, info->dev[index].block);
			if (ret == false) {
				continue;
			}
			strncpy(info->dev[index].vendor,
					udev_device_get_sysattr_value(scsi, "vendor"),
					USB_VENDOR_LEN);

			index++;
		}

		if (block) {
			udev_device_unref(block);
		}

		if (scsi_disk) {
			udev_device_unref(scsi_disk);
		}

		udev_device_unref(scsi);
	}

	info->dev_num = index;
	udev_enumerate_unref(enumerate);
}

bool UsbTest::usb_test_mount(char* block, const char* dir) {
	char cmd[64] = { 0, };

	if (NULL == block || NULL == dir) {
		LOG_INFO("mount dir=%s to block=%s failed\n", dir, block);
		return false;
	}

	sprintf(cmd, "mount %s %s", block, dir);
	if (system(cmd) < 0) {
		LOG_INFO("run %s failed\n", cmd);
		return false;
	}

	return true;
}

bool UsbTest::usb_test_write(const char* dir, const char* file_name) {

	int i = 0;
	bool ret = false;
	char name[128] = { 0, };
	int buf[USB_WRITE_LEN] = { 0, };

	for (i = 0; i < USB_WRITE_LEN; i++) {
		buf[i] = i;
	}

	sprintf(name, "%s/%s", dir, file_name);
	ret = write_local_data(name, "w+", (char*) buf,
			USB_WRITE_LEN * sizeof(int));
	if (ret == false) {
		LOG_INFO("write data to usb failed\n");
	}

	return ret;
}

bool UsbTest::usb_test_read(const char* dir, const char* file_name) {

	int i = 0;
	bool ret = false;
	char name[128] = { 0, };
	int buf[USB_WRITE_LEN] = { 0, };

	sprintf(name, "%s/%s", dir, file_name);
	ret = read_local_data(name, (char*) buf, USB_WRITE_LEN * sizeof(int));
	if (ret == false) {
		LOG_INFO("read data from usb failed\n");
		return false;
	}

	for (i = 0; i < USB_WRITE_LEN; i++) {
		if (buf[i] != i) {
			ret = false;
			LOG_INFO("read data failed\n");
			break;
		}

	}

	(void) remove(name);

	return ret;
}

bool UsbTest::usb_test_umount(const char* dir) {
	int ret = false;
	char cmd[64] = { 0, };

	if (NULL == dir) {
		LOG_INFO("unmount dir=%s failed\n", dir);
		return false;
	}
	sprintf(cmd, "umount %s", dir);
	ret = system(cmd);
	if (ret < 0) {
		LOG_INFO("run %s failed\n", cmd);
		return false;
	}

	return true;
}

bool UsbTest::usb_test_write_read(USB_INFO_T* info) {

	int i = 0;
	string path = "/mnt/usb_factory_test";
	string file_name = "usbbbbbb_test";

    (void) mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    
	for (i = 0; i < info->dev_num; i++) {
        if (!usb_test_mount(info->dev[i].block, path.c_str())) {
            return false;
        }
        if (!usb_test_write(path.c_str(), file_name.c_str())) {
            return false;
        }
        if (!usb_test_read(path.c_str(), file_name.c_str())) {
            return false;
        }
        if (!usb_test_umount(path.c_str())) {
            return false;
        }
		usleep(10000);
	}

	(void) remove(path.c_str());

	return true;
}

bool UsbTest::usb_test_all(int num) {
	bool ret = false;
	USB_INFO_T info;
	memset(&info, 0, sizeof(USB_INFO_T));

	info.udev = udev_new();
	if (NULL == info.udev) {
		LOG_INFO("new udev failed\n");
		return false;
	}
	get_usb_mass_storage(&info);
	
    if (num == info.dev_num) {
        ret = usb_test_write_read(&info);
    } else {
        return false;
    }
    return ret;
}


void* UsbTest::test_all(void *arg)
{
    Control *control = Control::get_control();
	control->set_interface_test_status(USB_TEST_NAME, false);
	BaseInfo* baseInfo = (BaseInfo *)arg;
	usb_screen_log += "==================== usb test ====================\n";
    int num = get_int_value(baseInfo->usb_total_num);
	bool result_num_test = false;
    result_num_test = usb_num_test(baseInfo->usb_total_num,baseInfo->usb_3_num);
    
    if (result_num_test) {
        bool result_write_read = usb_test_all(num);
        if (result_write_read) {
			usb_screen_log += "usb test result:\t\t\tSUCCESS\n\n";
	   		control->set_interface_test_result(USB_TEST_NAME, true); 
        } else {
        	usb_screen_log += "usb test result:\t\t\tFAIL\n\n";
			control->set_interface_test_result(USB_TEST_NAME, false);
        }
    } else {
		usb_screen_log += "usb test result:\t\t\tFAIL\n\n";
		control->set_interface_test_result(USB_TEST_NAME, false); 
    }	
	control->update_screen_log(usb_screen_log);
	control->set_interface_test_status(USB_TEST_NAME, true);
	usb_screen_log = "";
	return NULL;
}

void UsbTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}


bool UsbTest::usb_test_read_cfg(const char* dir)
{
    char name[128] = {0};
    char cmd[256] = {0};

    sprintf(name, "%s/fac_config.conf", dir);

    if (check_file_exit(name)) {
        LOG_INFO("find fac config conf!\n");
    } else {
    	LOG_ERROR("not find fac config conf!\n");
        return false;
    }

    sprintf(cmd, "sudo cp -r %s %s", name, FAC_CONFIG_FILE.c_str());
    if (execute_command(cmd) == "error") {
        LOG_ERROR("system cmd %s failed!", cmd);
        return false;
    }

    return true;
}

bool UsbTest::usb_test_read_cfg(USB_INFO_T* info) {

	int i = 0;
	bool ret = false;
	string path = "/mnt/usb_factory_test";

    (void) mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	for (i = 0; i < info->dev_num; i++) {

		usb_test_mount(info->dev[i].block, path.c_str());
        ret = usb_test_read_cfg(path.c_str());
		usb_test_umount(path.c_str());

        if (ret) {
            break;
        }
		usleep(10000);
	}

	return ret;
}

bool UsbTest::usb_test_read_status(){

    bool ret = false;
	USB_INFO_T info;
	memset(&info, 0, sizeof(USB_INFO_T));

	info.udev = udev_new();
	if (NULL == info.udev) {
		LOG_ERROR("new udev failed\n");
		return false;
	}
    
    get_usb_mass_storage(&info);

    if (0 != info.dev_num) {
		ret = usb_test_read_cfg(&info);
	} else {
		ret = false;
	}

	udev_unref(info.udev);

    return ret;
}

