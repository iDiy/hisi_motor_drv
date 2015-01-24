
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/fcntl.h>

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>

#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/io.h>

#include "moto_drv.h" 

/*
 *http://wenku.baidu.com/link?url=s5q1LhFbmOVIjVmPk2GwUbDTuyMnvLKrYmlqvDQFpNGbr2cYEfXwaWAHTgPe9AjJHx2Jcdi8nRY2SfY3p9mCwZaGvQIsJ5A1LwDqMeYjqEG&qq-pf-to=pcqq.discussion
*/
#if defined(MOTO_DBG)
#define MOTO_DRV_LOG printk
#else
#define MOTO_DRV_LOG
#endif


#define GPIO_5_BASE (0x20190000)
#define GPIO_9_BASE (0x201D0000)

#define GPIO_5_DIR IO_ADDRESS(GPIO_5_BASE + 0x400)
#define GPIO_9_DIR IO_ADDRESS(GPIO_9_BASE + 0x400)

#define ALL_MOTO_DIR_ADDR IO_ADDRESS(GPIO_5_BASE + 0x3C0)

#define GET_L_POSTION HW_REG(IO_ADDRESS(GPIO_5_BASE + 0x100))
#define GET_R_POSTION HW_REG(IO_ADDRESS(GPIO_5_BASE + 0x200))
#define GET_U_POSTION HW_REG(IO_ADDRESS(GPIO_5_BASE + 0x040))
#define GET_D_POSTION HW_REG(IO_ADDRESS(GPIO_5_BASE + 0x080))

#define HW_REG(reg)         *((volatile unsigned int *)(reg))
#define DELAY(us)           time_delay_us(us)

struct moto_step_drv {
    unsigned int regoff;
    unsigned char portval;
    int delay;/* ms */
};
#define ONE_STEP_PULSE (8)

#if (8 == ONE_STEP_PULSE)
struct moto_step_drv h_ccwtab[ONE_STEP_PULSE] = {
    	{0x03C,0x01,0x1}
    	,{0x03C,0x03,0x1}
    	,{0x03C,0x02,0x1}
	,{0x03C,0x06,0x1}
    	,{0x03C,0x04,0x1}
	,{0x03C,0x0C,0x1}
    	,{0x03C,0x08,0x1}
	,{0x03C,0x09,0x1}
};

struct moto_step_drv h_cwtab[ONE_STEP_PULSE] = {
    	{0x03C,0x01,0x1}
    	,{0x03C,0x09,0x1}
    	,{0x03C,0x08,0x1}
	,{0x03C,0x0C,0x1}
    	,{0x03C,0x04,0x1}
	,{0x03C,0x06,0x1}
    	,{0x03C,0x02,0x1}
	,{0x03C,0x03,0x1}
};

struct moto_step_drv v_ccwtab[ONE_STEP_PULSE] = {
    	{0x3C0,0x10,0x1}
    	,{0x3C0,0x30,0x1}
	,{0x3C0,0x20,0x1}
	,{0x3C0,0x60,0x1}
	,{0x3C0,0x40,0x1}
	,{0x3C0,0xC0,0x1}
	,{0x3C0,0x80,0x1}
	,{0x3C0,0x90,0x1}
};

struct moto_step_drv v_cwtab[ONE_STEP_PULSE] = {
    	{0x3C0,0x10,0x1}
    	,{0x3C0,0x90,0x1}
	,{0x3C0,0x80,0x1}
	,{0x3C0,0xC0,0x1}
	,{0x3C0,0x40,0x1}
	,{0x3C0,0x60,0x1}
	,{0x3C0,0x20,0x1}
	,{0x3C0,0x30,0x1}
};
#elif (4 == ONE_STEP_PULSE)
/* AB-BC-CD-DA */
struct moto_step_drv h_ccwtab[ONE_STEP_PULSE] = {
    	{0x03C,0x03,0x1}
	,{0x03C,0x06,0x1}
	,{0x03C,0x0C,0x1}
	,{0x03C,0x09,0x1}
};

