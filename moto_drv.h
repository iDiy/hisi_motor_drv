
#ifndef _MOTO_DRV_H
#define _MOTO_DRV_H

#define MOTO_DBG

enum {
    MOTO_DRV_NONE
    ,MOTO_DRV_DIR_CTL
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

struct moto_drv_action {
	unsigned int ind;
	unsigned int rsp;
};

typedef unsigned char		byte;

unsigned char moto_drv_read(unsigned char devaddress, unsigned char address);
void moto_drv_write(unsigned char devaddress, unsigned char address, unsigned char value);
byte siiReadSegmentBlockEDID(byte SlaveAddr, byte Segment, byte Offset, byte *Buffer, byte Length);


#endif

