

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/sched.h>
//#include <asm/hardware.h>
#include <linux/interrupt.h>

#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/errno.h>
#include <linux/fcntl.h>

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>

#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/io.h>
#include <linux/version.h>

#include "moto_drv.h" 

/*
 *http://wenku.baidu.com/link?url=s5q1LhFbmOVIjVmPk2GwUbDTuyMnvLKrYmlqvDQFpNGbr2cYEfXwaWAHTgPe9AjJHx2Jcdi8nRY2SfY3p9mCwZaGvQIsJ5A1LwDqMeYjqEG&qq-pf-to=pcqq.discussion
*/
#if defined(MOTO_DBG)
#define MOTO_DRV_LOG printk
#else
#define MOTO_DRV_LOG
#endif

#define H_MAX_PLULSE (5184)
#define V_MAX_PLULSE (1712)

/* ONE_STEP_PULSE/step */
#define H_MOTO_DEFAULT_STEP (324)
//#define H_MOTO_DEFAULT_STEP (0)
#define V_MOTO_DEFAULT_STEP (142)

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

#define MOTO_DEVICE_IRQ_NO (4)
#define TIMER3_BASE (0x20010000)
#define TIMER3_LOAD_REG IO_ADDRESS(TIMER3_BASE + 0x020)
#define TIMER3_VALUE_REG IO_ADDRESS(TIMER3_BASE + 0x024)
#define TIMER3_CONTROL_REG IO_ADDRESS(TIMER3_BASE + 0x028)
#define TIMER3_INTCLR_REG IO_ADDRESS(TIMER3_BASE + 0x02C)
#define TIMER3_RIS_REG IO_ADDRESS(TIMER3_BASE + 0x030)
#define TIMER3_MIS_REG IO_ADDRESS(TIMER3_BASE + 0x034)
#define TIMER3_BGLOAD_REG IO_ADDRESS(TIMER3_BASE + 0x038)

#define TIMER3_ENABLE HW_REG(TIMER3_CONTROL_REG) |= (1<<7)
#define TIMER3_DISABLE HW_REG(TIMER3_CONTROL_REG) &= ~(1<<7)
//#define MOTO_GPIO_DEFAULT HW_REG(IO_ADDRESS(GPIO_9_BASE + 0x3FC)) = 0xFF
#define MOTO_GPIO_DEFAULT HW_REG(IO_ADDRESS(GPIO_9_BASE + 0x3FC)) = 0x0
//#define MOTO_GPIO_DEFAULT

#define SC_BASE (0x20050000)
#define SC_CTRL_REG IO_ADDRESS(SC_BASE + 0x000)
#define SC_LOCKEN_REG IO_ADDRESS(SC_BASE + 0x044)

struct moto_step_drv {
    unsigned int regoff;
    unsigned char portval;
    int delay;/* ms */
};
#define ONE_STEP_PULSE (8)

#define PRESET_POS_NUM (8)
typedef struct
{
    int speed;
    int curr_step;
    int default_step;
    int is_running;
    int curr_dir;
    //struct moto_pos preset_pos[PRESET_POS_NUM];
    struct moto_step_drv *ordertab[MOTO_ORDER_NUM];
}moto_dev_param;

union inparams {
	unsigned int nsteps;
	struct moto_pos pos;
};

typedef struct
{
    moto_dev_param dev_param[MOTO_NUM];
    int is_self_test;
    int cmd;
    int state;
    int pulse_num;/*-1 means infinite*/
    int pulse_cnt;/**/
    int to_idle;
    unsigned long arg;
    union inparams ins;
}moto_dev_struct;

struct moto_action_func {
	unsigned int cmd;
	void *start_func;
	void *run_func;
	void *stop_func;
};

typedef void (*action_func)(void);

void moto_action_start(unsigned int cmd, unsigned long arg);
void moto_action_run(void);
void moto_action_stop(void);
void self_test_start(void);
void self_test_run(void);
void self_test_stop(void);
void hccw_start(void);
void hccw_run(void);
void hccw_stop(void);
void hcw_start(void);
void hcw_run(void);
void hcw_stop(void);
void vccw_start(void);
void vccw_run(void);
void vccw_stop(void);
void vcw_start(void);
void vcw_run(void);
void vcw_stop(void);
void hccwb_start(void);
void hccwb_run(void);
void hccwb_stop(void);
void hcwb_start(void);
void hcwb_run(void);
void hcwb_stop(void);
void vccwb_start(void);
void vccwb_run(void);
void vccwb_stop(void);
void vcwb_start(void);
void vcwb_run(void);
void vcwb_stop(void);
void hcruising_start(void);
void hcruising_run(void);
void hcruising_stop(void);
void vcruising_start(void);
void vcruising_run(void);
void vcruising_stop(void);
void hvcruising_start(void);
void hvcruising_run(void);
void hvcruising_stop(void);
void topos_start(void);
void topos_run(void);
void topos_stop(void);


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

static moto_dev_struct moto_dev={0};

struct moto_action_func moto_action_func_tab[] = {
    	{MTDRV_SELF_TEST,self_test_start,self_test_run,self_test_stop}
    	,{MTDRV_HCCW,hccw_start,hccw_run,hccw_stop}
    	,{MTDRV_HCW,hcw_start,hcw_run,hcw_stop}
    	,{MTDRV_VCCW,vccw_start,vccw_run,vccw_stop}
    	,{MTDRV_VCW,vcw_start,vcw_run,vcw_stop}
    	,{MTDRV_HCCWB,hccwb_start,hccwb_run,hccwb_stop}
    	,{MTDRV_HCWB,hcwb_start,hcwb_run,hcwb_stop}
    	,{MTDRV_VCCWB,vccwb_start,vccwb_run,vccwb_stop}
    	,{MTDRV_VCWB,vcwb_start,vcwb_run,vcwb_stop}
    	,{MTDRV_HCRUISING,hcruising_start,hcruising_run,hcruising_stop}
    	,{MTDRV_VCRUISING,vcruising_start,vcruising_run,vcruising_stop}
    	,{MTDRV_HVCRUISING,hvcruising_start,hvcruising_run,hvcruising_stop}
    	,{MTDRV_TO_POS,topos_start,topos_run,topos_stop}
	,{0xFFFFFFFF,NULL,NULL,NULL}
};
#define MOTO_ACITON_FUNC_TAB_SIZE (sizeof(moto_action_func_tab)/sizeof(moto_action_func_tab[0]))

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