/* AB-DA-CD-BC */
struct moto_step_drv h_cwtab[ONE_STEP_PULSE] = {
    	{0x03C,0x03,0x1}
	,{0x03C,0x09,0x1}
	,{0x03C,0x0C,0x1}
	,{0x03C,0x06,0x1}
};

/* AB-BC-CD-DA */
struct moto_step_drv v_ccwtab[ONE_STEP_PULSE] = {
    	{0x3C0,0x30,0x1}
	,{0x3C0,0x60,0x1}
	,{0x3C0,0xC0,0x1}
	,{0x3C0,0x90,0x1}
};

/* AB-DA-CD-BC */
struct moto_step_drv v_cwtab[ONE_STEP_PULSE] = {
    	{0x3C0,0x30,0x1}
	,{0x3C0,0x90,0x1}
	,{0x3C0,0xC0,0x1}
	,{0x3C0,0x60,0x1}
};
#endif
/*
 *  delays for a specified number of micro seconds rountine.
 *
 *  @param usec: number of micro seconds to pause for
 *
 */
void time_delay_us(unsigned int usec)
{
	int i,j;
	
	for(i=0;i<usec * 5;i++)
	{
		for(j=0;j<47;j++)
		{;}
	}
}

static void moto_action_none(unsigned long arg)
{
    unsigned char regvalue;

    regvalue = HW_REG(GPIO_5_DIR);
    MOTO_DRV_LOG("%s  GPIO_5_DIR value %#x\n",__func__,regvalue);

    regvalue = HW_REG(ALL_MOTO_DIR_ADDR);
    MOTO_DRV_LOG("%s  ALL_MOTO_DIR_ADDR value %#x\n",__func__,regvalue);

    regvalue = HW_REG(GPIO_9_DIR);
    MOTO_DRV_LOG("%s  GPIO_9_DIR value %#x\n",__func__,regvalue);
}

static void moto_action_l(unsigned long arg)
{
    int i;
    unsigned char regval = GET_L_POSTION;
    struct moto_drv_action *pmda = (struct moto_drv_action *)arg;
    MOTO_DRV_LOG("%s entry\n",__func__);
    MOTO_DRV_LOG("%s GET_L_POSTION=%#x\n",__func__,regval);
#if 1
    if(regval)
    {
        /* go on */
	for(i=0; i<ONE_STEP_PULSE; i++)
	{
		HW_REG(IO_ADDRESS(GPIO_9_BASE + h_ccwtab[i].regoff)) = h_ccwtab[i].portval;
		msleep(h_ccwtab[i].delay);
	}
	#if 0
	HW_REG(IO_ADDRESS(GPIO_9_BASE + h_ccwtab[0].regoff)) = h_ccwtab[0].portval;
	/* delay */
	msleep(1);
	HW_REG(IO_ADDRESS(GPIO_9_BASE + h_ccwtab[1].regoff)) = h_ccwtab[1].portval;
	/* delay */
	msleep(1);
	HW_REG(IO_ADDRESS(GPIO_9_BASE + h_ccwtab[2].regoff)) = h_ccwtab[2].portval;
	/* delay */
	msleep(1);
	HW_REG(IO_ADDRESS(GPIO_9_BASE + h_ccwtab[3].regoff)) = h_ccwtab[3].portval;
	/* delay */
	msleep(1);
	#endif
	pmda->rsp = MOTO_RSP_DONE;
    }
    else
    {
        /* up to max step, nothing to do, return rsp to app */
       MOTO_DRV_LOG("%s max step stop\n",__func__);
	pmda->rsp = MOTO_RSP_MAX;
    }
#endif
#if 0
    i = 0;
    while(regval)
    {
	HW_REG(IO_ADDRESS(GPIO_9_BASE + h_ccwtab[i%ONE_STEP_PULSE].regoff)) = h_ccwtab[i%ONE_STEP_PULSE].portval;
	/* delay */
	msleep(1);
	i++;
    	regval = GET_L_POSTION;
    }
#endif
    MOTO_DRV_LOG("%s exit\n",__func__);
}

