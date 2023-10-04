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




/* UART */
#define UART0_BASE             (0x05000000u)
#define UART1_BASE             (0x05000400u)
#define UART2_BASE             (0x05000800u)
#define UART3_BASE             (0x05000c00u)
#define R_UART0_BASE           (0x07080000u)
#define CFG_MXC_UART_BASE		UART0_BASE


#endif

