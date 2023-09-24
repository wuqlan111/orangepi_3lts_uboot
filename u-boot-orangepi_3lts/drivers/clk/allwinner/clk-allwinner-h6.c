
#define  LOG_CATEGORY UCLASS_CLK

#include  <common.h>
#include  <clk-uclass.h>
#include  <dm.h>
#include  <log.h>
#include  <asm/global_data.h>
#include  <dm/device_compat.h>
#include  <dm/lists.h>
#include  <errno.h>
#include  <asm/io.h>

#include  "clk-allwinner-h6.h"

static  uint32_t  allwinner_clk_regs_offset[] = {  foreach_array_clk_ctrl };
static  uint32_t  allwinner_pll_m[] = { 1,  2,  4};

typedef  struct {  
    fdt_addr_t  base;
} allwinner_h6_clk_plat_t;


static  void  _allwinner_h6_pll_enable(u32 *  regs)
{
    setbits_32(regs,  ALLWINNER_H6_PLLX_ENABLE | ALLWINNER_H6_PLLX_LOCK_ENABLE);
    while (!(readl(regs) & ALLWINNER_H6_PLLX_LOCK))  ;

}


static  void  _allwinner_h6_pll_disable(u32 *  regs)
{
    clrbits_32(regs,  ALLWINNER_H6_PLLX_ENABLE | ALLWINNER_H6_PLLX_LOCK_ENABLE);
}


static  void  _allwinner_h6_pll_enable(u32 *  regs)
{
    setbits_32(regs,  ALLWINNER_H6_PLLX_ENABLE | ALLWINNER_H6_PLLX_LOCK_ENABLE);
    while (!(readl(regs) & ALLWINNER_H6_PLLX_LOCK))  ;

}


static  int32_t  _allwinner_h6_pll_set_rate(uint32_t clk_id,  ulong rate, uint32_t base)
{
    uint32_t  pll_m =  0;
    ulong  pll_n  =  0;

    if (rate == 0) {
        log_err("rate is zero!");
        return  -EINVAL;
    }

    for (int32_t  i  =  0;  i  <  ARRAY_SIZE(allwinner_pll_m); i++) {
        ulong  tmp_rate  =  allwinner_pll_m[i] * rate;
        double  divisor  =  tmp_rate  / 24000000;
        pll_n  =  (ulong)  divisor;
        if ((double)pll_n != divisor) {
            pll_n++;
        }

        if (pll_n < 0xff) {
            pll_m  =  allwinner_pll_m[i];
            break;
        }
    }

    if (pll_n  > 0xff) {
        log_err("rate -- [%ul] not support!", rate);
        return  -EINVAL;
    }

    uint32_t  flag,  mask;
    flag  =  pll_n << 8;
    mask  =  ALLWINNER_H6_PLLX_FACTOR_N;

    switch (clk_id) {
        case  ALLWINNER_PLL_CPUX:
            if (pll_m < 4) {
                flag  |=  ( pll_m - 1)  << 16;
            } else {
                flag  |=  0x2  <<  16;
            }

            mask  |=  0x3 <<  16;
            break;

        case  ALLWINNER_PLL_DDR0 ... ALLWINNER_PLL_GPU:
        case  ALLWINNER_PLL_VE ...  ALLWINNER_PLL_HSIC:
            flag  |=  (pll_m - 1);
            mask  |=  0x3;
            break;
        case  ALLWINNER_PLL_VIDEO0:
        case  ALLWINNER_PLL_VIDEO1: 
            if (pll_m > 2) {
                log_err("pll_m -- [%u], rate -- [%ul] invalid!", pll_m, rate);
                return  -EINVAL;
            }
            flag  |=  (pll_m - 1) << 1;
            mask  |=  0x2;
            break;
        
        default:
            log_err("clk_id -- %u, invalid pll clk id!", clk_id);
            return  -EINVAL;
    }

    uint32_t  reg_addr =  base + allwinner_clk_regs_offset[clk_id];

    clrbits_32(reg_addr,  ALLWINNER_H6_PLLX_LOCK_ENABLE);
    clrsetbits_32(reg_addr, mask,  flag);
    setbits_32(reg_addr,  ALLWINNER_H6_PLLX_LOCK_ENABLE);

    while (!(readl(reg_addr) & ALLWINNER_H6_PLLX_LOCK))  ;

    return  0;

}


