
#include <stdio.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>

#include "moto_drv.h"

#if defined(MOTO_DBG)
#define MOTO_LOG printf
#else
#define MOTO_LOG
#endif

#define DO_SELF_TEST_CHAR "selftest"
#define DO_HCCW_CHAR "hccw"
#define DO_HCW_CHAR "hcw"
#define DO_VCCW_CHAR "vccw"
#define DO_VCW_CHAR "vcw"
#define DO_HCRUISING_CHAR "hcruising"
#define DO_VCRUISING_CHAR "vcruising"
#define DO_HVCRUISING_CHAR "hvcruising"
#define DO_STOP_CHAR "stop"

#define MAX_CHAR_LEN (16)

typedef void (*do_func)(int fd, int argc, char *argv[]);

struct func_arr {
	char *cmd;
	void *func;
};

void do_self_test(int fd, int argc , char* argv[]);
void do_hccw(int fd, int argc , char* argv[]);
void do_hcw(int fd, int argc , char* argv[]);
void do_vccw(int fd, int argc , char* argv[]);
void do_vcw(int fd, int argc , char* argv[]);
void do_hcruising(int fd, int argc , char* argv[]);
void do_vcruising(int fd, int argc , char* argv[]);
void do_hvcruising(int fd, int argc , char* argv[]);
void do_stop(int fd, int argc , char* argv[]);

struct func_arr func_tab[] = {
	{DO_SELF_TEST_CHAR,do_self_test}
	,{DO_HCCW_CHAR,do_hccw}
	,{DO_HCW_CHAR,do_hcw}
	,{DO_VCCW_CHAR,do_vccw}
	,{DO_VCW_CHAR,do_vcw}
	,{DO_HCRUISING_CHAR,do_hcruising}
	,{DO_VCRUISING_CHAR,do_vcruising}
	,{DO_HVCRUISING_CHAR,do_hvcruising}
	,{DO_STOP_CHAR,do_stop}
	,{NULL,NULL}
};

int main_dbg(int argc , char* argv[])
{
    return(0);
}

void do_self_test(int fd, int argc , char* argv[])
{
    int ret =0;

    ret = ioctl(fd, MTDRV_SELF_TEST);
    /* wait here for completing */
}

void do_hccw(int fd, int argc , char* argv[])
{
    int ret =0;
    unsigned int nsteps=0;

    if(argc < 3) return;
    nsteps = atol(argv[2]);
    MOTO_LOG("%s nsteps=%d\n",__func__,nsteps);
    ret = ioctl(fd, MTDRV_HCCW, &nsteps);
    MOTO_LOG("%s ret=%d, %s\n",__func__,ret, strerror(errno));
    /* wait here for completing */
}

void do_hcw(int fd, int argc , char* argv[])
{
    int ret =0;
    unsigned int nsteps=0;

    if(argc < 3) return;
    nsteps = atol(argv[2]);
    MOTO_LOG("%s nsteps=%d\n",__func__,nsteps);
    ret = ioctl(fd, MTDRV_HCW, &nsteps);
    MOTO_LOG("%s ret=%d, %s\n",__func__,ret, strerror(errno));
    /* wait here for completing */
}
void do_vccw(int fd, int argc , char* argv[])
{
    int ret =0;
    unsigned int nsteps=0;

    if(argc < 3) return;
    nsteps = atol(argv[2]);
    MOTO_LOG("%s nsteps=%d\n",__func__,nsteps);
    ret = ioctl(fd, MTDRV_VCCW, &nsteps);
    MOTO_LOG("%s ret=%d, %s\n",__func__,ret, strerror(errno));
    /* wait here for completing */
}

void do_vcw(int fd, int argc , char* argv[])
{
    int ret =0;
    unsigned int nsteps=0;

    if(argc < 3) return;
    nsteps = atol(argv[2]);
    MOTO_LOG("%s nsteps=%d\n",__func__,nsteps);
    ret = ioctl(fd, MTDRV_VCW, &nsteps);
    MOTO_LOG("%s ret=%d, %s\n",__func__,ret, strerror(errno));
    /* wait here for completing */
}
void do_hcruising(int fd, int argc , char* argv[])
{
    int ret =0;
    unsigned int state=0xFFFFFFFF;

    ret = ioctl(fd, MTDRV_GET_STATE,&state);
    while(MOTO_STATE_IDLE != state)
    {
        ret = ioctl(fd, MTDRV_STOP);
        usleep(100);
        ret = ioctl(fd, MTDRV_GET_STATE,&state);
    }
    ret = ioctl(fd, MTDRV_HCRUISING);
    /* wait here for completing */
}
void do_vcruising(int fd, int argc , char* argv[])
{
    int ret =0;
    unsigned int state=0xFFFFFFFF;

    ret = ioctl(fd, MTDRV_GET_STATE,&state);
    while(MOTO_STATE_IDLE != state)
    {
        ret = ioctl(fd, MTDRV_STOP);
        usleep(100);
        ret = ioctl(fd, MTDRV_GET_STATE,&state);
    }
    ret = ioctl(fd, MTDRV_VCRUISING);
    /* wait here for completing */
}
void do_hvcruising(int fd, int argc , char* argv[])
{
    int ret =0;
    unsigned int state=0xFFFFFFFF;

    ret = ioctl(fd, MTDRV_GET_STATE,&state);
    while(MOTO_STATE_IDLE != state)
    {
        ret = ioctl(fd, MTDRV_STOP);
        usleep(100);
        ret = ioctl(fd, MTDRV_GET_STATE,&state);
    }
    ret = ioctl(fd, MTDRV_HVCRUISING);
    /* wait here for completing */
}
void do_stop(int fd, int argc , char* argv[])
{
    int ret =0;

    ret = ioctl(fd, MTDRV_STOP);
    /* wait here for completing */
}
void *get_func(char *cmd)
{
	struct func_arr *fa = &func_tab[0];

	while(fa->cmd)
	{
		if(strcmp(cmd,fa->cmd) == 0)
		{
			MOTO_LOG("%s OK!!!\n",__func__);
			break;
		}
		fa++;
	}
	
	return(fa->func);
}

int main_body(int argc , char* argv[])
{
	int fd = -1;
	do_func func = NULL;

	if(argc < 2) return -1;
	
	fd = open("/dev/" MOTO_DEVICE_NAME, 0);
    if(fd<0)
    {
    	printf("Open "MOTO_DEVICE_NAME" error!\n");
    	return -1;
    }
    MOTO_LOG("%s open ok\n",__func__);
	func = (do_func)get_func(argv[1]);
    MOTO_LOG("%s get func ok\n",__func__);
	if(!func) return -1;
    MOTO_LOG("%s exec func ok\n",__func__);
	func(fd, argc, argv);
	
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
