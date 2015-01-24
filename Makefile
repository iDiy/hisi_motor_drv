
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
			
obj-m := moto_drv.o
#moto_drv-y += moto_drv.o

EXTRA_CFLAGS += -D$(HI_FPGA) 
 
default:	
	@$(CC) -g -Wall -o moto_test moto_test.c
	@make -C $(LINUX_ROOT) M=$(PWD) modules
clean:
	@rm *.o moto_test -rf
	@make -C $(LINUX_ROOT) M=$(PWD) clean
