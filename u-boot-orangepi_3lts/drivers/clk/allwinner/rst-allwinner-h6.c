
#define  LOG_CATEGORY UCLASS_CLK

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


#define  TYPE_CASE(x)         case x: return #x;
char * allwinner_clk_2_str(const uint32_t clk)
{

    switch (clk) {
        TYPE_CASE(ALLWINNER_PLL_CPUX)
        TYPE_CASE(ALLWINNER_PLL_DDR0)
        TYPE_CASE(ALLWINNER_PLL_PERI0_4X)
        TYPE_CASE(ALLWINNER_PLL_PERI0_2X)
        TYPE_CASE(ALLWINNER_PLL_PERI0_1X)
        TYPE_CASE(ALLWINNER_PLL_PERI1_4X)
        TYPE_CASE(ALLWINNER_PLL_PERI1_2X)
        TYPE_CASE(ALLWINNER_PLL_PERI1_1X)
        TYPE_CASE(ALLWINNER_PLL_GPU)
        TYPE_CASE(ALLWINNER_PLL_VIDEO0_4X)
        TYPE_CASE(ALLWINNER_PLL_VIDEO0_1X)
        TYPE_CASE(ALLWINNER_PLL_VIDEO1_4X)
        TYPE_CASE(ALLWINNER_PLL_VIDEO1_1X)
        TYPE_CASE(ALLWINNER_PLL_VE)
        TYPE_CASE(ALLWINNER_PLL_DE)
        TYPE_CASE(ALLWINNER_PLL_HSIC)
        TYPE_CASE(ALLWINNER_PLL_AUDIO_4X)
        TYPE_CASE(ALLWINNER_PLL_AUDIO)

        // clk
        TYPE_CASE(ALLWINNER_CLK_CPUX_AXI)
        TYPE_CASE(ALLWINNER_CLK_CPUX_APB)
        TYPE_CASE(ALLWINNER_CLK_PSI_AHB1_AHB2)
        TYPE_CASE(ALLWINNER_CLK_AHB3)
        TYPE_CASE(ALLWINNER_CLK_APB1)
        TYPE_CASE(ALLWINNER_CLK_APB2)
        TYPE_CASE(ALLWINNER_CLK_MBUS)
        TYPE_CASE(ALLWINNER_CLK_NAND0)
        TYPE_CASE(ALLWINNER_CLK_NAND1)
        TYPE_CASE(ALLWINNER_CLK_SMHC0)
        TYPE_CASE(ALLWINNER_CLK_SMHC1)
        TYPE_CASE(ALLWINNER_CLK_SMHC2)
        TYPE_CASE(ALLWINNER_CLK_SPI0)
        TYPE_CASE(ALLWINNER_CLK_SPI1)
        TYPE_CASE(ALLWINNER_CLK_TS)
        TYPE_CASE(ALLWINNER_CLK_CIR_TX)
        TYPE_CASE(ALLWINNER_CLK_I2S_PCM3)
        TYPE_CASE(ALLWINNER_CLK_I2S_PCM0)
        TYPE_CASE(ALLWINNER_CLK_I2S_PCM1)
        TYPE_CASE(ALLWINNER_CLK_I2S_PCM2)
        TYPE_CASE(ALLWINNER_CLK_OWA)
        TYPE_CASE(ALLWINNER_CLK_DMIC)
        TYPE_CASE(ALLWINNER_CLK_AUDIO_HUB)
        TYPE_CASE(ALLWINNER_CLK_USB0)
        TYPE_CASE(ALLWINNER_CLK_USB1)
        TYPE_CASE(ALLWINNER_CLK_USB3)
        TYPE_CASE(ALLWINNER_CLK_PCIE_REF)
        TYPE_CASE(ALLWINNER_CLK_PCIE_MAXI)
        TYPE_CASE(ALLWINNER_CLK_PCIE_AUX)
        TYPE_CASE(ALLWINNER_CLK_HDMI)
        TYPE_CASE(ALLWINNER_CLK_HDMI_SLOW)
        TYPE_CASE(ALLWINNER_CLK_HDMI_CEC)
        TYPE_CASE(ALLWINNER_CLK_HDMI_HDCP)
        TYPE_CASE(ALLWINNER_CLK_TCON_LED)
        TYPE_CASE(ALLWINNER_CLK_TCON_TV)
        TYPE_CASE(ALLWINNER_CLK_CSI_MISC)
        TYPE_CASE(ALLWINNER_CLK_CSI_TOP)
        TYPE_CASE(ALLWINNER_CLK_CSI_MASTER)
        TYPE_CASE(ALLWINNER_CLK_UART0)
        TYPE_CASE(ALLWINNER_CLK_UART1)
        TYPE_CASE(ALLWINNER_CLK_UART2)
        TYPE_CASE(ALLWINNER_CLK_UART3)

    }

    return   "invalid allwinner clk";

}

















