
#include <stdio.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "moto_drv.h"

#if defined(MOTO_DBG)
#define MOTO_LOG printf
#else
#define MOTO_LOG
#endif

#define DOIT_CHAR_L "l"
#define DOIT_CHAR_R "r"
#define DOIT_CHAR_U "u"
#define DOIT_CHAR_D "d"

#define MAX_CHAR_LEN (16)

typedef void (*func_doit)(int , void *);

int main_dbg(int argc , char* argv[])
{
    return(0);
}

void do_self_test(int fd, void *params)
{
    int ret =0;
    ret = ioctl(fd, MOTO_DRV_SELF_TEST, NULL);
    /* wait here for self test completing */
}

int main_body(int argc , char* argv[])
{
	int fd = -1;
	fd = open("/dev/" MOTO_DEVICE_NAME, 0);
    if(fd<0)
    {
    	printf("Open gpioi2c error!\n");
    	return -1;
    }
	do_self_test(fd, NULL);

	close(fd);
	return 0;
}

int main(int argc , char* argv[])
{
#if 0//defined(MOTO_DBG)
	return main_dbg(argc, argv);
#else
	return main_body(argc, argv);
#endif
}
