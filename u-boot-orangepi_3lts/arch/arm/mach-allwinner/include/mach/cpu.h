
#ifndef _ALLWINNER_H6_V200_CPU_H
#define _ALLWINNER_H6_V200_CPU_H

#include <stdint.h>

#define ALLWINNER_H6_CCU_BASE          (0x03001000u)


#define  ALLWINNER_H6_SIDC_BASE			(0x03006000u)
#define  ALLWINNER_H6_SID_BASE			(0x03006200u)




#define  ALLWINNER_H6_DRAM_COM_BASE		    (0x04002000u)
#define  ALLWINNER_H6_DRAM_CTL0_BASE		(0x04003000u)
#define  ALLWINNER_H6_DRAM_PHY0_BASE		(0x04005000u)

#define  ALLWINNER_H6_SMHCX_BASE            (0x04020000u)



int32_t  wait_reg32_flag(uint32_t addr, uint32_t mask, uint32_t val, uint32_t timeout);

#endif
