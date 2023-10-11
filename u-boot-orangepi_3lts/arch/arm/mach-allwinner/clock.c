#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <linux/delay.h>
#include <log.h>



int32_t  uart_clk_init(const uint32_t uart_id)
{
    if (uart_id > CCU_UART3_ID) {
        return  -1;
    }

    /*open uartx clock*/
    setbits_32(CCU_UARTX_GATE_REG,  (1 << uart_id));

    /*de-assert reset */
    setbits_32(CCU_UARTX_GATE_REG,  (1 << (uart_id + 16)));
    return   0;

}


int32_t  wait_reg32_flag(uint32_t addr, uint32_t mask, uint32_t val, uint32_t timeout)
{
	int32_t  ret  =  0;
	const uint32_t timeout_vld  =  timeout ? 1:  0;
	const uint32_t val_mask  =  mask & val;

	do {

		if (readl(addr) & mask ==  val_mask) {
			break;
		} 

		if (timeout_vld && !timeout) {
			ret  =  -1;
			break;
		}

		if (timeout_vld) {
			timeout--;
		}

	} while (1);

	return  ret;

}



int32_t  dram_clk_init(const uint32_t clock)
{
    /* Put all DRAM-related blocks to reset state */
	clrbits_32(CCU_MBUS_CLK_REG, BIT(31) | BIT(30));
	clrbits_32(CCU_DRAM_GATE_REG, BIT(0));

	udelay(5);

	/*mask dram clk and disable ddr pll*/
	writel(0, CCU_DRAM_GATE_REG);
	clrbits_32(CCU_DRAM_PLL_REG, BIT(31));
	clrbits_32(CCU_DRAM_PLL_REG, BIT(29));
	wait_reg32_flag(CCU_DRAM_PLL_REG,  BIT(28), 0, 0);
	clrbits_32(CCU_DRAM_CLK_REG, BIT(16));

	udelay(5);

	/* Set PLL5 rate to doubled DRAM clock rate */
	uint32_t pll_n = (clock  / 12 ) -1;
	writel(BIT(31) | BIT(29) |
	       (pll_n << 8), CCU_DRAM_PLL_REG);
	wait_reg32_flag(CCU_DRAM_PLL_REG,  BIT(28), BIT(28), 0);

	
	/* Configure DRAM mod clock */
	writel(0, CCU_DRAM_CLK_REG);
	setbits_32(CCU_DRAM_CLK_REG, BIT(27));
	writel(BIT(16), CCU_DRAM_GATE_REG);
	udelay(5);
	setbits_32(CCU_DRAM_GATE_REG, BIT(0));

	return  0;

}





