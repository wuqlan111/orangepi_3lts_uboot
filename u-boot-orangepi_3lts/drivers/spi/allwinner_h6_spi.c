
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <fdtdec.h>
#include <dm/device_compat.h>
#include <linux/types.h>
#include <spi.h>
#include <asm/io.h>

#include  "allwinner_h6_spi.h"

typedef  struct {
    u32  gcr;
    u32  tcr;
    u32  rsv1[1];
    u32  ier;
    u32  isr;
    u32  fcr;
    u32  fsr;
    u32  wcr;
    u32  ccr;
    u32  rsv2[2];
    u32  mbs;
    u32  mtc;
    u32  bcc;
    u32  batcr;
    u32  ccr_3w;
    u32  tbr;
    u32  rbr;
    u32  dma_ctl;
    u32  txd;
    u32  rxd;
} allwinner_h6_spi_t;


typedef  struct {
    fdt_addr_t  base;
    ulong   clk_rate;
} allwinner_h6_spi_plat_t;


static int allwinner_h6_spi_claim_bus(struct udevice * slave)
{
	struct udevice *bus = slave->parent;
    assert(bus != NULL);

	allwinner_h6_spi_plat_t * plat = dev_get_plat(bus);
    assert(plat !=  NULL);

    allwinner_h6_spi_t * regs  = (allwinner_h6_spi_t *)plat->base;

    setbits_32(&regs->gcr,  ALLWINNER_SPI_GCR_EN | ALLWINNER_SPI_GCR_MODE);

	return 0;
}


static int allwinner_h6_spi_spi_release_bus(struct udevice * slave)
{
	struct udevice *bus = slave->parent;
    assert(bus != NULL);

	allwinner_h6_spi_plat_t * plat = dev_get_plat(bus);
    assert(plat !=  NULL);

    allwinner_h6_spi_t * regs  = (allwinner_h6_spi_t *)plat->base;

    clrbits_32(&regs->gcr,  ALLWINNER_SPI_GCR_EN | ALLWINNER_SPI_GCR_MODE);

	return  0;
}


int allwinner_h6_spi_spi_set_speed(struct udevice * bus, uint speed)
{
	allwinner_h6_spi_plat_t * plat = dev_get_plat(bus);
    assert(plat !=  NULL);

    allwinner_h6_spi_t * regs  = (allwinner_h6_spi_t *)plat->base;

	if (!speed) {
		dev_err(bus, "speed is zero!");
		return  -EINVAL;
	}

	uint32_t  select_src2 =  0;
	double  divisior  =  plat->clk_rate  / speed;
	uint32_t  div  =  (uint32_t)divisior;
	if ((double)div  !=  divisior) {
		div++;
	}

	if (div > 0x200) {
		dev_err(bus, "speed [%u] too small!", speed);
		return  -EINVAL;
	}

	if ((div >> 1)  > 0xf) {
		select_src2  =  1;
	}

	uint32_t  new_div  =  select_src2 ? (div >> 1) - 1  :  div >> 1;
	uint32_t  flag  =  select_src2 << 12;
	uint32_t  mask  =  ALLWINNER_SPI_CCR_DRS;

	if (select_src2) {
		flag  |=  new_div ;
		mask  |=  ALLWINNER_SPI_CCR_CDR2;
	} else {
		flag  |=  new_div << 8 ;
		mask  |=  ALLWINNER_SPI_CCR_CDR1;
	}

	clrsetbits_32(&regs->ccr,  mask,  flag);

	return ret;
}


static int allwinner_h6_spi_spi_set_mode(struct udevice *bus, uint mode)
{
	allwinner_h6_spi_plat_t * plat = dev_get_plat(bus);
    assert(plat !=  NULL);

    allwinner_h6_spi_t * regs  = (allwinner_h6_spi_t *)plat->base;

	uint32_t   flag,  mask;
	flag  = mask  =  0;

	if (mode & SPI_CPHA) {
		flag |= ALLWINNER_SPI_TCR_CPHA;
	}

	if (mode & SPI_CPOL) {
		flag |= ALLWINNER_SPI_TCR_CPOL;
	}

	if (mode & SPI_LSB_FIRST) {
		flag  |=  ALLWINNER_SPI_TCR_FBS;
	}

	mask  =   ALLWINNER_SPI_TCR_CPHA | ALLWINNER_SPI_TCR_CPOL | ALLWINNER_SPI_TCR_FBS;
	clrsetbits_32(regs->tcr, mask, flag);

	if (mode & (SPI_TX_QUAD | SPI_RX_QUAD)) {
		flag  =  ALLWINNER_SPI_BCC_QUAD;
	} else if (mode & (SPI_TX_DUAL | SPI_RX_DUAL)) {
		flag  =  ALLWINNER_SPI_BCC_DRM;
	} else {
		flag  =  0;
	}

	mask   =   ALLWINNER_SPI_BCC_QUAD | ALLWINNER_SPI_BCC_DRM;

	clrsetbits_32(&regs->bcc,  mask,  flag);

	return 0;
}



static int allwinner_h6_spi_spi_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	allwinner_h6_spi_plat_t * plat = dev_get_plat(bus);
    assert(plat !=  NULL);

    allwinner_h6_spi_t * regs  = (allwinner_h6_spi_t *)plat->base;

	uint32_t bytes = bitlen / 8;
	const uint8_t * txp = dout;
	uint8_t * rxp = din;
	uint32_t reg, data, start;

	if (bitlen == 0) {
		return  0;
	}

	/* empty read buffer */
	if (readl(&regs->isr) & ALLWINNER_SPI_ISR_RX_RDY) {
		readl(&regs->rxd);
	}






	return 0;
}



static const struct dm_spi_ops allwinner_h6_spi_spi_ops = {
	.claim_bus	= allwinner_h6_spi_spi_claim_bus,
	.release_bus = allwinner_h6_spi_spi_release_bus,
	.xfer = allwinner_h6_spi_spi_xfer,
	.set_speed	= allwinner_h6_spi_spi_set_speed,
	.set_mode	= allwinner_h6_spi_spi_set_mode,
};

static const struct udevice_id allwinner_h6_spi_spi_ids[] = {
	{ .compatible = "allwinner,  h6-v200-spi" },
	{ .compatible = "allwinner,  h6-spi"},
};

U_BOOT_DRIVER(allwinner_h6_spi_spi) = {
	.name	= "allwinner_h6_spi_spi",
	.id	= UCLASS_SPI,
	.of_match = allwinner_h6_spi_spi_ids,
	.ops = &allwinner_h6_spi_spi_ops,
	.of_to_plat = allwinner_h6_spi_spi_of_to_plat,
	.plat_auto	= sizeof(allwinner_h6_spi_plat_t),
	// .priv_auto	= sizeof(struct allwinner_h6_spi_spi_priv),
	.probe	= allwinner_h6_spi_spi_probe,
};










