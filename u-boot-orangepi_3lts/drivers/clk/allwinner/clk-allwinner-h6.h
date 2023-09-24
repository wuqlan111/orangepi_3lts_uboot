
#ifndef   __COCLK_ALLWINNER_H6_H__
#define   __COCLK_ALLWINNER_H6_H__

#include  <linux/bitops.h>
#include  <dt-bindings/clock/allwinner-h6-clock.h>


#define  ARRAY_V(idx, v)   [idx] = v 

#define  ALLWINNER_CLK_REG_INVALID           (0xffffffu)

#define  foreach_array_clk_ctrl    \
    ARRAY_V(ALLWINNER_PLL_CPUX,         0),       \
    ARRAY_V(ALLWINNER_PLL_DDR0,         0x10),       \
    ARRAY_V(ALLWINNER_PLL_PERI0,        0x20),       \
    ARRAY_V(ALLWINNER_PLL_PERI1,        0x28),       \
    ARRAY_V(ALLWINNER_PLL_GPU,          0x30),       \
    ARRAY_V(ALLWINNER_PLL_VIDEO0,       0x40),       \
    ARRAY_V(ALLWINNER_PLL_VIDEO1,       0x48),       \
    ARRAY_V(ALLWINNER_PLL_VE,           0x58),       \
    ARRAY_V(ALLWINNER_PLL_DE,           0x60),       \
    ARRAY_V(ALLWINNER_PLL_HSIC,         0x70),       \
    ARRAY_V(ALLWINNER_PLL_AUDIO,        0x78),       \
    ARRAY_V(ALLWINNER_CLK_CPUX_AXI,     0x500),       \
    ARRAY_V(ALLWINNER_CLK_PSI_AHB1_AHB2,         0x510),        \
    ARRAY_V(ALLWINNER_CLK_AHB3,         0x51c),        \
    ARRAY_V(ALLWINNER_CLK_APB1,         0x520),        \
    ARRAY_V(ALLWINNER_CLK_APB2,         0x524),        \
    ARRAY_V(ALLWINNER_CLK_MBUS,         0x540),        \
    ARRAY_V(ALLWINNER_CLK_NAND0,        0x810),        \
    ARRAY_V(ALLWINNER_CLK_NAND1,        0x814),        \
    ARRAY_V(ALLWINNER_CLK_SMHC0,        0x830),        \
    ARRAY_V(ALLWINNER_CLK_SMHC1,        0x834),        \
    ARRAY_V(ALLWINNER_CLK_SMHC2,        0x838),        \
    ARRAY_V(ALLWINNER_CLK_SPI0,         0x940),        \
    ARRAY_V(ALLWINNER_CLK_SPI1,         0x944),
    // ARRAY_V(ALLWINNER_CLK_SMHC2,        0x838),
    // ARRAY_V(ALLWINNER_CLK_SMHC2,        0x838),
    // ARRAY_V(ALLWINNER_CLK_SMHC2,        0x838),
    // ARRAY_V(ALLWINNER_CLK_SMHC2,        0x838),
    // ARRAY_V(ALLWINNER_CLK_SMHC2,        0x838),


// #define  foreach_array_clk_pattern0   
//     ARRAY_V(ALLWINNER_PLL_CPUX,         ALLWINNER_CLK_REG_INVALID),       
//     ARRAY_V(ALLWINNER_PLL_DDR0,         0x110),       
//     ARRAY_V(ALLWINNER_PLL_PERI0,        ALLWINNER_CLK_REG_INVALID),      
//     ARRAY_V(ALLWINNER_PLL_PERI1,        0x128),       
//     ARRAY_V(ALLWINNER_PLL_GPU,          0x130),       
//     ARRAY_V(ALLWINNER_PLL_VIDEO0,       0x140),       
//     ARRAY_V(ALLWINNER_PLL_VIDEO1,       0x148),       
//     ARRAY_V(ALLWINNER_PLL_VE,           0x158),       
//     ARRAY_V(ALLWINNER_PLL_DE,           0x160),       
//     ARRAY_V(ALLWINNER_PLL_HSIC,         0x170),       
//     ARRAY_V(ALLWINNER_PLL_AUDIO,        0x178),


#define  ALLWINNER_H6_PLLX_ENABLE              BIT(31)
#define  ALLWINNER_H6_PLLX_LOCK_ENABLE         BIT(29)
#define  ALLWINNER_H6_PLLX_LOCK                BIT(28)
#define  ALLWINNER_H6_PLLX_FACTOR_N            GENMASK(15,   8)





#endif


