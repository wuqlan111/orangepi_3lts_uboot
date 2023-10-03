
#ifndef _ARCH_ASM_MACH_CLK_H
#define _ARCH_ASM_MACH_CLK_H

#include  <stdint.h>

#define   CCU_BASE_ADDR     (0x03001000u)

#define  CCU_PLL(x)        CCU_PLL_##x
#define  CCU_CLK(x)        CCU_CLK_##x

enum {
    CCU_PLL(CPUX)  = 0,
    CCU_PLL(DDR0),
    CCU_PLL(PER0),
    CCU_PLL(PER1),
    CCU_PLL(GPU),
    CCU_PLL(VIDEO0),
    CCU_PLL(VIDEO1),
    CCU_PLL(VE),
    CCU_PLL(DE),
    CCU_PLL(HSIC),
    CCU_PLL(AUDIO),
    CCU_PLL_MAX  =  CCU_PLL(AUDIO),
};


enum  {
    CCU_CLK(AXI) =  0,
    CCU_CLK(PSI_AHB1),
    CCU_CLK(AHB3),
    CCU_CLK(APB1),
    CCU_CLK(APB2),
    CCU_CLK_MAX  =   CCU_CLK(APB2),
};











#endif



