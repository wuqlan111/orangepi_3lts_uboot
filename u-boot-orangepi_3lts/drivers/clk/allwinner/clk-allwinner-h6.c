
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
#include  <linux/delay.h>

#include  "clk-allwinner-h6.h"
#include  "rst-allwinner-h6.h"

#define  ALLWINNER_PLLX_SRC_CLK            (24000000ul)

typedef  struct {  
    fdt_addr_t  base;
} allwinner_h6_clk_plat_t;


static  uint32_t  allwinner_clk_regs_offset[] = {  foreach_array_clk_ctrl };
static  uint32_t  allwinner_pll_m[] = { 1,  2,  4};

static  int32_t  _allwinner_h6_pll_enable(uint32_t clk_id, uint32_t base)
{
    uint32_t * regs =  (uint32_t *)(base + allwinner_clk_regs_offset[clk_id]);
    setbits_32(regs,  ALLWINNER_H6_PLLX_ENABLE | ALLWINNER_H6_PLLX_LOCK_ENABLE);
    while (!(readl(regs) & ALLWINNER_H6_PLLX_LOCK))  ;
    udelay(20);
    return  0;
}


static  int32_t  _allwinner_h6_pll_disable(uint32_t clk_id, uint32_t base)
{
    uint32_t * regs =  (uint32_t *)(base + allwinner_clk_regs_offset[clk_id]);
    clrbits_32(regs,  ALLWINNER_H6_PLLX_ENABLE | ALLWINNER_H6_PLLX_LOCK_ENABLE);
    return  0;
}



static  int32_t  _allwinner_h6_pll_set_rate(uint32_t clk_id,  ulong rate, uint32_t base,  ulong * new_rate)
{
    uint32_t  pll_m, pll_n, best_pll_m, best_pll_n;
    ulong   rate_delta  =  rate;
    pll_m  = pll_n  = best_pll_m  =  best_pll_n  =  0;

    const  uint32_t  pll_m_count  =  ARRAY_SIZE(allwinner_pll_m);
    const  ulong   tmp_rate_min   =  rate * allwinner_pll_m[0];
    const  ulong   tmp_rate_max   =  rate * allwinner_pll_m[pll_m_count - 1];
    const  ulong   pll_n_min  =  tmp_rate_min / ALLWINNER_PLLX_SRC_CLK;
    const  ulong   pll_n_max  =  tmp_rate_max / ALLWINNER_PLLX_SRC_CLK;

    if (!rate) {
        log_err("rate is zero!");
        return  -EINVAL;
    }

    for (int32_t  i  =  0;  i  <  pll_m_count; i++) {
        uint32_t  find_best  =  0;
        for (pll_n =  pll_n_min; pll_n < pll_n_max; pll_n++) {
            ulong  tmp_rate  =  (pll_n * ALLWINNER_PLLX_SRC_CLK) / allwinner_pll_m[i];
            ulong  tmp_delta  =  rate - tmp_rate;

            if ( (pll_n > 0xff) || (tmp_rate > rate) ) {
                break;
            }

            if (tmp_delta < rate_delta) {
                rate_delta  =  tmp_delta;
                best_pll_m  =  allwinner_pll_m[i];
                best_pll_n  =  pll_n;
                *new_rate   =  tmp_rate;
            }

            if (tmp_rate == rate) {
                find_best =  1;
                break;
            }

        }
    }


    if ( !best_pll_m || !best_pll_n) {
        log_err("rate -- [%lu] not support!", rate);
        return  -EINVAL;
    }

    uint32_t  flag,  mask;
    flag  =  (best_pll_n - 1) << 8;
    mask  =  ALLWINNER_H6_PLLX_FACTOR_N;

    switch (clk_id) {
        case  ALLWINNER_PLL_CPUX:
            if (best_pll_m < 4) {
                flag  |=  ( pll_m - 1)  << 16;
            } else {
                flag  |=  0x2  <<  16;
            }

            mask  |=  0x3 <<  16;
            break;

        case  ALLWINNER_PLL_DDR0 ... ALLWINNER_PLL_GPU:
        case  ALLWINNER_PLL_VE ...  ALLWINNER_PLL_HSIC:
            flag  |=  (best_pll_m - 1);
            mask  |=  0x3;
            break;
        // case  ALLWINNER_PLL_VIDEO0:
        // case  ALLWINNER_PLL_VIDEO1: 
            if (best_pll_m > 2) {
                log_err("pll_m -- [%u], rate -- [%lu] invalid!", best_pll_m, rate);
                return  -EINVAL;
            }
            flag  |=  (best_pll_m - 1) << 1;
            mask  |=  0x2;
            break;
        
        default:
            log_err("clk_id -- %u, invalid pll clk id!", clk_id);
            return  -EINVAL;
    }

    uint32_t * reg_addr =  (uint32_t *)(base + allwinner_clk_regs_offset[clk_id]);

    uint32_t  clk_ctrl  =  readl(reg_addr);
    uint32_t  clk_enable  =  clk_ctrl & ALLWINNER_H6_PLLX_ENABLE ? 1: 0;
    uint32_t  support_change_rate  =  (clk_id == ALLWINNER_PLL_CPUX) || 
                ( clk_id ==  ALLWINNER_PLL_GPU ) ? 1:  0;

    if ( (clk_ctrl & mask)  ==  flag) {
        log_warning("clk [%s] rate has been set!", allwinner_clk_2_str(clk_id));
        return  0;
    }

    if (clk_enable && !support_change_rate) {
        log_err("clk [%s] rate don't support DFS!", allwinner_clk_2_str(clk_id));
        return  -1;
    }

    clrbits_32(reg_addr,  ALLWINNER_H6_PLLX_LOCK_ENABLE);
    clrsetbits_32(reg_addr, mask,  flag);
    setbits_32(reg_addr,  ALLWINNER_H6_PLLX_LOCK_ENABLE);

    while (!(readl(reg_addr) & ALLWINNER_H6_PLLX_LOCK))  ;

    udelay(20);

    return  0;

}


