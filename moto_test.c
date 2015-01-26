
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

void moto_doit_l(int fhdl, void *arg);
void moto_doit_r(int fhdl, void *arg);
void moto_doit_u(int fhdl, void *arg);
void moto_doit_d(int fhdl, void *arg);

struct moto_action {
    char *doitchar;
    void *doitfunc;
    void *funcargs;
} moto_doit[] = {
    	{DOIT_CHAR_L,moto_doit_l,NULL}
    	,{DOIT_CHAR_R,moto_doit_r,NULL}
    	,{DOIT_CHAR_U,moto_doit_u,NULL}
    	,{DOIT_CHAR_D,moto_doit_d,NULL}
    	,{NULL,NULL,NULL}
};
#define DOIT_SIZES (sizeof(moto_doit)/sizeof(moto_doit[0]))

void *get_moto_doit(char *doitchar)
{
    struct moto_action *pma = &moto_doit[0];

    while(NULL != pma->doitchar)
    {
        if(strcmp(doitchar,pma->doitchar) == 0) return pma;
        pma++;
    }
    return NULL;
}

void moto_doit_l(int fhdl, void *arg)
{
    int i;
    int ret =0;
    struct moto_drv_action mda;
    MOTO_LOG("%s entry\n",__func__);
    if(fhdl<0)
    {
    	printf("%s failed!\n",__func__);
    	return;
    }
    mda.ind = MOTO_IND_L;
    mda.rsp = MOTO_RSP_INVALID;
    for(i=0; i<648/8; i++)
    {
    ret = ioctl(fhdl, MOTO_DRV_DIR_CTL, &mda);
    if(0 == ret)
    {
        MOTO_LOG("%s MOTO_DRV_DIR_CTL OK rsp=%d\n",__func__,mda.rsp);
    }
    else
    {
        MOTO_LOG("%s MOTO_DRV_DIR_CTL failed\n",__func__);
    }
    }
    MOTO_LOG("%s exit\n",__func__);
}

void moto_doit_r(int fhdl, void *arg)
{
    int i;
    int ret =0;
    struct moto_drv_action mda;
    MOTO_LOG("%s entry\n",__func__);
    if(fhdl<0)
    {
    	printf("%s failed!\n",__func__);
    	return;
    }
    mda.ind = MOTO_IND_R;
    mda.rsp = MOTO_RSP_INVALID;
    for(i=0; i<648/8; i++)
    {
    ret = ioctl(fhdl, MOTO_DRV_DIR_CTL, &mda);
    if(0 == ret)
    {
        MOTO_LOG("%s MOTO_DRV_DIR_CTL OK rsp=%d\n",__func__,mda.rsp);
    }
    else
    {
        MOTO_LOG("%s MOTO_DRV_DIR_CTL failed\n",__func__);
    }
    }
    MOTO_LOG("%s exit\n",__func__);
}
void moto_doit_u(int fhdl, void *arg)
{
    int i;
    int ret =0;
    struct moto_drv_action mda;
    MOTO_LOG("%s entry\n",__func__);
    if(fhdl<0)
    {
    	printf("%s failed!\n",__func__);
    	return;
    }
    mda.ind = MOTO_IND_U;
    mda.rsp = MOTO_RSP_INVALID;
    for(i=0; i<64; i++)
    {
    ret = ioctl(fhdl, MOTO_DRV_DIR_CTL, &mda);
    if(0 == ret)
    {
        MOTO_LOG("%s MOTO_DRV_DIR_CTL OK rsp=%d\n",__func__,mda.rsp);
    }
    else
    {
        MOTO_LOG("%s MOTO_DRV_DIR_CTL failed\n",__func__);
    }
    }
    MOTO_LOG("%s exit\n",__func__);
}
void moto_doit_d(int fhdl, void *arg)
{
    int i;
    int ret =0;
    struct moto_drv_action mda;
    MOTO_LOG("%s entry\n",__func__);
    if(fhdl<0)
    {
    	printf("%s failed!\n",__func__);
    	return;
    }
    mda.ind = MOTO_IND_D;
    mda.rsp = MOTO_RSP_INVALID;
    for(i=0; i<64; i++)
    {
    ret = ioctl(fhdl, MOTO_DRV_DIR_CTL, &mda);
    if(0 == ret)
    {
        MOTO_LOG("%s MOTO_DRV_DIR_CTL OK rsp=%d\n",__func__,mda.rsp);
    }
    else
    {
        MOTO_LOG("%s MOTO_DRV_DIR_CTL failed\n",__func__);
    }
    }
    MOTO_LOG("%s exit\n",__func__);
}

int main_dbg(int argc , char* argv[])
{
	int fd = -1;
	int ret =0;
	unsigned int device_addr, reg_addr, reg_value, value;
	char carr[MAX_CHAR_LEN+1] = {0};
	struct moto_drv_action mda;
	struct moto_action *pma;
		
	fd = open("/dev/moto_drv", 0);
    if(fd<0)
    {
    	printf("Open moto_drv error!\n");
    	return -1;
    }
	value = MOTO_IND_NONE;
	mda.ind = MOTO_IND_NONE;
	mda.rsp = MOTO_RSP_INVALID;
    ret = ioctl(fd, MOTO_DRV_NONE, &mda);
    while(1)
    {
	memset(carr,0,MAX_CHAR_LEN+1);
       scanf("%s", carr);
	if(strcmp("q",carr) == 0) break;
		
	pma = (struct moto_action *)get_moto_doit(carr);
	if((pma) && (pma->doitfunc))
	{
	    func_doit func = (func_doit)pma->doitfunc;
	    func(fd,NULL);
	}
    }
    close(fd);
    return(0);
}
int main_nodbg(int argc , char* argv[])
{
    return 0;
}

int main(int argc , char* argv[])
{
#if defined(MOTO_DBG)
	return main_dbg(argc, argv);
#else
	return main_nodbg(argc, argv);
#endif
}
