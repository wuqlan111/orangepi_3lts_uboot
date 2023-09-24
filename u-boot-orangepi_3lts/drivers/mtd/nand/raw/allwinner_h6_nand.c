#include <common.h>
#include <log.h>
#include <dm/device_compat.h>
#include <nand.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/mtd/rawnand.h>
#include <asm/io.h>
#include "allwinner_h6_nand.h"


typedef  struct {
    u32  ctrl;
    u32  st;
    u32  int_ctrl;
    u32  timing_ctrl;
    u32  timing_cfg;
    u32  addr_low;
    u32  addr_high;
    u32  data_blk_mask;
    u32  cnt;
    u32  cmd;
    u32  rcmd_set;
    u32  wcmd_set;
    u32  rsv1;
    u32  ecc_ctrl;
    u32  ecc_st;
    u32  data_pat_sta;
    u32  efr;
    u32  rdata_sta_ctrl;
    u32  rdata_sta0;
    u32  rdata_sta1;
    u32  err_cntx[8];
    u32  user_data_lenx[4];
    u32  user_datax[32];
    u32  efnand_sta;
    u32  spare_are;
    u32  pat_id;
    u32  ddr2_spec_ctrl;
    u32  ndma_mode_ctrl;
    u32  mdma_dlba;
    u32  mdma_sta;
    u32  dma_int_mask;
    u32  mdma_cur_desc_addr;
    u32  mdma_cur_buf_addr;
    u32  dma_cnt;
    u32  emce_ctrl;
    u32  emce_iv_fac_cmp_val;
    u32  emce_iv_cal_factor[32];
    u32  rsv2[24];
    u32  io_data;
} allwinner_h6_ndfc_t;


typedef struct {
	struct nand_chip chip;
	struct udevice *dev;
    fdt_addr_t  base;
} allwinner_h6_nand_info_t;




static void allwinner_h6_nand_select_chip(struct mtd_info *mtd, int32_t chip)
{
	struct nand_chip * nand = mtd_to_nand(mtd);
	allwinner_h6_nand_info_t * nand_info = nand_get_controller_data(nand);
    allwinner_h6_ndfc_t * regs  =  (allwinner_h6_ndfc_t * )nand_info->base;

    if ( (chip < 0) || (chip > 15) ) {
        dev_warn(mtd->dev, "chip_sel [%d] invalid", chip);
        return;
    }

    uint32_t  flag  =  chip << 24;
    clrsetbits_32(&regs->ctrl, ALLWINNER_NDFC_CTL_CE_SEL, flag);

}


static int32_t allwinner_h6_nand_device_ready(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	allwinner_h6_nand_info_t * nand_info = nand_get_controller_data(nand);
    allwinner_h6_ndfc_t * regs  =  (allwinner_h6_ndfc_t * )nand_info->base;
	uint32_t flag =  readl(&regs->st);

	return  !!(flag & ALLWINNER_NDFC_ST_STA);
}

static void allwinner_h6_nand_cmd_ctrl(struct mtd_info *mtd, int32_t data, uint32_t ctrl)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	allwinner_h6_nand_info_t * nand_info = nand_get_controller_data(nand);
    allwinner_h6_ndfc_t * regs  =  (allwinner_h6_ndfc_t * )nand_info->base;

    uint32_t  flag  =  0;

    if ( !(ctrl & NAND_ALE )) {
        flag  |=  ALLWINNER_NDFC_CTL_ALE_POL;
    }

    if (!(ctrl & NAND_CLE) ) {
        flag  |=  ALLWINNER_NDFC_CTL_CLE_POL;
    }

    uint32_t  mask  =  ALLWINNER_NDFC_CTL_CLE_POL | ALLWINNER_NDFC_CTL_ALE_POL;
    clrsetbits_32(&regs->ctrl,  mask,  flag);

}





int32_t mxs_nand_init_ctrl(allwinner_h6_nand_info_t *nand_info)
{
    int32_t  ret  =  0;
    struct  nand_chip * nand  =  nand_info->chip;
    struct  mtd_info * mtd  =  &nand_info->chip.mtd;

    if (nand_scan_ident(mtd, CONFIG_SYS_MAX_NAND_DEVICE, NULL)) {
        return  -1;
    }

    if (nand_scan_tail(mtd)) {
        return  -1;        
    }

    ret = nand_register(0, mtd);
	return  ret;

}


static int32_t  allwinner_h6_nand_probe(struct udevice *dev)
{
    int32_t   ret  =  0;
    allwinner_h6_nand_info_t *info = (allwinner_h6_nand_info_t *)dev_get_priv(dev);
    nand_chip * nand  = &info->chip;
    info->dev  = dev;
    nand_set_controller_data(nand,  info);


    return  ret;

}



static const struct udevice_id allwinner_h6_nand_ids[] = {
	{
		.compatible = "allwinner, h6-v200-nand",
	},
	{ /* sentinel */ }
};



U_BOOT_DRIVER(allwinner_h6_nand) = {
	.name = "allwinner_h6_nand",
	.id = UCLASS_MTD,
	.of_match = allwinner_h6_nand_ids,
	.probe = allwinner_h6_nand_probe,
	.priv_auto	= sizeof(allwinner_h6_nand_info_t),
};

void board_nand_init(void)
{
	struct udevice *dev = NULL;
	int32_t ret = 0;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(allwinner_h6_nand), &dev);
    if (ret && ret != -ENODEV) {
		pr_err("Failed to initialize %s. (error %d)\n", dev->name,
		       ret);        
    }

}