static  int32_t  _allwinner_h6_pll_get_rate(uint32_t clk_id,  uint32_t base,  ulong * rate)
{

    uint32_t * reg_addr =  (uint32_t *)(base + allwinner_clk_regs_offset[clk_id]);
    uint32_t  pll_n, pll_m, tmp_m;
    pll_n  = pll_m  = tmp_m = 0;    

    uint32_t  clk_ctrl = readl(reg_addr);
    pll_n  =  ((clk_ctrl & ALLWINNER_H6_PLLX_FACTOR_N ) >>  8)  + 1;

    switch (clk_id) {
        case  ALLWINNER_PLL_CPUX:
            tmp_m  =  (clk_ctrl & GENMASK(17,  16)) >> 16;
            pll_m  =  1 << tmp_m;
            break;

        case  ALLWINNER_PLL_DDR0 ... ALLWINNER_PLL_GPU:
        case  ALLWINNER_PLL_VE ...  ALLWINNER_PLL_HSIC:
            tmp_m  =  clk_ctrl & 0x3;
            pll_m =  (tmp_m ==  0)  || (tmp_m == 3) ? tmp_m + 1: 2;
            break;
        case ALLWINNER_PLL_VIDEO0_4X ... ALLWINNER_PLL_VIDEO1_1X:
            pll_m  =  clk_ctrl &  BIT(1) ? 2: 1;
            break;

        case  ALLWINNER_PLL_AUDIO_4X:
        case  ALLWINNER_PLL_AUDIO: {
                uint32_t in_div  =  clk_ctrl & BIT(1)? 2: 1;
                uint32_t out_div  =  clk_ctrl & BIT(0)? 2:  1;
                uint32_t p = (clk_ctrl & GENMASK(21,  16) >> 16) + 1;
                if (clk_id == ALLWINNER_PLL_AUDIO_4X) {
                    pll_m  =  2 * in_div;
                } else {
                    pll_m  =  p * in_div * out_div;
                }
        }
            break;
        
        default:
            log_err("clk_id -- %u, invalid pll clk id!", clk_id);
            return  -EINVAL;
    }

    uint32_t  shift =  0;
    switch (clk_id) {
        case ALLWINNER_PLL_PERI0_1X:
        case ALLWINNER_PLL_PERI1_1X:
        case ALLWINNER_PLL_VIDEO0_1X:
        case ALLWINNER_PLL_VIDEO1_1X:
            shift =  2;
            break;

        case ALLWINNER_PLL_PERI0_2X:
        case ALLWINNER_PLL_PERI1_2X:
            shift  =  1;
            break;
    
        default:
            shift  =  0;
    }

    *rate  =  ( ALLWINNER_PLLX_SRC_CLK * pll_n) / ( pll_m * (1 << shift) );

    return  0;

}

