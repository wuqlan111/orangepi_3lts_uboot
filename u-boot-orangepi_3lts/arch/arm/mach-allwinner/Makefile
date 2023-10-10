
obj-y   +=  clock.o
obj-y   +=  pinmux.o
obj-y   +=  sdram.o
obj-y   +=  board.o
obj-y   +=  cpu_info.o

ifneq ($(CONFIG_SPL_BUILD),)
obj-y	+=  spl.o
obj-$(CONFIG_ALLWINNER_DRAM_DW)	+= dram_timings/
endif


