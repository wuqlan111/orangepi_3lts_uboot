
#ifndef _ARCH_ASM_MACH_CLK_H
#define _ARCH_ASM_MACH_CLK_H

#include  <stdint.h>
#include  <linux/bitops.h>
#include  <asm/arch/cpu.h>


/*ccu apbx clk cfg*/
#define  CCU_APBX_CFG(n)    (ALLWINNER_H6_CCU_BASE + 0x520 + (n) * 4)


#define CCU_DRAM_PLL_REG      (ALLWINNER_H6_CCU_BASE + 0x10)
#define CCU_DRAM_CLK_REG      (ALLWINNER_H6_CCU_BASE + 0x800)
#define CCU_DRAM_GATE_REG     (ALLWINNER_H6_CCU_BASE + 0x80c)


#define CCU_MBUS_CLK_REG      (ALLWINNER_H6_CCU_BASE + 0x540)
#define MBUS_ENABLE			  BIT(31)
#define MBUS_RESET			  BIT(30)
#define MBUS_CLK_SRC_MASK		GENMASK(25, 24)
#define MBUS_CLK_SRC_OSCM24		(0u)
#define MBUS_CLK_SRC_PLL6X2		(1 << 24u)
#define MBUS_CLK_SRC_PLL5		(2 << 24u)
#define MBUS_CLK_SRC_PLL6X4		(3 << 24u)
#define MBUS_CLK_M(m)			(((m)-1) << 0)



/*uart clk gate reg*/
#define  CCU_UARTX_GATE_REG       (ALLWINNER_H6_CCU_BASE + 0x90c)
#define  CCU_UART0_ID              0
#define  CCU_UART1_ID              1
#define  CCU_UART2_ID              2
#define  CCU_UART3_ID              3

int32_t  uart_clk_init(const uint32_t uart_id);

int32_t  dram_clk_init(const uint32_t clock);


#endif