static int32_t _allweinner_clk_psi_get_rate(uint32_t base,  ulong * rate)
{
    int32_t  ret = 0;

    uint32_t * reg_addr  =  (uint32_t *)(base + allwinner_clk_regs_offset[ALLWINNER_CLK_PSI_AHB1_AHB2]);
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
        ret = _allwinner_h6_pll_get_rate(ALLWINNER_PLL_PERI0_1X, base, &src_rate);
        if (ret) {
            log_err("get pll_peri0 clk failed!\n");
            return  ret;
        }
    }

    uint32_t  clk_n  =  (clk_ctrl & GENMASK(9,  8)) >> 8;
    uint32_t  clk_m  =  (clk_ctrl & GENMASK(1, 0))  +  1;
    uint32_t  factor =  (1 << clk_n) * clk_m;
    
    *rate  =  src_rate / factor;

    return  0;

}


static int32_t _allweinner_clk_psi_set_rate(uint32_t base,  ulong rate,  ulong * new_rate)
{
    int32_t  ret  =  0;
    uint32_t * reg_addr  =  (uint32_t *)(base + allwinner_clk_regs_offset[ALLWINNER_CLK_PSI_AHB1_AHB2]);
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
        ret = _allwinner_h6_pll_get_rate(ALLWINNER_PLL_PERI0_1X, base, &src_rate);
        if (ret) {
            log_err("get pll_peri0 clk failed!\n");
            return  ret;
        }
    }

    if ( (rate < (src_rate >> 5)) || (rate > src_rate) ) {
        log_err("src rate [%lu] not support target rate [%lu]!", src_rate, rate);
        return   -EINVAL;
    }

    const  ulong   clk_m_min  =  src_rate % rate ? (src_rate / rate) + 1: src_rate / rate;
    const  ulong   clk_m_max  =  src_rate % (rate << 3) ? (src_rate / (rate << 3)) + 1: src_rate / (rate << 3);

    uint32_t  clk_n, clk_m,  best_clk_n, best_clk_m;
    clk_n  =  clk_m  =  best_clk_n  = best_clk_m  = 0; 

    ulong  rate_delta  =  src_rate - rate;

    for (clk_n =  0; clk_n < 4; clk_n++) {
        uint32_t  find_best  =  0;
        for (clk_m =  clk_m_min; (clk_m <= clk_m_max) && (clk_m <= 4); clk_m++) {
            ulong  tmp_factor  =  clk_m * (1 << clk_n);
            ulong  tmp_rate   =  src_rate / tmp_factor;
            ulong  tmp_delta  =  tmp_rate - rate;

            if ( tmp_rate < rate ) {
                continue;
            }

            if (tmp_delta  < rate_delta) {
                best_clk_m  =  clk_m;
                best_clk_n  =  clk_n;
                rate_delta  =  tmp_delta;
                *new_rate  =  tmp_rate;
            }

            if (tmp_delta == 0) {
                find_best  =  1;
                break;
            }
                
        }

        if (find_best) {
            break;
        }

    }

    if (!best_clk_m) {
        return  -1;
    }

    uint32_t  flag  =  ( best_clk_n << 8 ) | (best_clk_m  - 1);
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
    uint32_t  * reg_addr   =  (uint32_t *)(base + allwinner_clk_regs_offset[reg_offset]);

    uint32_t  clk_ctrl  =  readl(reg_addr);
    uint32_t  clk_src  =  (clk_ctrl & GENMASK(25, 24)) >> 24;
    ulong  src_rate  = 0;

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
        ret  =  _allwinner_h6_pll_get_rate(ALLWINNER_PLL_PERI0_1X, base, &src_rate);
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