static  int32_t  _allwinner_h6_pll_get_rate(uint32_t clk_id,  uint32_t base,  ulong * rate)
{

    uint32_t  reg_addr =  base + allwinner_clk_regs_offset[clk_id];
    uint32_t  pll_n, pll_m, tmp_m;

    assert(rate);

    pll_n  = pll_m  = tmp_m = 0;
    uint32_t  clk_ctrl = readl(reg_addr);
    pll_n  =  (clk_ctrl & ALLWINNER_H6_PLLX_FACTOR_N ) >>  8;

    switch (clk_id) {
        case  ALLWINNER_PLL_CPUX:
            tmp_m  =  (clk_ctrl & GENMASK(17,  16)) >> 16;
            pll_m  =  tmp_m  ==  2 ? 4: tmp_m + 1;
            break;

        case  ALLWINNER_PLL_DDR0 ... ALLWINNER_PLL_GPU:
        case  ALLWINNER_PLL_VE ...  ALLWINNER_PLL_HSIC:
            tmp_m  =  clk_ctrl & 0x3;
            pll_m =  (tmp_m ==  0)  || (tmp_m == 3) ? tmp_m + 1: 2;
            break;
        case  ALLWINNER_PLL_VIDEO0:
        case  ALLWINNER_PLL_VIDEO1: 
            pll_m  =  clk_ctrl &  BIT(1) ? 2: 1;
            break;
        
        default:
            log_err("clk_id -- %u, invalid pll clk id!", clk_id);
            return  -EINVAL;
    }

    *rate  =  ( 24000000 * pll_n) / pll_m;

    return  0;

}

static int32_t _allweinner_clk_psi_get_rate(uint32_t base,  double * rate)
{
    assert(rate != NULL);
    *rate  =   0;

    uint32_t  reg_addr  =  base + allwinner_clk_regs_offset[ALLWINNER_CLK_PSI_AHB1_AHB2];
    uint32_t  clk_ctrl = readl(reg_addr);
    uint32_t  clk_src  =  (clk_ctrl & GENMASK(25, 24)) >> 24;
    ulong  src_rate  = 0;
    if (clk_src == 0) {
        src_rate  = 24000000;
    } else if (clk_src == 1) {
        src_rate  =  32768;
    } else if (clk_src  ==  2) {
        src_rate  =  16000000;
    } else {
        ret = _allwinner_h6_pll_get_rate(ALLWINNER_PLL_PERI0, base, &src_rate);
        if (ret) {
            log_err("get pll_peri0 clk failed!\n");
            return  ret;
        }
    }

    uint32_t  clk_n  =  (clk_ctrl & GENMASK(9,  8)) >> 8;
    uint32_t  clk_m  =  (clk_ctrl & GENMASK(1, 0))  +  1;
    uint32_t  factor =  (1 << clk_n) * clk_m;
    
    *rate  =  (double)src_rate / factor;

    return  0;

}


static int32_t _allweinner_clk_psi_set_rate(uint32_t base,  ulong rate)
{
    assert(rate != NULL);
    *rate  =   0;

    uint32_t  reg_addr  =  base + allwinner_clk_regs_offset[ALLWINNER_CLK_PSI_AHB1_AHB2];
    uint32_t  clk_ctrl = readl(reg_addr);
    uint32_t  clk_src  =  (clk_ctrl & GENMASK(25, 24)) >> 24;
    ulong  src_rate  = 0;
    if (clk_src == 0) {
        src_rate  = 24000000;
    } else if (clk_src == 1) {
        src_rate  =  32768;
    } else if (clk_src  ==  2) {
        src_rate  =  16000000;
    } else {
        ret = _allwinner_h6_pll_get_rate(ALLWINNER_PLL_PERI0, base, &src_rate);
        if (ret) {
            log_err("get pll_peri0 clk failed!\n");
            return  ret;
        }
    }

    if ( (rate < (src_rate >> 5)) || (rate > src_rate) ) {
        log_err("src rate [%ul] not support target rate [%ul]!", src_rate, rate);
        return   -EINVAL;
    }

    double divisor  =  (double)src_rate / rate;
    double  delta  =  64;
    uint32_t  clk_n,  clk_m, best_m, best_n;
    clk_n  = clk_m = best_m = best_n  =  0;

    for (clk_m =  0; clk_m < 4; clk_m++) 
        for (clk_n =  0; clk_n < 4; clk_n++) {
            double  tmp_factor  =  (clk_m + 1) * (1 << clk_n);
            if (tmp_factor < divisor) {
                continue;
            }

            double  tmp_delta  =  tmp_factor - divisor;

            if (tmp_delta  < delta) {
                best_m  =  clk_m;
                best_n  =  clk_n;
                delta  =  tmp_delta;
            }

            if (tmp_delta == 0) {
                break;
            }
                
        }


    uint32_t  flag  =  ( best_n << 8 ) | best_m;
    uint32_t  mask  =   GENMASK(9,  8) | GENMASK(1,  0);

    clk_ctrl  &=  ~mask;
    clk_ctrl  |=  ( flag & mask) ;

    writel(clk_ctrl,  reg_addr);

    return  0;

}