static void moto_action_r(unsigned long arg)
{
    int i;
    unsigned char regval = GET_R_POSTION;
    struct moto_drv_action *pmda = (struct moto_drv_action *)arg;
    MOTO_DRV_LOG("%s entry\n",__func__);
    MOTO_DRV_LOG("%s GET_R_POSTION=%#x\n",__func__,regval);
#if 1
    if(regval)
    {
        /* go on */
	for(i=0; i<ONE_STEP_PULSE; i++)
	{
		HW_REG(IO_ADDRESS(GPIO_9_BASE + h_cwtab[i].regoff)) = h_cwtab[i].portval;
		msleep(h_cwtab[i].delay);
	}
	#if 0
	HW_REG(IO_ADDRESS(GPIO_9_BASE + h_cwtab[0].regoff)) = h_cwtab[0].portval;
	/* delay */
	msleep(1);
	HW_REG(IO_ADDRESS(GPIO_9_BASE + h_cwtab[1].regoff)) = h_cwtab[1].portval;
	/* delay */
	msleep(1);
	HW_REG(IO_ADDRESS(GPIO_9_BASE + h_cwtab[2].regoff)) = h_cwtab[2].portval;
	/* delay */
	msleep(1);
	HW_REG(IO_ADDRESS(GPIO_9_BASE + h_cwtab[3].regoff)) = h_cwtab[3].portval;
	/* delay */
	msleep(1);
	#endif
	pmda->rsp = MOTO_RSP_DONE;
    }
    else
    {
        /* up to max step, nothing to do, return rsp to app */
       MOTO_DRV_LOG("%s max step stop\n",__func__);
	pmda->rsp = MOTO_RSP_MAX;
    }
#endif
    MOTO_DRV_LOG("%s exit\n",__func__);
}
static void moto_action_u(unsigned long arg)
{
    int i;
    unsigned char regval = GET_U_POSTION;
    struct moto_drv_action *pmda = (struct moto_drv_action *)arg;
    MOTO_DRV_LOG("%s entry\n",__func__);
    MOTO_DRV_LOG("%s GET_U_POSTION=%#x\n",__func__,regval);
#if 1
    if(regval)
    {
        /* go on */
	for(i=0; i<ONE_STEP_PULSE; i++)
	{
		HW_REG(IO_ADDRESS(GPIO_9_BASE + v_ccwtab[i].regoff)) = v_ccwtab[i].portval;
		msleep(v_ccwtab[i].delay);
	}
	#if 0
	HW_REG(IO_ADDRESS(GPIO_9_BASE + v_ccwtab[0].regoff)) = v_ccwtab[0].portval;
	/* delay */
	msleep(1);
	HW_REG(IO_ADDRESS(GPIO_9_BASE + v_ccwtab[1].regoff)) = v_ccwtab[1].portval;
	/* delay */
	msleep(1);
	HW_REG(IO_ADDRESS(GPIO_9_BASE + v_ccwtab[2].regoff)) = v_ccwtab[2].portval;
	/* delay */
	msleep(1);
	HW_REG(IO_ADDRESS(GPIO_9_BASE + v_ccwtab[3].regoff)) = v_ccwtab[3].portval;
	/* delay */
	msleep(1);
	#endif
	pmda->rsp = MOTO_RSP_DONE;
    }
    else
    {
        /* up to max step, nothing to do, return rsp to app */
       MOTO_DRV_LOG("%s max step stop\n",__func__);
	pmda->rsp = MOTO_RSP_MAX;
    }
#endif
    MOTO_DRV_LOG("%s exit\n",__func__);
}
static void moto_action_d(unsigned long arg)
{
    int i;
    unsigned char regval = GET_D_POSTION;
    struct moto_drv_action *pmda = (struct moto_drv_action *)arg;
    MOTO_DRV_LOG("%s entry\n",__func__);
    MOTO_DRV_LOG("%s GET_D_POSTION=%#x\n",__func__,regval);
#if 1
    if(regval)
    {
        /* go on */
	for(i=0; i<ONE_STEP_PULSE; i++)
	{
		HW_REG(IO_ADDRESS(GPIO_9_BASE + v_cwtab[i].regoff)) = v_cwtab[i].portval;
		msleep(v_cwtab[i].delay);
	}
	#if 0
	HW_REG(IO_ADDRESS(GPIO_9_BASE + v_cwtab[0].regoff)) = v_cwtab[0].portval;
	/* delay */
	msleep(1);
	HW_REG(IO_ADDRESS(GPIO_9_BASE + v_cwtab[1].regoff)) = v_cwtab[1].portval;
	/* delay */
	msleep(1);
	HW_REG(IO_ADDRESS(GPIO_9_BASE + v_cwtab[2].regoff)) = v_cwtab[2].portval;
	/* delay */
	msleep(1);
	HW_REG(IO_ADDRESS(GPIO_9_BASE + v_cwtab[3].regoff)) = v_cwtab[3].portval;
	/* delay */
	msleep(1);
	#endif
	pmda->rsp = MOTO_RSP_DONE;
    }
    else
    {
        /* up to max step, nothing to do, return rsp to app */
       MOTO_DRV_LOG("%s max step stop\n",__func__);
	pmda->rsp = MOTO_RSP_MAX;
    }
#endif
    MOTO_DRV_LOG("%s exit\n",__func__);
}

