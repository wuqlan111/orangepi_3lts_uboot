
#ifndef __DT_BINDINGS_CLOCK_ALLWINNER_H6_H
#define __DT_BINDINGS_CLOCK_ALLWINNER_H6_H

#define  CLK_NONE			0

#define  ALLWINNER_CLK(module)   ALLWINNER_CLK_##module
#define  ALLWINNER_PLL(module)   ALLWINNER_PLL_##module

// pll clk
#define  ALLWINNER_PLL_CPUX     0
#define  ALLWINNER_PLLDDR0      1
#define  ALLWINNER_PLLPERI0     2
#define  ALLWINNER_PLLPERI1     3
#define  ALLWINNER_PLLGPU       4
#define  ALLWINNER_PLLVIDEO0    5
#define  ALLWINNER_PLLVIDEO1    6
#define  ALLWINNER_PLLVE        7
#define  ALLWINNER_PLLDE        8
#define  ALLWINNER_PLLHSIC      9
#define  ALLWINNER_PLLAUDIO     10

// clk
#define  ALLWINNER_CLK_CPUX_AXI                  11
#define  ALLWINNER_CLK_CPUX_APB                  12
#define  ALLWINNER_CLK_PSI_AHB1_AHB2             13
#define  ALLWINNER_CLK_AHB3                      14
#define  ALLWINNER_CLK_APB1                      15
#define  ALLWINNER_CLK_APB2                      16
#define  ALLWINNER_CLK_MBUS                      17
#define  ALLWINNER_CLK_NAND0                     18
#define  ALLWINNER_CLK_NAND1                     19
#define  ALLWINNER_CLK_SMHC0                     20
#define  ALLWINNER_CLK_SMHC1                     21
#define  ALLWINNER_CLK_SMHC2                     22
#define  ALLWINNER_CLK_SPI0                      23
#define  ALLWINNER_CLK_SPI1                      24
#define  ALLWINNER_CLK_TS                        25
#define  ALLWINNER_CLK_CIR_TX                    26
#define  ALLWINNER_CLK_I2S_PCM3                  27
#define  ALLWINNER_CLK_I2S_PCM0                  28
#define  ALLWINNER_CLK_I2S_PCM1                  29
#define  ALLWINNER_CLK_I2S_PCM2                  30
#define  ALLWINNER_CLK_OWA                       31
#define  ALLWINNER_CLK_DMIC                      32
#define  ALLWINNER_CLK_AUDIO_HUB                 33
#define  ALLWINNER_CLK_USB0                      34
#define  ALLWINNER_CLK_USB1                      35
#define  ALLWINNER_CLK_USB3                      36
#define  ALLWINNER_CLK_PCIE_REF                  37
#define  ALLWINNER_CLK_PCIE_MAXI                 38
#define  ALLWINNER_CLK_PCIE_AUX                  39
#define  ALLWINNER_CLK_HDMI                      40
#define  ALLWINNER_CLK_HDMI_SLOW                 41
#define  ALLWINNER_CLK_HDMI_CEC                  42
#define  ALLWINNER_CLK_HDMI_HDCP                 43
#define  ALLWINNER_CLK_TCON_LED                  44
#define  ALLWINNER_CLK_TCON_TV                   45
#define  ALLWINNER_CLK_CSI_MISC                  46
#define  ALLWINNER_CLK_CSI_TOP                   47
#define  ALLWINNER_CLK_CSI_MASTER                48

#define  ALLWINNER_PLL_MAX         ALLWINNER_PLL_AUDIO
#define  ALLWINNER_CLK_UARTX       ALLWINNER_CLK_APB2


#endif