static int32_t _allweinner_clk_ahbx_get_rate(uint32_t is_ahb3, uint32_t base, ulong * rate)
{
    int32_t  ret  =  0;
    uint32_t  reg_offset  =  is_ahb3 ? ALLWINNER_CLK_AHB3: ALLWINNER_CLK_PSI_AHB1_AHB2;
    uint32_t  reg_addr   =  base + allwinner_clk_regs_offset[reg_offset];

    uint32_t  clk_ctrl  =  readl(reg_addr);
    uint32_t  clk_src  =  (clk_ctrl & GENMASK(25, 24)) >> 24;
    double src_rate  = 0;

    if (!is_ahb3) {
        ret  =   _allweinner_clk_psi_get_rate(base,  &src_rate);
        *rate  =  (ulong)src_rate;
        return  ret;
    }

    if (clk_src == 0) {
        src_rate  = 24000000;
    } else if (clk_src == 1) {
        src_rate  =  32768;
    } else if (clk_src ==  3) {
        ret  =  _allwinner_h6_pll_get_rate(ALLWINNER_PLL_PERI0, base, &src_rate);
    } else {
        ret  =  _allweinner_clk_psi_get_rate(base,  &src_rate);
    }
    
    if (ret) {
        return  ret;
    }

    uint32_t  clk_n  =  (clk_ctrl & GENMASK(9,  8)) >> 8;
    uint32_t  clk_m  =  (clk_ctrl & GENMASK(1, 0))  +  1;
    uint32_t  factor =  (1 << clk_n) * clk_m;

    *rate  =  src_rate  /  factor;

    return  0;

}


static int32_t _allweinner_clk_ahbx_set_rate(ulong rate, uint32_t is_ahb3, uint32_t base)
{
    int32_t  ret  =  0;
    uint32_t  reg_offset  =  is_ahb3 ? ALLWINNER_CLK_AHB3: ALLWINNER_CLK_PSI_AHB1_AHB2;
    uint32_t  reg_addr   =  base + allwinner_clk_regs_offset[reg_offset];

    uint32_t  clk_ctrl  =  readl(reg_addr);
    uint32_t  clk_src  =  (clk_ctrl & GENMASK(25, 24)) >> 24;
    double  src_rate  = 0;

    if (!is_ahb3) {
        ret  =  _allweinner_clk_psi_set_rate(base,  rate);
        return  ret;
    }

    if (clk_src == 0) {
        src_rate  = 24000000;
    } else if (clk_src == 1) {
        src_rate  =  32768;
    } else if (clk_src ==  3) {
        ulong tmp_src_rate  =  0;
        ret = _allwinner_h6_pll_get_rate(ALLWINNER_PLL_PERI0, base, &tmp_src_rate);
        src_rate  =  tmp_src_rate;
    } else {
        ret  =  _allweinner_clk_psi_get_rate(base, &src_rate);
    }
    
    if (ret) {
        return  ret;
    }

    if ( (rate < (src_rate >> 5)) || (rate > src_rate) ) {
        log_err("src rate [%ul] not support target rate [%ul]!", src_rate, rate);
        return   -EINVAL;
    }

    double divisor  =  (double)src_rate / rate;
    double  delta  =  64;
    uint32_t  clk_n,  clk_m, best_m, best_n;
    clk_n  = clk_m = best_m = best_n  =  0;

    for (clk_m =  0; clk_m < 4; clk_m++) 
        for (clk_n =  0; clk_n < 4; clk_n++) {
            double  tmp_factor  =  (clk_m + 1) * (1 << clk_n);
            if (tmp_factor < divisor) {
                continue;
            }

            double  tmp_delta  =  tmp_factor - divisor;

            if (tmp_delta  < delta) {
                best_m  =  clk_m;
                best_n  =  clk_n;
                delta  =  tmp_delta;
            }

            if (tmp_delta == 0) {
                break;
            }
                
        }


    uint32_t  flag  =  ( best_n << 8 ) | best_m;
    uint32_t  mask  =   GENMASK(9,  8) | GENMASK(1,  0);

    clk_ctrl  &=  ~mask;
    clk_ctrl  |=  ( flag & mask) ;

    writel(clk_ctrl,  reg_addr);

    return  0;

}