static int32_t _allweinner_clk_ahbx_set_rate(ulong rate, uint32_t is_ahb3, uint32_t base,  ulong * new_rate)
{
    int32_t  ret  =  0;
    uint32_t  reg_offset  =  is_ahb3 ? ALLWINNER_CLK_AHB3: ALLWINNER_CLK_PSI_AHB1_AHB2;
    uint32_t  * reg_addr   =  (uint32_t *)(base + allwinner_clk_regs_offset[reg_offset]);

    uint32_t  clk_ctrl  =  readl(reg_addr);
    uint32_t  clk_src  =  (clk_ctrl & GENMASK(25, 24)) >> 24;
    ulong  src_rate  = 0;

    if (!is_ahb3) {
        ret  =  _allweinner_clk_psi_set_rate(base,  rate, new_rate);
        return  ret;
    }

    if (clk_src == 0) {
        src_rate  = 24000000;
    } else if (clk_src == 1) {
        src_rate  =  32768;
    } else if (clk_src ==  3) {
        ulong tmp_src_rate  =  0;
        ret = _allwinner_h6_pll_get_rate(ALLWINNER_PLL_PERI0_1X, base, &tmp_src_rate);
        src_rate  =  tmp_src_rate;
    } else {
        ret  =  _allweinner_clk_psi_get_rate(base, &src_rate);
    }
    
    if (ret) {
        return  ret;
    }

    if ( (rate < (src_rate >> 5)) || (rate > src_rate) ) {
        log_err("src rate [%lu] not support target rate [%lu]!", src_rate, rate);
        return   -EINVAL;
    }

    const  ulong   clk_m_min  =  src_rate % rate ? (src_rate / rate) + 1: src_rate / rate;
    const  ulong   clk_m_max  =  src_rate % (rate << 3) ? (src_rate / (rate << 3)) + 1: src_rate / (rate << 3);

    uint32_t  clk_n, clk_m,  best_clk_n, best_clk_m;
    clk_n  =  clk_m  =  best_clk_n  = best_clk_m  = 0; 

    ulong  rate_delta  =  src_rate - rate;

    for (clk_n =  0; clk_n < 4; clk_n++) {
        uint32_t  find_best  =  0;
        for (clk_m =  clk_m_min; (clk_m <= clk_m_max) && (clk_m <= 4); clk_m++) {
            ulong  tmp_factor  =  clk_m * (1 << clk_n);
            ulong  tmp_rate   =  src_rate / tmp_factor;
            ulong  tmp_delta  =  tmp_rate - rate;

            if ( tmp_rate < rate ) {
                continue;
            }

            if (tmp_delta  < rate_delta) {
                best_clk_m  =  clk_m;
                best_clk_n  =  clk_n;
                rate_delta  =  tmp_delta;
                *new_rate  =  tmp_rate;
            }

            if (tmp_delta == 0) {
                find_best  =  1;
                break;
            }
                
        }

        if (find_best) {
            break;
        }

    }

    if (!best_clk_m) {
        return  -1;
    }

    uint32_t  flag  =  ( best_clk_n << 8 ) | (best_clk_m - 1);
    uint32_t  mask  =   GENMASK(9,  8) | GENMASK(1,  0);

    clk_ctrl  &=  ~mask;
    clk_ctrl  |=  ( flag & mask) ;

    writel(clk_ctrl,  reg_addr);

    return  0;

}

