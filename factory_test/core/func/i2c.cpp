/* The Great I2C Getter.
 *
 * (C)opyright 2008-2014 Matthew Kern
 * Full license terms in file LICENSE
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "../../inc/i2c_dev.h" //use ours 'cuz it's betterer.
#include "../../inc/fac_utils.h"
#include "../../inc/fac_log.h"


#define EDID_INFO_FILE  "/var/tmp/edid.info"
#define I2C_CLASS_PATH  "/sys/class/i2c-dev"

#define I2C_DEV_PATH_LEN    256
#define I2C_DEV_NAME_LEN    256

static int open_i2c_dev(int i2cbus)
{
	int i2cfile;
	char filename[16];
	unsigned long funcs;

	sprintf(filename, "/dev/i2c-%d", i2cbus);
	i2cfile = open(filename, O_RDWR);

	if (i2cfile < 0 && errno == ENOENT) {
		filename[8] = '/';
		i2cfile = open(filename, O_RDWR);
	}

#if 0
	if (errno == EACCES) {
		display("Permission denied opening i2c. Run as root!\n");
		i2cfile = -2;
	}
#endif
	
	if (i2cfile >= 0) {
		if (ioctl(i2cfile, I2C_FUNCS, &funcs) < 0) {
            LOG_ERROR("ioctl I2C_FUNCS: %s\n", strerror(errno));
			i2cfile = -3;
		}

		if (!(funcs & (I2C_FUNC_SMBUS_READ_BYTE_DATA))) {
			LOG_ERROR("No byte reading on this bus...\n");
			i2cfile = -4;
		}

		if (ioctl(i2cfile, I2C_SLAVE, 0x50) < 0) {
            LOG_ERROR("Problem requesting slave address: %s\n", strerror(errno));
			i2cfile = -5;
		}
	}

	return i2cfile;
}

int parse_edid_from_file(const char *edid_info);
static int parse_i2c_edid(char *block, int len)
{
    FILE *fp;
    int i;

    unlink(EDID_INFO_FILE);
    fp = fopen(EDID_INFO_FILE, "w");
    if (fp == NULL) {
        LOG_ERROR("Failed to create %s to write!\n", EDID_INFO_FILE);
        return FAIL;
    }
    for (i = 0; i < len; i++)
        fprintf(fp, "%c", block[i]);
    fflush(fp);
    fclose(fp);

    return parse_edid_from_file(EDID_INFO_FILE);
}

static char *edid_i2c_dev_name(int i2cbus)
{
    char i2c_devname_syspath[I2C_DEV_PATH_LEN];
    FILE *fp;
    char *p;

    static char edid_i2c_devname[I2C_DEV_NAME_LEN];
    memset(edid_i2c_devname, '\0', I2C_DEV_NAME_LEN);

    snprintf(i2c_devname_syspath, I2C_DEV_PATH_LEN, "%s/i2c-%d/name", I2C_CLASS_PATH, i2cbus);
    fp = fopen(i2c_devname_syspath, "r");
    if (fp == NULL) {
        LOG_ERROR("Failed to open EDID I2C devname file %s\n", i2c_devname_syspath);
        p = edid_i2c_devname;
        return p;
    }
    p = fgets(edid_i2c_devname, I2C_DEV_NAME_LEN, fp);
    if (p == NULL) {
        LOG_ERROR("Error occur while reading EDID I2C devname file %s\n", i2c_devname_syspath);
        p = edid_i2c_devname;
        fclose(fp);
        return p;
    }
    fclose(fp);
    return p;
}

int edid_read_i2c_test(int edid_num)
{
	int i, j, ret, len, numbusses=0, i2cfile, i2cbus=0;
	int goodbus[128];
	unsigned char block[256];

	for (i2cfile = open_i2c_dev(i2cbus); i2cfile >= 0 || i2cfile < -3;) {
		//read a byte. This is the official way to scan
		if (i2cfile < -3) //problem with a bus, not general enough. Skip and close.
			goto endloop;

		ret = i2c_smbus_read_byte_data(i2cfile, 0);
		if (ret < 0) {
			LOG_ERROR("No EDID on bus %i\n", i2cbus);
		} else {
			goodbus[numbusses] = i2cbus;
			numbusses++;
		}

endloop:
		close(i2cfile);
		i2cbus++;
		i2cfile = open_i2c_dev(i2cbus);
	}
#if 0
	if (i2cfile == -2) {
		return 1;
	}
#endif

	if (numbusses == 0) {
		LOG_ERROR("Looks like no buses have an EDID. Sorry!\n");
		return AGAIN;
	}
#if 0
	SCREEN_SIMPLE("\t%i potential buses found: [", numbusses);
	for (i=0; i<numbusses; i++)
		SCREEN_SIMPLE(" %i ", goodbus[i]);
	SCREEN_SIMPLE("]\n\n");
#endif

    ret = 0;
	for (i=0; i < numbusses; i++) {
		i2cbus = goodbus[i];
		i2cfile = open_i2c_dev(i2cbus);
		if (i2cfile >= 0) {
            //no matter how many times, >=0 still looks really angry.
			for (j=0; j<256; j++)
				block[j] = i2c_smbus_read_byte_data(i2cfile, j);
		}
		close(i2cfile);
		if (block[0]==0x00 && block[7]==0x00
            && block[1]==0xff && block[2]==0xff
            && block[3]==0xff && block[4]==0xff
            && block[5]==0xff && block[6]==0xff) {
            if (block[128] == 0xff)
                len = 128;
            else
                len = 256;
            LOG_INFO("%i-byte EDID successfully retrieved from i2c bus [%i]:\n", len, i2cbus);
            LOG_INFO("\tEDID I2C device name: %s\n", edid_i2c_dev_name(i2cbus));
            if (parse_i2c_edid((char *)block, len) == FAIL) {
                ret = FAIL;
                LOG_ERROR("ERROR: Failed to parse the EDID information from i2c bus %i!\n",
                    i2cbus);
            }
		} else {
			LOG_ERROR("ERROR: Bus %i doesn't really have an EDID...\n", i2cbus);
            ret = FAIL;
		}
	}

    if (numbusses < edid_num) {
        /* Detected bus less than the real num, FAIL */
        LOG_ERROR("ERROR: This product contains %d DDC interfaces, but only %d detected.\n",
            edid_num, numbusses);
        ret = FAIL;
    }
    return ret;

#if 0
	if (ret==0) {
		if (block[128] == 0xff)
			len = 128;
		else
			len = 256;
		display("%i-byte EDID successfully retrieved from i2c bus %i\n", len, i2cbus);
		if (i2cbus < (numbusses-1))
			display("If this isn't the EDID you were looking for, consider the other potential busses.\n");
		for (i=0;i<len;i++) {
			printf("%c", block[i]);
		}
	}
	else {
		display("Couldn't find an accessible EDID on this %s.\n", (tryonly==-1)?"computer":"bus");
		return 3;
	}
	return 0;
#endif
}