static int32_t _allweinner_clk_ahbx_get_rate(uint32_t is_ahb3, uint32_t base, ulong * rate)
{
    int32_t  ret  =  0;
    uint32_t  reg_offset  =  is_ahb3 ? ALLWINNER_CLK_AHB3: ALLWINNER_CLK_PSI_AHB1_AHB2;
    uint32_t  reg_addr   =  base + allwinner_clk_regs_offset[reg_offset];

    uint32_t  clk_ctrl  =  readl(reg_addr);
    uint32_t  clk_src  =  (clk_ctrl & GENMASK(25, 24)) >> 24;
    double src_rate  = 0;

    if (!is_ahb3) {
        ret  =   _allweinner_clk_psi_get_rate(base,  &src_rate);
        *rate  =  (ulong)src_rate;
        return  ret;
    }

    if (clk_src == 0) {
        src_rate  = 24000000;
    } else if (clk_src == 1) {
        src_rate  =  32768;
    } else if (clk_src ==  3) {
        ret  =  _allwinner_h6_pll_get_rate(ALLWINNER_PLL_PERI0, base, &src_rate);
    } else {
        ret  =  _allweinner_clk_psi_get_rate(base,  &src_rate);
    }
    
    if (ret) {
        return  ret;
    }

    uint32_t  clk_n  =  (clk_ctrl & GENMASK(9,  8)) >> 8;
    uint32_t  clk_m  =  (clk_ctrl & GENMASK(1, 0))  +  1;
    uint32_t  factor =  (1 << clk_n) * clk_m;

    *rate  =  src_rate  /  factor;

    return  0;

}



static  int32_t  _allwinner_h6_clk_set_rate(uint32_t clk_id,  ulong rate, uint32_t base)
{
    uint32_t  clk_m =  0;
    ulong  pll_n  =  0;

    if (rate == 0) {
        log_err("rate is zero!");
        return  -EINVAL;
    }



    if (pll_n  > 0xff) {
        log_err("rate -- [%ul] not support!", rate);
        return  -EINVAL;
    }

    uint32_t  flag,  mask;
    flag  =  pll_n << 8;
    mask  =  ALLWINNER_H6_PLLX_FACTOR_N;

    switch (clk_id) {
        case  ALLWINNER_PLL_CPUX:
            if (pll_m < 4) {
                flag  |=  ( pll_m - 1)  << 16;
            } else {
                flag  |=  0x2  <<  16;
            }

            mask  |=  0x3 <<  16;
            break;

        case  ALLWINNER_PLL_DDR0 ... ALLWINNER_PLL_GPU:
        case  ALLWINNER_PLL_VE ...  ALLWINNER_PLL_HSIC:
            flag  |=  (pll_m - 1);
            mask  |=  0x3;
            break;
        case  ALLWINNER_PLL_VIDEO0:
        case  ALLWINNER_PLL_VIDEO1: 
            if (pll_m > 2) {
                log_err("pll_m -- [%u], rate -- [%ul] invalid!", pll_m, rate);
                return  -EINVAL;
            }
            flag  |=  (pll_m - 1) << 1;
            mask  |=  0x2;
            break;
        
        default:
            log_err("clk_id -- %u, invalid pll clk id!", clk_id);
            return  -EINVAL;
    }

    uint32_t  reg_addr =  base + allwinner_clk_regs_offset[clk_id];

    clrbits_32(reg_addr,  ALLWINNER_H6_PLLX_LOCK_ENABLE);
    clrsetbits_32(reg_addr, mask,  flag);
    setbits_32(reg_addr,  ALLWINNER_H6_PLLX_LOCK_ENABLE);

    while (!(readl(reg_addr) & ALLWINNER_H6_PLLX_LOCK))  ;

    return  0;

}


static const struct clk_ops allwinner_h6_ops = {
	.set_rate = allwinner_h6_set_rate,
	.get_rate = allwinner_h6_get_rate,
	.disable =  allwinner_h6_disable,
    .enable  =  allwinner_h6_enable,
};

static int allwinner_h6_clk_probe(struct udevice *dev)
{

    allwinner_h6_clk_plat_t  * plat  =  dev_get_plat(dev);
    assert(plat ==  NULL);

    plat->base  =  devfdt_get_addr(dev);

    if (plat->base ==  FDT_ADDR_T_NONE) {
        log_err("alllwinner h6 get reg base failed!\n");
        return  -EINVAL;
    }

	return 0;
}

static const struct udevice_id allwinner_h6_clk_id[] = {
	{ .compatible = "allwinner,h6-v200-ccu" },
	{ }
};

U_BOOT_DRIVER(allwinner_h6_clk) = {
	.name = "hsdk-cgu-clk",
	.id = UCLASS_CLK,
	.of_match = allwinner_h6_clk_id,
	.probe = allwinner_h6_clk_probe,
	.plat_auto	= sizeof(allwinner_h6_clk_plat_t),
	.ops = &allwinner_h6_ops,
};