static int32_t _allwinner_h6_get_clk_by_cfg(uint32_t clk_id,  uint32_t base,  ulong * rate)
{
    int32_t  ret  =  0;
    uint32_t  * reg_addr   =  (uint32_t *)(base + allwinner_clk_regs_offset[clk_id]);
    const uint32_t flag = readl(reg_addr);
    const clock_reg_cfg_t * reg_cfg = get_clk_config(clk_id);
    if (!reg_cfg) {
        return  -1;
    }

    const uint32_t  clk_select_max  =  reg_cfg->clk_select_bits? GENMASK(reg_cfg->clk_select_bits - 1,  0):  0;
    const uint32_t  clk_select  = (flag >> 24) & clk_select_max;
    if (!clk_select_max || !(( 1 << clk_select) & (reg_cfg->clk_select_vld)) ) {
        _DBG_PRINTF("clk select bits invalid!\n");
        return  -1;
    }

    const  uint32_t  clk_m_max  =  reg_cfg->clk_m_bits ? GENMASK(reg_cfg->clk_m_bits - 1,  0): 0;
    const  uint32_t  clk_n_max  =  reg_cfg->clk_n_bits ? GENMASK(reg_cfg->clk_n_bits - 1,  0): 0;
    const  uint32_t  clk_n  =  clk_n_max?  ( flag >> 8 ) & clk_n_max: 0;
    const  uint32_t  clk_m  =  clk_m_max?  flag & clk_m_max:  0;

    uint32_t factor  =  0;

    if (clk_n_max && clk_m_max) {
        factor  =  (1 << clk_n) * (clk_m + 1);
    } else {
        factor  =  clk_n_max ? 1 << clk_n:  clk_m;
    }

    ulong src_rate =  0;
    if (reg_cfg->clk_select[clk_select] == CLOCK_OSC24M) {
        src_rate  =  24000000;
    } else {
        ret  =  _allwinner_h6_pll_get_rate(clk_id, base,  &src_rate);
    }

    if (ret) {
        _DBG_PRINTF("get src clk failed!\n");
        return  ret;
    }

    *rate  =  src_rate / factor;

    return   ret;

}


static int32_t _allwinner_h6_set_clk_by_cfg(uint32_t clk_id,  ulong rate, uint32_t base,  ulong * new_rate)
{
    int32_t  ret  =  0;
    uint32_t  * reg_addr   =  (uint32_t *)(base + allwinner_clk_regs_offset[clk_id]);
    const clock_reg_cfg_t * reg_cfg = get_clk_config(clk_id);
    if (!reg_cfg) {
        return  -1;
    }

    const uint32_t  clk_select_max  =  reg_cfg->clk_select_bits? GENMASK(reg_cfg->clk_select_bits - 1,  0):  0;
    if (!clk_select_max || !reg_cfg->clk_select_vld) {
        _DBG_PRINTF("clk select bits invalid!\n");
        return  -1;
    }

    const  uint32_t  clk_m_max  =  reg_cfg->clk_m_bits ? GENMASK(reg_cfg->clk_m_bits - 1,  0): 0;
    const  uint32_t  clk_n_max  =  reg_cfg->clk_n_bits ? GENMASK(reg_cfg->clk_n_bits - 1,  0): 0;

    if (!clk_n_max && !clk_m_max) {
        _DBG_PRINTF("clock reg cfg invalid for both m and n bits zero!\n");
        return  -1;
    }

    uint32_t  best_clk_n, best_clk_m, best_clk_select, find_best;
    ulong delta, src_rate;
    best_clk_n  = best_clk_m  = best_clk_select  = find_best =  0; 
    delta  =  rate;

    for (uint32_t tmp_select = 0; tmp_select < (clk_select_max + 1); tmp_select++) {
        if ( ! ((1 << tmp_select) & reg_cfg->clk_select_vld) ) {
            continue;
        }

        if (reg_cfg->clk_select[tmp_select] == CLOCK_OSC24M) {
            src_rate  =  24000000;
        } else {
            ret  =  _allwinner_h6_pll_get_rate(reg_cfg->clk_select[tmp_select], base,  &src_rate);
            if (ret) {
                continue;
            }
        }

        for (uint32_t tmp_n =  0; tmp_n < (clk_n_max + 1) ; tmp_n++) {
            for (uint32_t tmp_m =  0; tmp_m < (clk_m_max + 1) ; tmp_m++) {
                uint32_t  tmp_factor =  ( 1<< tmp_n) * (tmp_m + 1);
                ulong  tmp_rate   =  src_rate / tmp_factor;
                ulong  tmp_delta  =  tmp_rate > rate ? tmp_rate - rate: rate - tmp_rate;

                if (tmp_delta < delta) {
                    best_clk_n  =  tmp_n;
                    best_clk_m  = tmp_m;
                    best_clk_select  =  tmp_select;
                    delta = tmp_delta;
                    *new_rate  =  tmp_rate;
                }

                if (tmp_rate == rate) {
                    find_best  =  1;
                    break;
                }

            }

            if (find_best) {
                break;
            }

        }

        if (find_best) {
            break;
        }

    }

    if (delta == rate) {
        _DBG_PRINTF("no src clk found failed!\n");
        return  -1;
    }

    uint32_t mask  = (clk_select_max << 24)  |  (clk_n_max << 8)  |  clk_m_max;
    uint32_t flag  = (best_clk_select << 24)  |  (best_clk_n << 8)  |  best_clk_m;

    clrsetbits_32(reg_addr,  mask,  flag);

    return   0;

}


