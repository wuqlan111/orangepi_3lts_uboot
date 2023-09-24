
#ifndef  __ALLWINNER_H6_SPI_H__
#define  __ALLWINNER_H6_SPI_H__

#include <linux/bitops.h>


#define  ALLWINNER_SPI_GCR_SRST              BIT(31)
#define  ALLWINNER_SPI_GCR_TP_EN             BIT(7)
#define  ALLWINNER_SPI_GCR_MODE              BIT(1)
#define  ALLWINNER_SPI_GCR_EN                BIT(0)


#define  ALLWINNER_SPI_TCR_XCH                   BIT(31)
#define  ALLWINNER_SPI_TCR_SDDM                  BIT(14)
#define  ALLWINNER_SPI_TCR_SDM                   BIT(13)
#define  ALLWINNER_SPI_TCR_FBS                   BIT(12)
#define  ALLWINNER_SPI_TCR_SDC                   BIT(11)
#define  ALLWINNER_SPI_TCR_RPSM                  BIT(10)
#define  ALLWINNER_SPI_TCR_DDB                   BIT(9)
#define  ALLWINNER_SPI_TCR_DHB                   BIT(8)
#define  ALLWINNER_SPI_TCR_SS                    BIT(7)
#define  ALLWINNER_SPI_TCR_SS_OWNER              BIT(6)
#define  ALLWINNER_SPI_TCR_SS_SEL                GENMASK(5,  4)
#define  ALLWINNER_SPI_TCR_SSCTL                 BIT(3)
#define  ALLWINNER_SPI_TCR_SPOL                  BIT(2)
#define  ALLWINNER_SPI_TCR_CPOL                  BIT(1)
#define  ALLWINNER_SPI_TCR_CPHA                  BIT(0)

#define  ALLWINNER_SPI_ISR_TX_EMP                BIT(5)
#define  ALLWINNER_SPI_ISR_TX_RDY                BIT(4)
#define  ALLWINNER_SPI_ISR_RX_EMP                BIT(1)
#define  ALLWINNER_SPI_ISR_RX_RDY                BIT(0)

#define  ALLWINNER_SPI_FCR_TX_RST                BIT(31)
#define  ALLWINNER_SPI_FCR_TX_TEST               BIT(30)
#define  ALLWINNER_SPI_FCR_TX_DMA                BIT(24)
#define  ALLWINNER_SPI_FCR_TX_LEVEL              GENMASK(23,  16)
#define  ALLWINNER_SPI_FCR_RX_RST                BIT(15)
#define  ALLWINNER_SPI_FCR_RX_TEST               BIT(14)
#define  ALLWINNER_SPI_FCR_RX_DMA                BIT(8)
#define  ALLWINNER_SPI_FCR_RX_LEVEL              GENMASK(7,  0)


#define  ALLWINNER_SPI_FSR_TB_WR                BIT(31)
#define  ALLWINNER_SPI_FSR_TB_CNT               GENMASK(30,  28)
#define  ALLWINNER_SPI_FSR_TX_CNT               GENMASK(23,  16)
#define  ALLWINNER_SPI_FSR_RB_WR                BIT(15)
#define  ALLWINNER_SPI_FSR_RB_CNT               GENMASK(14,  12)
#define  ALLWINNER_SPI_FSR_RX_CNT               GENMASK(7,  0)


#define  ALLWINNER_SPI_WCR_SWC                 GENMASK(19,  16)
#define  ALLWINNER_SPI_WCR_WWC                 GENMASK(15,  0)

#define  ALLWINNER_SPI_CCR_DRS                 BIT(12)
#define  ALLWINNER_SPI_CCR_CDR1                GENMASK(11,  8)
#define  ALLWINNER_SPI_CCR_CDR2                GENMASK(7,  0)


#define  ALLWINNER_SPI_MBC_MBC                GENMASK(23,  0)
#define  ALLWINNER_SPI_MTC_MWTC               GENMASK(23,  0)


#define  ALLWINNER_SPI_BCC_QUAD               BIT(29)
#define  ALLWINNER_SPI_BCC_DRM                BIT(28)
#define  ALLWINNER_SPI_BCC_DBC                GENMASK(27,  24)
#define  ALLWINNER_SPI_BCC_STC                GENMASK(23,  0)



#endif
