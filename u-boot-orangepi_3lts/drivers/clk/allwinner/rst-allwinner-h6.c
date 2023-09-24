#include  <common.h>
#include  <log.h>
#include  <errno.h>
#include  <dt-bindings/clock/allwinner-h6-clock.h>

#include  "rst-allwinner-h6.h"

#define  ALLWINNER_FLAG_NONE   (0ul)
#define  ALLWINNER_FLAG_RST_VALID     (1<<1ul)
#define  ALLWINNER_FLAG_GATE_VALID    (1<<2ul)

typedef  struct {
    u32  offset;
    u32  pos;
    u32  flag;
} allwinner_ccu_rst_gate_t;

#define ALLWINNER_RESET(clk, _offset, _pos)  [clk] = { \
                        .offset  =  _offset,  \
                        .pos     =  _pos,   \
                        .flag    =  ALLWINNER_FLAG_RST_VALID, \
                    }

#define ALLWINNER_GATE(clk, _offset, _pos)  [clk] = { \
                        .offset  =  _offset,  \
                        .pos     =  _pos,   \
                        .flag    =  ALLWINNER_FLAG_GATE_VALID, \
                    }


#define ALLWINNER_RESET_ARRAY                         \
    ALLWINNER_RESET(ALLWINNER_CLK_MBUS,   0x540, 30),      \
    ALLWINNER_RESET(ALLWINNER_CLK_NAND0,   0x82c,  16),      \
    ALLWINNER_RESET(ALLWINNER_CLK_NAND1,   0x82c,  16),      \
    ALLWINNER_RESET(ALLWINNER_CLK_SMHC0,   0x84c,  16),     \
    ALLWINNER_RESET(ALLWINNER_CLK_SMHC1,   0x84c,  17),      \
    ALLWINNER_RESET(ALLWINNER_CLK_SMHC2,   0x84c,  18),      \
    ALLWINNER_RESET(ALLWINNER_CLK_UART0,   0x90c,  16),      \
    ALLWINNER_RESET(ALLWINNER_CLK_UART1,   0x90c,  17),      \
    ALLWINNER_RESET(ALLWINNER_CLK_UART2,   0x90c,  18),      \
    ALLWINNER_RESET(ALLWINNER_CLK_UART3,   0x90c,  19),      \
    ALLWINNER_RESET(ALLWINNER_CLK_SPI0,    0x86c,  16),      \
    ALLWINNER_RESET(ALLWINNER_CLK_SPI1,    0x86c,  17)


#define ALLWINNER_GATE_ARRAY                            \
    ALLWINNER_GATE(ALLWINNER_CLK_MBUS,   0x540, 31),      \
    ALLWINNER_GATE(ALLWINNER_CLK_NAND0,   0x82c,  0),      \
    ALLWINNER_GATE(ALLWINNER_CLK_NAND1,   0x82c,  0),      \
    ALLWINNER_GATE(ALLWINNER_CLK_SMHC0,   0x84c,  0),      \
    ALLWINNER_GATE(ALLWINNER_CLK_SMHC1,   0x84c,  1),      \
    ALLWINNER_GATE(ALLWINNER_CLK_SMHC2,   0x84c,  2),      \
    ALLWINNER_GATE(ALLWINNER_CLK_UART0,   0x90c,  0),      \
    ALLWINNER_GATE(ALLWINNER_CLK_UART1,   0x90c,  1),      \
    ALLWINNER_GATE(ALLWINNER_CLK_UART2,   0x90c,  2),      \
    ALLWINNER_GATE(ALLWINNER_CLK_UART3,   0x90c,  3),      \
    ALLWINNER_GATE(ALLWINNER_CLK_SPI0,    0x86c,  0),      \
    ALLWINNER_GATE(ALLWINNER_CLK_SPI1,    0x86c,  1)


static const allwinner_ccu_rst_gate_t allwinner_h6_reset[] = {
                            ALLWINNER_RESET_ARRAY
};

static const allwinner_ccu_rst_gate_t allwinner_h6_gate[] = { 
                            ALLWINNER_GATE_ARRAY
};


int32_t  get_allwinner_rst_or_gate(const uint32_t is_gate, const uint32_t clk, 
                uint32_t * const offset,  uint32_t * const pos)
{
    int32_t  ret  =  0;
    const  allwinner_ccu_rst_gate_t * const  arrays  =  is_gate? 
                                allwinner_h6_gate: allwinner_h6_reset;

    uint32_t  gate_size   =  sizeof(allwinner_h6_gate) / 
                                    sizeof(allwinner_ccu_rst_gate_t);
    uint32_t  reset_size  =  sizeof(allwinner_h6_reset)  /
                                    sizeof(allwinner_ccu_rst_gate_t);

    uint32_t target_size  =  is_gate? gate_size: reset_size;

    if (clk >= target_size) {
        log_err("allwinner reset_or_gate clk_id [%u] invalid!", clk);
        return  -1;
    }

    if (arrays[clk].flag & (ALLWINNER_FLAG_GATE_VALID | ALLWINNER_FLAG_GATE_VALID) ) {
        *offset  =  arrays[clk].offset;
        *pos   =  arrays[clk].offset;
        ret =  0;
    } else {
        ret =  -1;
    }

    return  ret;

}






