static int32_t _allwinner_h6_get_clk_by_alias(uint32_t clk_id, uint32_t base,  ulong * rate)
{
    int32_t  ret  =  0;
    uint32_t  * reg_addr   =  (uint32_t *)(base + allwinner_clk_regs_offset[clk_id]);
    uint32_t  alias_clk =  0;

    if (get_clk_alias(clk_id,  &alias_clk)) {
        _DBG_PRINTF("get alias clk failed!\n");
        return  -1;
    }

    if (alias_clk ==  CLOCK_OSC24M) {
        *rate  =  24000000;
    } else if (alias_clk <= ALLWINNER_PLL_MAX) {
        ret  =  _allwinner_h6_pll_get_rate(alias_clk, base,  rate);
    } else {
        ret  =  _allwinner_h6_get_clk_by_cfg(alias_clk,  base,  rate);
    }

    return  ret;

}



static  int32_t  _allwinner_h6_clk_set_rate(uint32_t clk_id,  ulong rate, uint32_t base,  ulong * new_rate)
{
    int32_t  ret  =  0;

    if (clk_id <= ALLWINNER_PLL_MAX) {
        ret  =  _allwinner_h6_pll_set_rate(clk_id,  rate,  base,  new_rate);
    } else if ( (clk_id == ALLWINNER_CLK_PSI_AHB1_AHB2) || (clk_id == ALLWINNER_CLK_AHB3)) {
        uint32_t is_ahb3 = clk_id == ALLWINNER_CLK_AHB3? 1:  0; 
        ret  =  _allweinner_clk_ahbx_set_rate(rate,  is_ahb3, base, new_rate);
    } else if ( !_allwinner_h6_set_clk_by_cfg(clk_id,  rate, base,  new_rate) ){
        _DBG_PRINTF("set clk by cfg success!\n");
    } else {
        uint32_t alias_clk  =  0;
        ret  =  get_clk_alias(clk_id,  &alias_clk);
    }

    return   ret;

}


static  int32_t  _allwinner_h6_clk_get_rate(uint32_t clk_id, uint32_t base,  ulong * rate)
{
    int32_t  ret  =  0;

    if (clk_id <= ALLWINNER_PLL_MAX) {
        ret  =  _allwinner_h6_pll_get_rate(clk_id,  base,  rate);
    } else if ( (clk_id == ALLWINNER_CLK_PSI_AHB1_AHB2) || (clk_id == ALLWINNER_CLK_AHB3)) {
        uint32_t is_ahb3 = clk_id == ALLWINNER_CLK_AHB3? 1:  0; 
        ret  =  _allweinner_clk_ahbx_get_rate(is_ahb3, base,  rate);
    } else if (!_allwinner_h6_get_clk_by_cfg(clk_id,  base,  rate)){
        _DBG_PRINTF("get clk rate by reg cfg success\n");
    } else {
        ret  =  _allwinner_h6_get_clk_by_alias(clk_id,  base,  rate);
    }

    return   ret;

}

