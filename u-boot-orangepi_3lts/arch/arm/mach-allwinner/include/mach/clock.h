
#ifndef _ARCH_ASM_MACH_CLK_H
#define _ARCH_ASM_MACH_CLK_H

#include  <stdint.h>

#define   CCU_BASE_ADDR     (0x03001000u)

/*ccu apbx clk cfg*/
#define  CCU_APBX_CFG(n)    (CCU_BASE_ADDR + 0x520 + (n) * 4)




/*uart clk gate reg*/
#define  CCU_UARTX_GATE_REG       (CCU_BASE_ADDR + 0x90c)
#define  CCU_UART0_ID              0
#define  CCU_UART1_ID              1
#define  CCU_UART2_ID              2
#define  CCU_UART3_ID              3






#endif