long moto_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct moto_drv_action *pmda = (struct moto_drv_action *)arg;
	
	switch(cmd)
	{
		case MOTO_DRV_DIR_CTL:
			switch(pmda->ind)
			{
				case MOTO_IND_L:
					moto_action_l(arg);
					break;
				case MOTO_IND_R:
					moto_action_r(arg);
					break;
				case MOTO_IND_U:
					moto_action_u(arg);
					break;
				case MOTO_IND_D:
					moto_action_d(arg);
					break;
				default:
					return -1;
			}
			break;
		
		case MOTO_DRV_NONE:
			moto_action_none(arg);
			break;

		default:
			return -1;
	}
    return 0;
}

int moto_open(struct inode * inode, struct file * file)
{
    return 0;
}
int moto_close(struct inode * inode, struct file * file)
{
    return 0;
}


static struct file_operations moto_fops = {
    .owner      = THIS_MODULE,
    //.ioctl      = moto_ioctl,
    .unlocked_ioctl = moto_ioctl,
    .open       = moto_open,
    .release    = moto_close
};

static struct miscdevice moto_dev = {
   .minor		= MISC_DYNAMIC_MINOR,
   .name		= "moto_drv",
   .fops  = &moto_fops,
};

static int __init moto_drv_init(void)
{
    int ret;
    unsigned char regvalue;

    MOTO_DRV_LOG("%s entry\n",__func__);
    
    ret = misc_register(&moto_dev);
    if(0 != ret)
    	return -1;

    /* position switch */        
    regvalue = HW_REG(GPIO_5_DIR);
    MOTO_DRV_LOG("%s  GPIO_5_DIR old value %#x\n",__func__,regvalue);
    regvalue &= 0x0F;
    HW_REG(GPIO_5_DIR) = regvalue;
    #if defined(MOTO_DBG)
    regvalue = HW_REG(GPIO_5_DIR);
    MOTO_DRV_LOG("%s  GPIO_5_DIR new value %#x\n",__func__,regvalue);
    #endif

    /* moto driver */        
    regvalue = HW_REG(GPIO_9_DIR);
    MOTO_DRV_LOG("%s  GPIO_9_DIR old value %#x\n",__func__,regvalue);
    regvalue |= 0xFF;
    HW_REG(GPIO_9_DIR) = regvalue;
    #if defined(MOTO_DBG)
    regvalue = HW_REG(GPIO_9_DIR);
    MOTO_DRV_LOG("%s  GPIO_9_DIR new value %#x\n",__func__,regvalue);
    #endif

    MOTO_DRV_LOG("%s exit\n",__func__);
    return 0;    
}

static void __exit moto_drv_exit(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    misc_deregister(&moto_dev);
    MOTO_DRV_LOG("%s exit\n",__func__);
}


module_init(moto_drv_init);
module_exit(moto_drv_exit);

#ifdef MODULE
//#include <linux/compile.h>
#endif
//MODULE_INFO(build, UTS_VERSION);
MODULE_LICENSE("GPL");




