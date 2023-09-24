#ifndef   __RESET_ALLWINNER_H6_H__
#define   __RESET_ALLWINNER_H6_H__

#include  <linux/bitops.h>
#include  <stdint.h>

int32_t  get_allwinner_rst_or_gate(const uint32_t is_gate, const uint32_t clk, 
                uint32_t * const offset,  uint32_t * const pos);









#endif

