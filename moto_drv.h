
#ifndef _MOTO_DRV_H
#define _MOTO_DRV_H

//#define MOTO_DBG
#define MOTO_DEVICE_NAME "moto_drv"

enum {
    MOTO_DRV_NONE
    ,MOTO_DRV_SELF_TEST=0x80
    ,MOTO_DRV_HCCW
    ,MOTO_DRV_HCW
    ,MOTO_DRV_VCCW
    ,MOTO_DRV_VCW
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