static void printf_clk_info(const struct clk * const clk)
{
    if (!clk) {
        return;
    }

    if (clk->dev) {
        _DBG_PRINTF("clk_dev -- %s\n", clk->dev->name? clk->dev->name: "null");        
    } else {
        _DBG_PRINTF("no clk dev!\n");
    }

    _DBG_PRINTF("clk_rate -- %lld\n", clk->rate);
    _DBG_PRINTF("flags -- 0x%08x\n", clk->flags);
    _DBG_PRINTF("enable -- %d\n", clk->enable_count);
    _DBG_PRINTF("id -- %lu\n", clk->id);
    _DBG_PRINTF("enable -- %lu\n", clk->data);
}


static ulong allwinner_h6_set_rate(struct clk *clk, ulong rate)
{
    int32_t  ret  =  0;
    ulong new_rate =  0;
    allwinner_h6_clk_plat_t  * plat  =  dev_get_plat(clk->dev);
    uint32_t  clk_id   =  clk->id;

    ret  =  _allwinner_h6_clk_set_rate(clk_id, rate, plat->base, &new_rate);

	return  ret? -1: new_rate;
}


static ulong allwinner_h6_get_rate(struct clk *clk)
{
    int32_t  ret  =  0;
    ulong rate =  0;
    allwinner_h6_clk_plat_t  * plat  =  dev_get_plat(clk->dev);
    uint32_t  clk_id   =  clk->id;

    ret  =  _allwinner_h6_clk_get_rate(clk_id, plat->base, &rate);

    if (ret) {
        printf_clk_info(clk);
    }

    return  ret ? -1: rate;

}

int32_t allwinner_h6_enable(struct clk *clk)
{
    int32_t  ret  =  0;
    allwinner_h6_clk_plat_t  * plat  =  dev_get_plat(clk->dev);
    uint32_t  clk_id   =  clk->id;

    if (clk_id <= ALLWINNER_PLL_MAX) {
        ret  =   _allwinner_h6_pll_enable(clk_id, plat->base);
    }

    return   ret;
}

int32_t allwinner_h6_disable(struct clk *clk)
{
    int32_t  ret  =  0;
    allwinner_h6_clk_plat_t  * plat  =  dev_get_plat(clk->dev);
    uint32_t  clk_id   =  clk->id;

    if (clk_id <= ALLWINNER_PLL_MAX) {
        ret  =  _allwinner_h6_pll_disable(clk_id, plat->base);
    }

    return  ret;
}

static const struct clk_ops allwinner_h6_ops = {
	.set_rate = allwinner_h6_set_rate,
	.get_rate = allwinner_h6_get_rate,
	.disable =  allwinner_h6_disable,
    .enable  =  allwinner_h6_enable,
};

static int32_t allwinner_h6_clk_probe(struct udevice *dev)
{
    int32_t  ret  =  0;
    allwinner_h6_clk_plat_t  * plat  =  dev_get_plat(dev);
    assert(plat ==  NULL);

    plat->base  =  devfdt_get_addr(dev);

    if (plat->base ==  FDT_ADDR_T_NONE) {
        dev_err(dev, "alllwinner h6 get reg base failed!\n");
        ret = -EINVAL;
    }

	return  ret;
}

static const struct udevice_id allwinner_h6_clk_id[] = {
	{ .compatible = "allwinner,h6-v200-ccu" },
	{ }
};

U_BOOT_DRIVER(allwinner_h6_clk) = {
	.name = "allwinner-h6-clk",
	.id = UCLASS_CLK,
	.of_match = allwinner_h6_clk_id,
	.probe = allwinner_h6_clk_probe,
	.plat_auto	= sizeof(allwinner_h6_clk_plat_t),
	.ops = &allwinner_h6_ops,
    .flags = DM_FLAG_PRE_RELOC,
};



