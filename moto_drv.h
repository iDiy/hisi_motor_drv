
#ifndef _MOTO_DRV_H
#define _MOTO_DRV_H
#include <linux/ioctl.h>
#include <linux/types.h>
//#define MOTO_DBG
#define MOTO_DEVICE_NAME "moto_drv"
#define MOTO_IOCTL_BASE (0x11)

enum {
    MTDRV_NONE = _IO(MOTO_IOCTL_BASE,0)
    ,MTDRV_SELF_TEST = _IO(MOTO_IOCTL_BASE,1)
    ,MTDRV_HCCW = _IOW(MOTO_IOCTL_BASE,2,unsigned int)
    ,MTDRV_HCW = _IOW(MOTO_IOCTL_BASE,3,unsigned int)
    ,MTDRV_VCCW = _IOW(MOTO_IOCTL_BASE,4,unsigned int)
    ,MTDRV_VCW = _IOW(MOTO_IOCTL_BASE,5,unsigned int)
    ,MTDRV_HCRUISING = _IO(MOTO_IOCTL_BASE,6)
    ,MTDRV_GET_STATE = _IOR(MOTO_IOCTL_BASE,7,unsigned int)
    ,MTDRV_STOP = _IO(MOTO_IOCTL_BASE,8)
};

enum {
    MOTO_IND_NONE
    ,MOTO_IND_L
    ,MOTO_IND_R
    ,MOTO_IND_U
    ,MOTO_IND_D
    ,MOTO_IND_INVALID = 0xFFFFFFFF
};

enum {
    MOTO_RSP_NONE
    ,MOTO_RSP_DONE
    ,MOTO_RSP_MAX
    ,MOTO_RSP_INVALID = 0xFFFFFFFF
};

enum {
	MOTO_HDEV = 0
	,MOTO_VDEV = 1
	,MOTO_NUM = 2
};

enum {
	MOTO_CW = 0
	,MOTO_CCW = 1
	,MOTO_ORDER_NUM = 2
};

enum {
	MOTO_SPD_LV1 = 0
	,MOTO_SPD_LV2
	,MOTO_SPD_LV3
	,MOTO_SPD_LV4
	,MOTO_SPD_LV5
	,MOTO_SPD_LV_MAX
};

enum {
	 MOTO_STATE_IDLE= 0
	,MOTO_STATE_SELF_TEST
	,MOTO_STATE_PRESETTING
	,MOTO_STATE_TRIMMING
	,MOTO_STATE_CRUISING
	,MOTO_STATE_CONTROLLING
};

struct moto_drv_action {
	void *params;
};

typedef unsigned char		byte;

#endif