static irqreturn_t moto_interrupt(int irq, void *dev_id)
{
    static int cnt = 0;
    MOTO_DRV_LOG("%s entry\n",__func__);
    /* moto action running */
    moto_action_run();
    HW_REG(TIMER3_INTCLR_REG) = 0x0;
    MOTO_DRV_LOG("%s exit\n",__func__);
    return IRQ_HANDLED;
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

void *get_moto_action_func(unsigned int cmd)
{
    struct moto_action_func *pmaf = &moto_action_func_tab[0];
    while(pmaf->cmd != 0xFFFFFFFF)
    {
        if(cmd == pmaf->cmd)
        {
            return pmaf;
        }
        pmaf++;
    }
    return NULL;
}

void moto_action_start(unsigned int cmd, unsigned long arg)
{
    struct moto_action_func *pmaf = get_moto_action_func(cmd);
    action_func func = NULL;
    MOTO_DRV_LOG("%s entry\n",__func__);
    if(!pmaf) return;
    MOTO_DRV_LOG("%s 11111\n",__func__);
    func = (action_func)pmaf->start_func;
    if(!func) return;
    MOTO_DRV_LOG("%s 22222\n",__func__);
    moto_dev.arg = arg;
    func();
    MOTO_DRV_LOG("%s exit\n",__func__);
}

void moto_action_run(void)
{
    struct moto_action_func *pmaf = get_moto_action_func(moto_dev.cmd);
    action_func func = NULL;
    if(!pmaf) return;
    func = (action_func)pmaf->run_func;
    if(!func) return;
    func();
}

void moto_action_stop(void)
{
    struct moto_action_func *pmaf = get_moto_action_func(moto_dev.cmd);
    action_func func = NULL;
    if(!pmaf) return;
    func = (action_func)pmaf->stop_func;
    if(!func) return;
    func();
}

void self_test_start(void)
{
    moto_dev.is_self_test = 0;
    moto_dev.cmd = MTDRV_SELF_TEST;
    moto_dev.state = MOTO_STATE_SELF_TEST;
    moto_dev.pulse_num = H_MAX_PLULSE+H_MOTO_DEFAULT_STEP*ONE_STEP_PULSE;
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    moto_dev.dev_param[MOTO_HDEV].curr_step = -1;
    moto_dev.dev_param[MOTO_HDEV].default_step = H_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_HDEV].is_running = 1;

    moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    moto_dev.dev_param[MOTO_VDEV].curr_step = -1;
    moto_dev.dev_param[MOTO_VDEV].default_step = V_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_VDEV].is_running = 1;

    TIMER3_ENABLE;
}
void self_test_run(void)
{
    struct moto_step_drv *pmsd;

    if(moto_dev.pulse_num == moto_dev.pulse_cnt)
    {
        /* moto action stop */
	moto_action_stop();
	return;
    }
    if(moto_dev.pulse_cnt < H_MAX_PLULSE)
    {
        pmsd = moto_dev.dev_param[MOTO_HDEV].ordertab[MOTO_CCW];
    }
    else
    {
        pmsd = moto_dev.dev_param[MOTO_HDEV].ordertab[MOTO_CW];
    }
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    if(moto_dev.pulse_cnt < V_MAX_PLULSE)
    {
        pmsd = moto_dev.dev_param[MOTO_VDEV].ordertab[MOTO_CCW];
    }
    else
    {
        pmsd = moto_dev.dev_param[MOTO_VDEV].ordertab[MOTO_CW];
    }
    if(moto_dev.pulse_cnt < (V_MAX_PLULSE+V_MOTO_DEFAULT_STEP*ONE_STEP_PULSE))
    {
        pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
        HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;
    }

    moto_dev.pulse_cnt++;
}
void self_test_stop(void)
{
    TIMER3_DISABLE;

    moto_dev.is_self_test = 1;
    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    moto_dev.dev_param[MOTO_HDEV].curr_step = moto_dev.dev_param[MOTO_HDEV].default_step;
    //moto_dev.dev_param[MOTO_HDEV].default_step = -1;
    moto_dev.dev_param[MOTO_HDEV].is_running = 0;

    //moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    moto_dev.dev_param[MOTO_VDEV].curr_step = moto_dev.dev_param[MOTO_VDEV].default_step;
    //moto_dev.dev_param[MOTO_VDEV].default_step = -1;
    moto_dev.dev_param[MOTO_VDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
}

void hccw_start(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    moto_dev.ins.nsteps = *(unsigned int *)moto_dev.arg;
    moto_dev.cmd = MTDRV_HCCW;
    moto_dev.state = MOTO_STATE_TRIMMING;
    if(moto_dev.ins.nsteps <= moto_dev.dev_param[MOTO_HDEV].curr_step)
    {
        moto_dev.pulse_num = moto_dev.ins.nsteps*ONE_STEP_PULSE;
    }
    else
    {
        moto_dev.pulse_num = moto_dev.dev_param[MOTO_HDEV].curr_step*ONE_STEP_PULSE;
    }
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_HDEV].default_step = H_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_HDEV].is_running = 1;

    TIMER3_ENABLE;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hccw_run(void)
{
    struct moto_step_drv *pmsd;

    MOTO_DRV_LOG("%s entry\n",__func__);
    if(moto_dev.pulse_num == moto_dev.pulse_cnt)
    {
        /* moto action stop */
	moto_action_stop();
	return;
    }
    pmsd = moto_dev.dev_param[MOTO_HDEV].ordertab[MOTO_CCW];
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    moto_dev.pulse_cnt++;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hccw_stop(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    TIMER3_DISABLE;

    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    moto_dev.dev_param[MOTO_HDEV].curr_step -= moto_dev.ins.nsteps;
    if(moto_dev.dev_param[MOTO_HDEV].curr_step < 0)
    {
        moto_dev.dev_param[MOTO_HDEV].curr_step = 0;
    }
    //moto_dev.dev_param[MOTO_HDEV].default_step = -1;
    moto_dev.dev_param[MOTO_HDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hcw_start(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    moto_dev.ins.nsteps = *(unsigned int *)moto_dev.arg;
    moto_dev.cmd = MTDRV_HCW;
    moto_dev.state = MOTO_STATE_TRIMMING;
    if((moto_dev.dev_param[MOTO_HDEV].curr_step+moto_dev.ins.nsteps) \
		> (H_MAX_PLULSE/ONE_STEP_PULSE))
    {
        moto_dev.pulse_num = H_MAX_PLULSE - moto_dev.dev_param[MOTO_HDEV].curr_step*ONE_STEP_PULSE;
    }
    else
    {
        moto_dev.pulse_num = moto_dev.ins.nsteps*ONE_STEP_PULSE;
    }
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_HDEV].default_step = H_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_HDEV].is_running = 1;

    TIMER3_ENABLE;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hcw_run(void)
{
    struct moto_step_drv *pmsd;

    MOTO_DRV_LOG("%s entry\n",__func__);
    if(moto_dev.pulse_num == moto_dev.pulse_cnt)
    {
        /* moto action stop */
	moto_action_stop();
	return;
    }
    pmsd = moto_dev.dev_param[MOTO_HDEV].ordertab[MOTO_CW];
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    moto_dev.pulse_cnt++;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hcw_stop(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    TIMER3_DISABLE;

    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    moto_dev.dev_param[MOTO_HDEV].curr_step += moto_dev.ins.nsteps;
    if(moto_dev.dev_param[MOTO_HDEV].curr_step > (H_MAX_PLULSE/ONE_STEP_PULSE))
    {
        moto_dev.dev_param[MOTO_HDEV].curr_step = (H_MAX_PLULSE/ONE_STEP_PULSE);
    }
    //moto_dev.dev_param[MOTO_HDEV].default_step = -1;
    moto_dev.dev_param[MOTO_HDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
    MOTO_DRV_LOG("%s exit\n",__func__);
}

void vccw_start(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    moto_dev.ins.nsteps = *(unsigned int *)moto_dev.arg;
    moto_dev.cmd = MTDRV_VCCW;
    moto_dev.state = MOTO_STATE_TRIMMING;
    if(moto_dev.ins.nsteps <= moto_dev.dev_param[MOTO_VDEV].curr_step)
    {
        moto_dev.pulse_num = moto_dev.ins.nsteps*ONE_STEP_PULSE;
    }
    else
    {
        moto_dev.pulse_num = moto_dev.dev_param[MOTO_VDEV].curr_step*ONE_STEP_PULSE;
    }
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_VDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_VDEV].default_step = V_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_VDEV].is_running = 1;

    TIMER3_ENABLE;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vccw_run(void)
{
    struct moto_step_drv *pmsd;

    MOTO_DRV_LOG("%s entry\n",__func__);
    if(moto_dev.pulse_num == moto_dev.pulse_cnt)
    {
        /* moto action stop */
	moto_action_stop();
	return;
    }
    pmsd = moto_dev.dev_param[MOTO_VDEV].ordertab[MOTO_CCW];
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    moto_dev.pulse_cnt++;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vccw_stop(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    TIMER3_DISABLE;

    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    moto_dev.dev_param[MOTO_VDEV].curr_step -= moto_dev.ins.nsteps;
    if(moto_dev.dev_param[MOTO_VDEV].curr_step < 0)
    {
        moto_dev.dev_param[MOTO_VDEV].curr_step = 0;
    }
    //moto_dev.dev_param[MOTO_VDEV].default_step = -1;
    moto_dev.dev_param[MOTO_VDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vcw_start(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    moto_dev.ins.nsteps = *(unsigned int *)moto_dev.arg;
    moto_dev.cmd = MTDRV_VCW;
    moto_dev.state = MOTO_STATE_TRIMMING;
    if((moto_dev.dev_param[MOTO_VDEV].curr_step+moto_dev.ins.nsteps) \
		> (V_MAX_PLULSE/ONE_STEP_PULSE))
    {
        moto_dev.pulse_num = V_MAX_PLULSE - moto_dev.dev_param[MOTO_VDEV].curr_step*ONE_STEP_PULSE;
    }
    else
    {
        moto_dev.pulse_num = moto_dev.ins.nsteps*ONE_STEP_PULSE;
    }
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_VDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_VDEV].default_step = V_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_VDEV].is_running = 1;

    TIMER3_ENABLE;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vcw_run(void)
{
    struct moto_step_drv *pmsd;

    MOTO_DRV_LOG("%s entry\n",__func__);
    if(moto_dev.pulse_num == moto_dev.pulse_cnt)
    {
        /* moto action stop */
	moto_action_stop();
	return;
    }
    pmsd = moto_dev.dev_param[MOTO_VDEV].ordertab[MOTO_CW];
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    moto_dev.pulse_cnt++;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vcw_stop(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    TIMER3_DISABLE;

    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    moto_dev.dev_param[MOTO_VDEV].curr_step += moto_dev.ins.nsteps;
    if(moto_dev.dev_param[MOTO_VDEV].curr_step > (V_MAX_PLULSE/ONE_STEP_PULSE))
    {
        moto_dev.dev_param[MOTO_VDEV].curr_step = (V_MAX_PLULSE/ONE_STEP_PULSE);
    }
    //moto_dev.dev_param[MOTO_VDEV].default_step = -1;
    moto_dev.dev_param[MOTO_VDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hccwb_start(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    //moto_dev.ins.nsteps = *(unsigned int *)moto_dev.arg;
    moto_dev.cmd = MTDRV_HCCWB;
    moto_dev.state = MOTO_STATE_TOBOUND;
    moto_dev.pulse_num = moto_dev.dev_param[MOTO_HDEV].curr_step*ONE_STEP_PULSE;
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_HDEV].default_step = H_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_HDEV].is_running = 1;

    TIMER3_ENABLE;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hccwb_run(void)
{
    struct moto_step_drv *pmsd;

    MOTO_DRV_LOG("%s entry\n",__func__);
    if(((moto_dev.to_idle == 1) && ((moto_dev.pulse_cnt%ONE_STEP_PULSE) == 0))
		| (moto_dev.pulse_num == moto_dev.pulse_cnt))
    {
        /* moto action stop */
	moto_action_stop();
	return;
    }
    pmsd = moto_dev.dev_param[MOTO_HDEV].ordertab[MOTO_CCW];
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    moto_dev.pulse_cnt++;
    if((moto_dev.pulse_cnt%ONE_STEP_PULSE) == 0)
    {
        moto_dev.dev_param[MOTO_HDEV].curr_step--;
        if(moto_dev.dev_param[MOTO_HDEV].curr_step < 0)
        {
            moto_dev.dev_param[MOTO_HDEV].curr_step = 0;
        }
    }
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hccwb_stop(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    TIMER3_DISABLE;

    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].default_step = -1;
    moto_dev.dev_param[MOTO_HDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hcwb_start(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    //moto_dev.ins.nsteps = *(unsigned int *)moto_dev.arg;
    moto_dev.cmd = MTDRV_HCWB;
    moto_dev.state = MOTO_STATE_TOBOUND;
    moto_dev.pulse_num = H_MAX_PLULSE - moto_dev.dev_param[MOTO_HDEV].curr_step*ONE_STEP_PULSE;
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_HDEV].default_step = H_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_HDEV].is_running = 1;

    TIMER3_ENABLE;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hcwb_run(void)
{
    struct moto_step_drv *pmsd;

    MOTO_DRV_LOG("%s entry\n",__func__);
    if(((moto_dev.to_idle == 1) && ((moto_dev.pulse_cnt%ONE_STEP_PULSE) == 0))
		| (moto_dev.pulse_num == moto_dev.pulse_cnt))
    {
        /* moto action stop */
	moto_action_stop();
	return;
    }
    pmsd = moto_dev.dev_param[MOTO_HDEV].ordertab[MOTO_CW];
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    moto_dev.pulse_cnt++;
    if((moto_dev.pulse_cnt%ONE_STEP_PULSE) == 0)
    {
        moto_dev.dev_param[MOTO_HDEV].curr_step++;
        if(moto_dev.dev_param[MOTO_HDEV].curr_step > (H_MAX_PLULSE/ONE_STEP_PULSE))
        {
            moto_dev.dev_param[MOTO_HDEV].curr_step = (H_MAX_PLULSE/ONE_STEP_PULSE);
        }
    }
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hcwb_stop(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    TIMER3_DISABLE;

    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].default_step = -1;
    moto_dev.dev_param[MOTO_HDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
    MOTO_DRV_LOG("%s exit\n",__func__);
}

void vccwb_start(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    //moto_dev.ins.nsteps = *(unsigned int *)moto_dev.arg;
    moto_dev.cmd = MTDRV_VCCWB;
    moto_dev.state = MOTO_STATE_TOBOUND;
    moto_dev.pulse_num = moto_dev.dev_param[MOTO_VDEV].curr_step*ONE_STEP_PULSE;
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_VDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_VDEV].default_step = V_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_VDEV].is_running = 1;

    TIMER3_ENABLE;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vccwb_run(void)
{
    struct moto_step_drv *pmsd;

    MOTO_DRV_LOG("%s entry\n",__func__);
    if(((moto_dev.to_idle == 1) && ((moto_dev.pulse_cnt%ONE_STEP_PULSE) == 0))
		| (moto_dev.pulse_num == moto_dev.pulse_cnt))
    {
        /* moto action stop */
	moto_action_stop();
	return;
    }
    pmsd = moto_dev.dev_param[MOTO_VDEV].ordertab[MOTO_CCW];
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    moto_dev.pulse_cnt++;
    if((moto_dev.pulse_cnt%ONE_STEP_PULSE) == 0)
    {
        moto_dev.dev_param[MOTO_VDEV].curr_step--;
        if(moto_dev.dev_param[MOTO_VDEV].curr_step < 0)
        {
            moto_dev.dev_param[MOTO_VDEV].curr_step = 0;
        }
    }
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vccwb_stop(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    TIMER3_DISABLE;

    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_VDEV].default_step = -1;
    moto_dev.dev_param[MOTO_VDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vcwb_start(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    //moto_dev.ins.nsteps = *(unsigned int *)moto_dev.arg;
    moto_dev.cmd = MTDRV_VCWB;
    moto_dev.state = MOTO_STATE_TOBOUND;
    moto_dev.pulse_num = V_MAX_PLULSE - moto_dev.dev_param[MOTO_VDEV].curr_step*ONE_STEP_PULSE;
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_VDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_VDEV].default_step = V_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_VDEV].is_running = 1;

    TIMER3_ENABLE;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vcwb_run(void)
{
    struct moto_step_drv *pmsd;

    MOTO_DRV_LOG("%s entry\n",__func__);
    if(((moto_dev.to_idle == 1) && ((moto_dev.pulse_cnt%ONE_STEP_PULSE) == 0))
		| (moto_dev.pulse_num == moto_dev.pulse_cnt))
    {
        /* moto action stop */
	moto_action_stop();
	return;
    }
    pmsd = moto_dev.dev_param[MOTO_VDEV].ordertab[MOTO_CW];
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    moto_dev.pulse_cnt++;
    if((moto_dev.pulse_cnt%ONE_STEP_PULSE) == 0)
    {
        moto_dev.dev_param[MOTO_VDEV].curr_step++;
        if(moto_dev.dev_param[MOTO_VDEV].curr_step > (V_MAX_PLULSE/ONE_STEP_PULSE))
        {
            moto_dev.dev_param[MOTO_VDEV].curr_step = (V_MAX_PLULSE/ONE_STEP_PULSE);
        }
    }
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vcwb_stop(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    TIMER3_DISABLE;

    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_VDEV].default_step = -1;
    moto_dev.dev_param[MOTO_VDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
    MOTO_DRV_LOG("%s exit\n",__func__);
}

void hcruising_start(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    moto_dev.cmd = MTDRV_HCRUISING;
    moto_dev.state = MOTO_STATE_CRUISING;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_HDEV].default_step = H_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_HDEV].is_running = 1;
    if(moto_dev.dev_param[MOTO_HDEV].curr_step < (H_MAX_PLULSE/ONE_STEP_PULSE)/2)
    {
        moto_dev.dev_param[MOTO_HDEV].curr_dir = MOTO_CW;
    }
    else
    {
        moto_dev.dev_param[MOTO_HDEV].curr_dir = MOTO_CCW;
    }

    TIMER3_ENABLE;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hcruising_run(void)
{
    struct moto_step_drv *pmsd;

    MOTO_DRV_LOG("%s entry\n",__func__);
    if((moto_dev.to_idle == 1) && ((moto_dev.pulse_cnt%ONE_STEP_PULSE) == 0))
    {
        /* moto action stop */
	MOTO_DRV_LOG("%s stop\n",__func__);
	moto_action_stop();
	return;
    }
    pmsd = moto_dev.dev_param[MOTO_HDEV].ordertab[moto_dev.dev_param[MOTO_HDEV].curr_dir];
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    moto_dev.pulse_cnt++;
    moto_dev.pulse_cnt %= ONE_STEP_PULSE;
    if(0 == moto_dev.pulse_cnt)
    {
        if(MOTO_CW == moto_dev.dev_param[MOTO_HDEV].curr_dir)
        {
            moto_dev.dev_param[MOTO_HDEV].curr_step++;
            if(moto_dev.dev_param[MOTO_HDEV].curr_step > (H_MAX_PLULSE/ONE_STEP_PULSE))
            {
                moto_dev.dev_param[MOTO_HDEV].curr_dir = MOTO_CCW;
            }
        }
        else if(MOTO_CCW == moto_dev.dev_param[MOTO_HDEV].curr_dir)
        {
            moto_dev.dev_param[MOTO_HDEV].curr_step--;
            if(0 == moto_dev.dev_param[MOTO_HDEV].curr_step)
            {
                moto_dev.dev_param[MOTO_HDEV].curr_dir = MOTO_CW;
            }
        }
    }
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hcruising_stop(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    TIMER3_DISABLE;

    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].curr_step = moto_dev.dev_param[MOTO_HDEV].default_step;
    //moto_dev.dev_param[MOTO_HDEV].default_step = -1;
    moto_dev.dev_param[MOTO_HDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vcruising_start(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    moto_dev.cmd = MTDRV_VCRUISING;
    moto_dev.state = MOTO_STATE_CRUISING;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_VDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_VDEV].default_step = V_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_VDEV].is_running = 1;
    if(moto_dev.dev_param[MOTO_VDEV].curr_step < (V_MAX_PLULSE/ONE_STEP_PULSE)/2)
    {
        moto_dev.dev_param[MOTO_VDEV].curr_dir = MOTO_CW;
    }
    else
    {
        moto_dev.dev_param[MOTO_VDEV].curr_dir = MOTO_CCW;
    }

    TIMER3_ENABLE;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vcruising_run(void)
{
    struct moto_step_drv *pmsd;

    MOTO_DRV_LOG("%s entry\n",__func__);
    if((moto_dev.to_idle == 1) && ((moto_dev.pulse_cnt%ONE_STEP_PULSE) == 0))
    {
        /* moto action stop */
	MOTO_DRV_LOG("%s stop\n",__func__);
	moto_action_stop();
	return;
    }
    pmsd = moto_dev.dev_param[MOTO_VDEV].ordertab[moto_dev.dev_param[MOTO_VDEV].curr_dir];
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    moto_dev.pulse_cnt++;
    moto_dev.pulse_cnt %= ONE_STEP_PULSE;
    if(0 == moto_dev.pulse_cnt)
    {
        if(MOTO_CW == moto_dev.dev_param[MOTO_VDEV].curr_dir)
        {
            moto_dev.dev_param[MOTO_VDEV].curr_step++;
            if(moto_dev.dev_param[MOTO_VDEV].curr_step > (V_MAX_PLULSE/ONE_STEP_PULSE))
            {
                moto_dev.dev_param[MOTO_VDEV].curr_dir = MOTO_CCW;
            }
        }
        else if(MOTO_CCW == moto_dev.dev_param[MOTO_VDEV].curr_dir)
        {
            moto_dev.dev_param[MOTO_VDEV].curr_step--;
            if(0 == moto_dev.dev_param[MOTO_VDEV].curr_step)
            {
                moto_dev.dev_param[MOTO_VDEV].curr_dir = MOTO_CW;
            }
        }
    }
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void vcruising_stop(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    TIMER3_DISABLE;

    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].curr_step = moto_dev.dev_param[MOTO_HDEV].default_step;
    //moto_dev.dev_param[MOTO_HDEV].default_step = -1;
    moto_dev.dev_param[MOTO_VDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
    MOTO_DRV_LOG("%s exit\n",__func__);
}

void hvcruising_start(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    /* =========================================================== */
    moto_dev.cmd = MTDRV_HVCRUISING;
    moto_dev.state = MOTO_STATE_CRUISING;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_HDEV].default_step = H_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_HDEV].is_running = 1;
    if(moto_dev.dev_param[MOTO_HDEV].curr_step < (H_MAX_PLULSE/ONE_STEP_PULSE)/2)
    {
        moto_dev.dev_param[MOTO_HDEV].curr_dir = MOTO_CW;
    }
    else
    {
        moto_dev.dev_param[MOTO_HDEV].curr_dir = MOTO_CCW;
    }

    moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_VDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_VDEV].default_step = V_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_VDEV].is_running = 1;
    if(moto_dev.dev_param[MOTO_VDEV].curr_step < (V_MAX_PLULSE/ONE_STEP_PULSE)/2)
    {
        moto_dev.dev_param[MOTO_VDEV].curr_dir = MOTO_CW;
    }
    else
    {
        moto_dev.dev_param[MOTO_VDEV].curr_dir = MOTO_CCW;
    }

    TIMER3_ENABLE;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hvcruising_run(void)
{
    struct moto_step_drv *pmsd;

    MOTO_DRV_LOG("%s entry\n",__func__);
    if((moto_dev.to_idle == 1) && ((moto_dev.pulse_cnt%ONE_STEP_PULSE) == 0))
    {
        /* moto action stop */
	MOTO_DRV_LOG("%s stop\n",__func__);
	moto_action_stop();
	return;
    }

    pmsd = moto_dev.dev_param[MOTO_HDEV].ordertab[moto_dev.dev_param[MOTO_HDEV].curr_dir];
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    pmsd = moto_dev.dev_param[MOTO_VDEV].ordertab[moto_dev.dev_param[MOTO_VDEV].curr_dir];
    pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
    HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;

    moto_dev.pulse_cnt++;
    moto_dev.pulse_cnt %= ONE_STEP_PULSE;
    if(0 == moto_dev.pulse_cnt)
    {
        if(MOTO_CW == moto_dev.dev_param[MOTO_HDEV].curr_dir)
        {
            moto_dev.dev_param[MOTO_HDEV].curr_step++;
            if(moto_dev.dev_param[MOTO_HDEV].curr_step > (H_MAX_PLULSE/ONE_STEP_PULSE))
            {
                moto_dev.dev_param[MOTO_HDEV].curr_dir = MOTO_CCW;
            }
        }
        else if(MOTO_CCW == moto_dev.dev_param[MOTO_HDEV].curr_dir)
        {
            moto_dev.dev_param[MOTO_HDEV].curr_step--;
            if(0 == moto_dev.dev_param[MOTO_HDEV].curr_step)
            {
                moto_dev.dev_param[MOTO_HDEV].curr_dir = MOTO_CW;
            }
        }

        if(MOTO_CW == moto_dev.dev_param[MOTO_VDEV].curr_dir)
        {
            moto_dev.dev_param[MOTO_VDEV].curr_step++;
            if(moto_dev.dev_param[MOTO_VDEV].curr_step > (V_MAX_PLULSE/ONE_STEP_PULSE))
            {
                moto_dev.dev_param[MOTO_VDEV].curr_dir = MOTO_CCW;
            }
        }
        else if(MOTO_CCW == moto_dev.dev_param[MOTO_VDEV].curr_dir)
        {
            moto_dev.dev_param[MOTO_VDEV].curr_step--;
            if(0 == moto_dev.dev_param[MOTO_VDEV].curr_step)
            {
                moto_dev.dev_param[MOTO_VDEV].curr_dir = MOTO_CW;
            }
        }
    }
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void hvcruising_stop(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    TIMER3_DISABLE;

    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].curr_step = moto_dev.dev_param[MOTO_HDEV].default_step;
    //moto_dev.dev_param[MOTO_HDEV].default_step = -1;
    moto_dev.dev_param[MOTO_HDEV].is_running = 0;
    moto_dev.dev_param[MOTO_VDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void topos_start(void)
{
    unsigned int hsteps,vsteps,nsteps;
    MOTO_DRV_LOG("%s entry\n",__func__);
    /* =========================================================== */
    moto_dev.ins.pos.hpos = ((struct moto_pos *)moto_dev.arg)->hpos;
    moto_dev.ins.pos.vpos = ((struct moto_pos *)moto_dev.arg)->vpos;
    moto_dev.cmd = MTDRV_TO_POS;
    moto_dev.state = MOTO_STATE_TOPOS;

    if(moto_dev.dev_param[MOTO_HDEV].curr_step < moto_dev.ins.pos.hpos)
    {
        moto_dev.dev_param[MOTO_HDEV].curr_dir = MOTO_CW;
        if(moto_dev.ins.pos.hpos > (H_MAX_PLULSE/ONE_STEP_PULSE))
        {
            hsteps = (H_MAX_PLULSE/ONE_STEP_PULSE) - moto_dev.dev_param[MOTO_HDEV].curr_step;
        }
        else
        {
            hsteps = moto_dev.ins.pos.hpos - moto_dev.dev_param[MOTO_HDEV].curr_step;
        }
    }
    else
    {
        moto_dev.dev_param[MOTO_HDEV].curr_dir = MOTO_CCW;
        if(moto_dev.ins.pos.hpos < 0)
        {
            hsteps = moto_dev.dev_param[MOTO_HDEV].curr_step;
        }
        else
        {
            hsteps = moto_dev.dev_param[MOTO_HDEV].curr_step - moto_dev.ins.pos.hpos;
        }
    }

    if(moto_dev.dev_param[MOTO_VDEV].curr_step < moto_dev.ins.pos.vpos)
    {
        moto_dev.dev_param[MOTO_VDEV].curr_dir = MOTO_CW;
        if(moto_dev.ins.pos.vpos > (V_MAX_PLULSE/ONE_STEP_PULSE))
        {
            vsteps = (V_MAX_PLULSE/ONE_STEP_PULSE) - moto_dev.dev_param[MOTO_VDEV].curr_step;
        }
        else
        {
            vsteps = moto_dev.ins.pos.vpos - moto_dev.dev_param[MOTO_VDEV].curr_step;
        }
    }
    else
    {
        moto_dev.dev_param[MOTO_VDEV].curr_dir = MOTO_CCW;
        if(moto_dev.ins.pos.vpos < 0)
        {
            vsteps = moto_dev.dev_param[MOTO_VDEV].curr_step;
        }
        else
        {
            vsteps = moto_dev.dev_param[MOTO_VDEV].curr_step - moto_dev.ins.pos.vpos;
        }
    }

    nsteps = (hsteps>vsteps)?hsteps:vsteps;

    moto_dev.pulse_num = nsteps*ONE_STEP_PULSE;
    moto_dev.pulse_cnt = 0;

    moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_HDEV].default_step = H_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_HDEV].is_running = 1;

    moto_dev.dev_param[MOTO_VDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_VDEV].curr_step = -1;
    //moto_dev.dev_param[MOTO_VDEV].default_step = V_MOTO_DEFAULT_STEP;
    moto_dev.dev_param[MOTO_VDEV].is_running = 1;

    TIMER3_ENABLE;
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void topos_run(void)
{
    struct moto_step_drv *pmsd;

    MOTO_DRV_LOG("%s entry\n",__func__);
    if(moto_dev.pulse_num == moto_dev.pulse_cnt)
    {
        /* moto action stop */
	MOTO_DRV_LOG("%s stop\n",__func__);
	moto_action_stop();
	return;
    }

    if(moto_dev.dev_param[MOTO_HDEV].curr_step != moto_dev.ins.pos.hpos)
    {
        pmsd = moto_dev.dev_param[MOTO_HDEV].ordertab[moto_dev.dev_param[MOTO_HDEV].curr_dir];
        pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
        HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;
    }

    if(moto_dev.dev_param[MOTO_VDEV].curr_step != moto_dev.ins.pos.vpos)
    {
        pmsd = moto_dev.dev_param[MOTO_VDEV].ordertab[moto_dev.dev_param[MOTO_VDEV].curr_dir];
        pmsd += moto_dev.pulse_cnt%ONE_STEP_PULSE;
        HW_REG(IO_ADDRESS(GPIO_9_BASE + pmsd->regoff)) = pmsd->portval;
    }

    moto_dev.pulse_cnt++;
    if(0 == moto_dev.pulse_cnt%ONE_STEP_PULSE)
    {
        if(MOTO_CW == moto_dev.dev_param[MOTO_HDEV].curr_dir)
        {
            if(moto_dev.dev_param[MOTO_HDEV].curr_step != moto_dev.ins.pos.hpos)
            {
                moto_dev.dev_param[MOTO_HDEV].curr_step++;
            }
            if(moto_dev.dev_param[MOTO_HDEV].curr_step > (H_MAX_PLULSE/ONE_STEP_PULSE))
            {
                moto_dev.dev_param[MOTO_HDEV].curr_step = (H_MAX_PLULSE/ONE_STEP_PULSE);
            }
        }
        else if(MOTO_CCW == moto_dev.dev_param[MOTO_HDEV].curr_dir)
        {
            if(moto_dev.dev_param[MOTO_HDEV].curr_step != moto_dev.ins.pos.hpos)
            {
                moto_dev.dev_param[MOTO_HDEV].curr_step--;
            }
            if(0 > moto_dev.dev_param[MOTO_HDEV].curr_step)
            {
                moto_dev.dev_param[MOTO_HDEV].curr_step = 0;
            }
        }

        if(MOTO_CW == moto_dev.dev_param[MOTO_VDEV].curr_dir)
        {
            if(moto_dev.dev_param[MOTO_VDEV].curr_step != moto_dev.ins.pos.vpos)
            {
                moto_dev.dev_param[MOTO_VDEV].curr_step++;
            }
            if(moto_dev.dev_param[MOTO_VDEV].curr_step > (V_MAX_PLULSE/ONE_STEP_PULSE))
            {
                moto_dev.dev_param[MOTO_VDEV].curr_step = (V_MAX_PLULSE/ONE_STEP_PULSE);
            }
        }
        else if(MOTO_CCW == moto_dev.dev_param[MOTO_VDEV].curr_dir)
        {
            if(moto_dev.dev_param[MOTO_VDEV].curr_step != moto_dev.ins.pos.vpos)
            {
                moto_dev.dev_param[MOTO_VDEV].curr_step--;
            }
            if(0 > moto_dev.dev_param[MOTO_VDEV].curr_step)
            {
                moto_dev.dev_param[MOTO_VDEV].curr_step = 0;
            }
        }
    }
    MOTO_DRV_LOG("%s exit\n",__func__);
}
void topos_stop(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    TIMER3_DISABLE;

    moto_dev.cmd = MTDRV_NONE;
    moto_dev.state = MOTO_STATE_IDLE;
    moto_dev.pulse_num = -1;
    moto_dev.pulse_cnt = 0;

    //moto_dev.dev_param[MOTO_HDEV].speed = MOTO_SPD_LV5;
    //moto_dev.dev_param[MOTO_HDEV].curr_step = moto_dev.dev_param[MOTO_HDEV].default_step;
    //moto_dev.dev_param[MOTO_HDEV].default_step = -1;
    moto_dev.dev_param[MOTO_HDEV].is_running = 0;
    moto_dev.dev_param[MOTO_VDEV].is_running = 0;
    moto_dev.to_idle = 0;
    MOTO_GPIO_DEFAULT;
    MOTO_DRV_LOG("%s exit\n",__func__);
}

long moto_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    //struct moto_drv_action *pmda = (struct moto_drv_action *)arg;
	
    MOTO_DRV_LOG("%s entry\n",__func__);
	switch(cmd)
	{
		case MTDRV_SELF_TEST:
		case MTDRV_HCCW:
		case MTDRV_HCW:
		case MTDRV_VCCW:
		case MTDRV_VCW:
		case MTDRV_HCRUISING:
		case MTDRV_VCRUISING:
		case MTDRV_HVCRUISING:
		case MTDRV_TO_POS:
		case MTDRV_HCCWB:
		case MTDRV_HCWB:
		case MTDRV_VCCWB:
		case MTDRV_VCWB:
			/* moto action start */
			MOTO_DRV_LOG("%s action 11111\n",__func__);
			if(moto_dev.state != MOTO_STATE_IDLE) return -1;
			MOTO_DRV_LOG("%s action 22222\n",__func__);
			moto_action_start(cmd, arg);
			break;

		case MTDRV_GET_STATE:
			MOTO_DRV_LOG("%s MTDRV_GET_STATE\n",__func__);
			*(unsigned int *)arg = (unsigned int)moto_dev.state;
			break;

		case MTDRV_GET_POS:
			MOTO_DRV_LOG("%s MTDRV_GET_POS\n",__func__);
			((struct moto_pos *)arg)->hpos = moto_dev.dev_param[MOTO_HDEV].curr_step;
			((struct moto_pos *)arg)->vpos = moto_dev.dev_param[MOTO_VDEV].curr_step;
			break;

		case MTDRV_STOP:
			MOTO_DRV_LOG("%s MTDRV_STOP\n",__func__);
			if(moto_dev.state == MOTO_STATE_IDLE) return -1;
			moto_dev.to_idle = 1;
			break;

		case MTDRV_NONE:
			moto_action_none(arg);
			break;

		default:
			return -1;
	}
    MOTO_DRV_LOG("%s exit\n",__func__);
    return 0;
}

static atomic_t moto_s_available = ATOMIC_INIT(1);
int moto_open(struct inode * inode, struct file * file)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
#if 0
    if(! atomic_dec_and_test(&moto_s_available) )
    {
        atomic_inc(&moto_s_available);
        printk("%s Error: device already open.\n",__func__);
        return -EBUSY;
    }
#endif
    MOTO_DRV_LOG("%s exit\n",__func__);
    return 0;
}
int moto_close(struct inode * inode, struct file * file)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
#if 0
    TIMER3_DISABLE;
    atomic_inc(&moto_s_available);
#endif
    MOTO_DRV_LOG("%s exit\n",__func__);
    return 0;
}


static struct file_operations moto_fops = {
    .owner      = THIS_MODULE,
    //.ioctl      = moto_ioctl,
    .unlocked_ioctl = moto_ioctl,
    .open       = moto_open,
    .release    = moto_close
};

static struct miscdevice moto_miscdev = {
   .minor		= MISC_DYNAMIC_MINOR,
   .name		= MOTO_DEVICE_NAME,
   .fops  = &moto_fops,
};

static int __init moto_drv_init(void)
{
    int ret;
    unsigned char regvalue;
    unsigned int val;
    int i;

    MOTO_DRV_LOG("%s entry\n",__func__);
    
    ret = misc_register(&moto_miscdev);
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
    MOTO_GPIO_DEFAULT;

    ret = request_irq(MOTO_DEVICE_IRQ_NO, moto_interrupt, 0, MOTO_DEVICE_NAME, &moto_interrupt);
    if(ret)
    {
        printk( "%s Error: request IRQ(%d) failed\n", __func__, MOTO_DEVICE_IRQ_NO);
        misc_deregister(&moto_miscdev);
        goto moto_init_failed;
    }

    moto_dev.state = MOTO_STATE_IDLE;

    moto_dev.dev_param[MOTO_HDEV].ordertab[MOTO_CW]= &h_cwtab[0];
    moto_dev.dev_param[MOTO_HDEV].ordertab[MOTO_CCW]= &h_ccwtab[0];
    //memset(&moto_dev.dev_param[MOTO_HDEV].preset_pos[0], -1, sizeof(struct moto_pos)*PRESET_POS_NUM);

    moto_dev.dev_param[MOTO_VDEV].ordertab[MOTO_CW]= &v_cwtab[0];
    moto_dev.dev_param[MOTO_VDEV].ordertab[MOTO_CCW]= &v_ccwtab[0];
    //memset(&moto_dev.dev_param[MOTO_VDEV].preset_pos[0], -1, sizeof(struct moto_pos)*PRESET_POS_NUM);

    /*
      * perfect 1:TIMER3_LOAD_REG=0x000035B6 TIMER3_BGLOAD_REG=0x000035B6, Bus Clock, 4 prescale, ONE_STEP_PULSE=4, max steps=2592, horizontal
      * perfect 2:TIMER3_LOAD_REG=0x00001ADB TIMER3_BGLOAD_REG=0x00001ADB, Bus Clock, 4 prescale, ONE_STEP_PULSE=8, max steps=5184, horizontal
      * perfect 3:TIMER3_LOAD_REG=0x00001ADB TIMER3_BGLOAD_REG=0x00001ADB, Bus Clock, 4 prescale, ONE_STEP_PULSE=8, max steps=1712, vertical
	*/
    HW_REG(TIMER3_LOAD_REG) = 0x00001ADB;
    HW_REG(TIMER3_BGLOAD_REG) = 0x00001ADB;

    val = HW_REG(SC_LOCKEN_REG);
    if(val)
    {
        /* locked */
        HW_REG(SC_LOCKEN_REG) = 0x1ACCE551;
        HW_REG(SC_CTRL_REG) |= (1<<22);
        HW_REG(SC_LOCKEN_REG) = 0;
    }
    else
    {
        /* unlocked */
        HW_REG(SC_CTRL_REG) |= (1<<22);
    }

    HW_REG(TIMER3_CONTROL_REG) = (1<<6)|(1<<5)|(1<<1)|(0x01<<2);

    MOTO_DRV_LOG("%s init ok. ver=%s, %s.\n", __func__, __DATE__, __TIME__);

moto_init_failed:

    MOTO_DRV_LOG("%s exit\n",__func__);
    return 0;    
}

static void __exit moto_drv_exit(void)
{
    MOTO_DRV_LOG("%s entry\n",__func__);
    free_irq(MOTO_DEVICE_IRQ_NO, &moto_interrupt);
    misc_deregister(&moto_miscdev);
    memset(&moto_dev,0,sizeof(moto_dev_struct));
    MOTO_DRV_LOG("%s exit\n",__func__);
}


module_init(moto_drv_init);
module_exit(moto_drv_exit);

#ifdef MODULE
//#include <linux/compile.h>
#endif
//MODULE_INFO(build, UTS_VERSION);
MODULE_LICENSE("GPL");




