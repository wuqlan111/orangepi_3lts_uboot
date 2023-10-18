#ifndef   __RESET_ALLWINNER_H6_H__
#define   __RESET_ALLWINNER_H6_H__

#include  <linux/bitops.h>
#include  <stdint.h>

#define  CLOCK_OSC24M   (0x450ul)
#define  CLOCK_CCU32K   (0x451ul)

typedef struct {
    uint32_t   clk_id;
    uint32_t   clk_select_vld;
    uint32_t   clk_select_bits;
    uint32_t   clk_select[8];
    uint32_t   clk_m_bits;
    uint32_t   clk_n_bits;
} clock_reg_cfg_t;

int32_t  get_allwinner_rst_or_gate(const uint32_t is_gate, const uint32_t clk, 
                uint32_t * const offset,  uint32_t * const pos);


char * allwinner_clk_2_str(const uint32_t clk);

const clock_reg_cfg_t * get_clk_config(const uint32_t clk_id);

int32_t  get_clk_alias(const uint32_t clk_id,  uint32_t * const alias_clk);

#endif

