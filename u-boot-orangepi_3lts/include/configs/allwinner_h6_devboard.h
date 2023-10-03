#ifndef __CONFIG_H
#define __CONFIG_H


// H6-v200 sdram space (0x40000000 - 0xffffffff)
#define CONFIG_SYS_SDRAM_BASE		(0x40000000u)

/*
 * spl use sram A1 (0x20000 - 0x27fff) 32KB
 * 
 * */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_H6_V200_SRAM_ADDRESS
#define CONFIG_SYS_INIT_RAM_SIZE	0x8000 /* 32 KB */





#endif

