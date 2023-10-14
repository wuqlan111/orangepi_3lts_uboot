#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/cpu.h>
#include <linux/delay.h>
#include <log.h>


int32_t  ccu_get_pll_perix_clk(const uint32_t is_peri0, uint64_t * rate)
{
	const uint32_t addr = is_peri0? CCU_PERIX_PLL_REG(0): CCU_PERIX_PLL_REG(1);
	const uint32_t flag  =  readl(addr);

	if (!rate || !(flag & CCU_PLL_CLK_ENABLE)) {
		return  -1;
	}

	uint32_t pll_n   =  (flag & CCU_PLL_FACTOR_N) >> 8;
	uint32_t pll_m0  =  flag & BIT(0);
	uint32_t pll_m1  =  (flag & BIT(1)) >> 1;
	uint32_t tmp_m  =  (pll_m0 + 1) * (pll_m1);

	*rate  =  CCU_PLL_SRC_RATE * (pll_n + 1) / tmp_m;

	_DBG_PRINTF("%s rate -- %lu Hz\n",  *rate);

	return   0;

}


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

	while ( (readl(addr) & mask) !=  val_mask) {

		if (timeout_vld && !timeout) {
			ret  =  -1;
			break;
		}

		if (timeout_vld) {
			timeout--;
		}

	}

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


int32_t  mmc_clk_init(const uint32_t smhc,  const uint32_t clk)
{
	int32_t  ret =  0;
	const  uint32_t reg  =  CCU_SMHCX_CLK_REG(smhc);
	if (smhc > CCU_SMHCX_MAX_ID ) {
		_DBG_PRINTF("smhc [%u] invalid!\n", smhc);
		return  -1;
	}

	setbits_32(CCU_SMHCX_GATE_REG,  BIT(smhc));
	setbits_32(CCU_SMHCX_GATE_REG,  BIT(smhc+16));
	clrbits_32(reg,  BIT(31));

	uint64_t  peri0_4x,   peri1_4x, peri0_2x,  peri1_2x;
	uint8_t   peri0_vld,  peri1_vld;

	peri0_4x  =  peri0_2x =  peri1_4x = peri1_2x = 0;
	peri0_vld  =  peri1_vld  =  0;

	if (!ccu_get_pll_perix_clk(0, &peri0_4x)) {
		peri0_vld =  1;
		peri0_2x  =  peri0_4x >> 1;
	}

	if (!ccu_get_pll_perix_clk(0, &peri1_4x)) {
		peri0_vld =  1;
		peri1_2x  =  peri1_4x >> 1;
	}

	const uint8_t  src_vld[3]  = { 1, peri0_vld,  peri1_vld };
	const uint64_t src_rate[3] = { CCU_PLL_SRC_RATE,   peri0_2x,  peri1_2x  };

	uint8_t  best_idx, best_n, best_m;
	uint64_t  delta  =  clk;

	for (int32_t tmp_idx =  0; (tmp_idx < 3) && (src_vld[tmp_idx]); tmp_idx++) {
		for (int32_t tmp_n =  0; tmp_n < 4; tmp_n++) {
			uint32_t factor_n  =  1 << tmp_n;
			for (int32_t tmp_m =  0; tmp_m < 16; tmp_m++) {
				uint64_t tmp_rate = src_rate[tmp_idx] / (factor_n * (tmp_m + 1));
				uint64_t tmp_delta = tmp_rate > clk ? tmp_rate - clk: clk - tmp_rate;

				if (tmp_delta < delta) {
					best_idx = tmp_idx;
					best_n   =  tmp_n;
					best_m   =  tmp_m;
					delta  =  tmp_delta;
				}

				if (tmp_delta == 0) {
					break;
				}

			}
		}
	}

	writel( BIT(31) | (best_idx << 24) | (best_n << 8) | best_m, reg );

	_DBG_PRINTF("smhc%u ccu -- 0x%08x\n", smhc, readl(reg));
	_DBG_PRINTF("src_rate -- %lu\n",  src_rate[best_idx]);

	return  0;

}


