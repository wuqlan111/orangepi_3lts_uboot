
#include  <common.h>
#include  <dm.h>
#include  <log.h>
#include  <asm/global_data.h>
#include  <asm/io.h>
#include  <errno.h>
#include  <mmc.h>
#include  <dm/device_compat.h>

#include "allwinner_smhc.h"



typedef  struct {
	u32  ctrl;
	u32  clkdiv;
	u32  tmout;
	u32  ctype;
	u32  blksiz;
	u32  bycnt;
	u32  cmd;
	u32  cmdarg;
	u32  respx[4],
	u32  intmask;
	u32  mintsts;
	u32  rinsts;
	u32  status;
	u32  fifoth;
	u32  funs;
	u32  tcbcnt;
	u32  tbbcnt;
	u32  csdc;
	u32  a12a;
	u32  ntsr;
	u32  sdbg;
	u32  emce;
	u32  emce_dbg;
	u32  rsv1[3];
	u32  hwrst;
	u32  rsv2;
	u32  dmac;
	u32  dlba;
	u32  idst;
	u32  idie;
	u32  rsv3[26];
	u32  thld;
	u32  rsv4[2];
	u32  edsd;
	u32  res_crc;
	u32  dx_crc[8];
	u32  crc_sta;
	u32  rsv5[2];
	u32  drv_dl;
	u32  smap_dl;
	u32  ds_dl;
	u32  rsv6;
	u32  emce_bmx[32];
	u32  rsv7[12];
	u32  fifo;
} allwinner_h6_smhc_t;

typedef  struct {
	struct  mmc_config  cfg;
	struct  mmc  mmc_info;
} allwinner_h6_smhc_plat_t;

typedef  struct {
	fdt_addr_t   base;
	uint32_t  sd_idx;
	struct mmc_config cfg;
} allwinner_h6_smhc_priv_t;

typedef enum {
	ALLWINNER_SMHC_FSM_IDLE  =  0,
	ALLWINNER_SMHC_FSM_INIT_SEQ,
	ALLWINNER_SMHC_FSM_TX_CMD_START,
	ALLWINNER_SMHC_FSM_TX_CMD,
	ALLWINNER_SMHC_FSM_TX_CMD_IDX,
	ALLWINNER_SMHC_FSM_TX_CMD_CRC,
	ALLWINNER_SMHC_FSM_TX_CMD_END,
	ALLWINNER_SMHC_FSM_RX_RSP_START,
	ALLWINNER_SMHC_FSM_RX_RSP_IRQ,
	ALLWINNER_SMHC_FSM_RX_RSP_TX,
	ALLWINNER_SMHC_FSM_RX_RSP_CMD_IDX,
	ALLWINNER_SMHC_FSM_RX_RSP_DATA,
	ALLWINNER_SMHC_FSM_RX_RSP_CRC,
	ALLWINNER_SMHC_FSM_RX_RSP_END,
	ALLWINNER_SMHC_FSM_WAIT_NCC,
	ALLWINNER_SMHC_FSM_WAIT,
} allwinner_smhc_fsm_state_t;





static  uint32_t  allwinner_get_commond_flag(struct mmc_cmd *cmd, struct mmc_data *data)
{
	uint32_t  flag  =  0;

	if (data) {
		flag |=  ALLWINNER_SMHC_CMD_DATA_TRANS | ALLWINNER_SMHC_CMD_STOP_AUTO;
		if (data->flag & MMC_DATA_WRITE) {
			flag |=  ALLWINNER_SMHC_CMD_TRANS_DIR;
		}

		flag |= ALLWINNER_SMHC_CMD_WAIT_TRANSFER;
	}

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		flag  |=  ALLWINNER_SMHC_CMD_RSP_RCV;
	}

	if (cmd->resp_type & MMC_RSP_CRC) {
		flag |=  ALLWINNER_SMHC_CMD_RSP_CRC;
	}

	if (cmd->resp_type & MMC_RSP_136) {
		flag |=  ALLWINNER_SMHC_CMD_LONG_RSP;
	}

	flag |= ALLWINNER_SMHC_CMD_LOAD;
	flag |= (cmd->cmdidx & ALLWINNER_SMHC_CMD_INDEX);

	return  flag;

}


static int32_t allwinner_smhc_poll_read_write(allwinner_h6_smhc_t * const smhc,
				struct mmc_data *data)
{
	int32_t  ret  =  0;
	const uint32_t  times  =  (data->blocksize * data->blocks) >> 2;
	const uint32_t  is_write  =  data->flags & MMC_DATA_READ? 0:  1;
	const uint32_t wait_mask = is_write ? BIT(3):  BIT(2);
	const uint32_t wait_bit  = 0;
	uint32_t * const buffer = (uint32_t *)(is_write ? data->src : data->dest );

	for (uint32_t i  =  0; i < times; i++) {
		if (wait_reg32_flag(&smhc->status, wait_mask, wait_bit, 60000)) {
			ret =  -1;
			break;
		}

		if (is_write) {
			writel(buffer[i], &smhc->fifo);
		} else {
			buffer[i]  =  readl(&smhc->fifo);
		}

	}

	return  ret;

}

static int32_t allwiner_smhc_set_ios_common(allwinner_h6_smhc_t * const smhc,
				    struct mmc *mmc)
{
	/* Change clock first */
	// if (mmc->clock && mmc_config_clock(priv, mmc) != 0) {
	// 	return -EINVAL;
	// }

	/* Change bus width */
	if (mmc->bus_width == 8) {
		writel(0x2, &smhc->ctype);		
	} else if (mmc->bus_width == 4) {
		writel(0x1, &smhc->ctype);		
	} else {
		writel(0x0, &smhc->ctype);		
	}

	return 0;
}


static int32_t allwiner_smhc_send_cmd_common(allwinner_h6_smhc_t * const smhc, struct mmc *mmc,
				 struct mmc_cmd *cmd, struct mmc_data *data)
{
	int32_t   ret  =  0;
	writel(0,  &smhc->intmask);
	writel(GENMASK(31,  0),  &smhc->rinsts);

	while (readl(&smhc->status) & ALLWINNER_SMHC_STATUS_CARD_BUSY) ;

	uint32_t  cmd_flag  =  allwinner_get_commond_flag(cmd, data);
	writel(cmd->cmdarg,  &smhc->cmdarg);
	writel(cmd_flag,  &smhc->cmd);

	if (!data && (cmd->resp_type & MMC_RSP_BUSY)) {
		uint32_t timeout = 600;

		/* Poll on DATA0 line for cmd with busy signal for 60 ms */
		while ( (timeout > 0) && (readl(&smhc->status) 
					& ALLWINNER_SMHC_STATUS_CARD_BUSY) ) {
			udelay(100);
			timeout--;
		}

		if (timeout <= 0) {
			dev_err("Timeout waiting for DAT0 to go high!\n");
			ret = -ETIMEDOUT;
			goto   error;
		}
	}

	uint32_t  rsp_len = cmd->resp_type & MMC_RSP_136 ? 4: 1;
	for (uint32_t i  = 0; i < rsp_len; i++) {
		cmd->response[i] = readl(&smhc->respx[i]);
	}


	if (data) {
		ret  =  allwinner_smhc_poll_read_write(smhc,  data);
	}

error:

	if (ret) {
		setbits_32(&smhc->ctrl,  ALLWINNER_SMHC_CTRL_SOFT_RST);

		while(readl(&smhc->ctrl) & ALLWINNER_SMHC_CTRL_SOFT_RST);
	}

	return  ret;

}


static int32_t allwiner_smhc_getcd_common(allwinner_h6_smhc_t * const smhc)
{
	int32_t  ret  =  readl(smhc->status) & ALLWINNER_SMHC_STATUS_CARD_PRESENT? 1:  0;
	return  ret;
}


static int32_t allwiner_smhc_wait_dat0_common(allwinner_h6_smhc_t * const smhc, int32_t state,
			       int32_t timeout_us)
{
	int32_t  ret  =  0;
	uint32_t  tmp  =  0;
	ret = readx_poll_timeout(readl, &smhc->status, tmp,
				!!(tmp & ALLWINNER_SMHC_STATUS_CARD_BUSY) == !!state,
				timeout_us);
	return ret;
}







#if !CONFIG_IS_ENABLED(DM_MMC)

static allwinner_h6_smhc_priv_t mmc_priv[CCU_SMHCX_MAX_ID+1];
static const char * const mmc_name[CCU_SMHCX_MAX_ID+1] = {
	"SMHC0",
	"SMHC1",
	"SMHC2",
};

static int sunxi_mmc_set_ios_legacy(struct mmc *mmc)
{
	allwinner_h6_smhc_priv_t * priv =  (allwinner_h6_smhc_priv_t *)mmc->priv;
	allwinner_h6_smhc_t * regs = (allwinner_h6_smhc_t *)priv->base;

	return  allwiner_smhc_set_ios_common(regs, mmc);
}

static int allwinner_mmc_send_cmd_legacy(struct mmc *mmc, struct mmc_cmd *cmd,
				     struct mmc_data *data)
{
	allwinner_h6_smhc_priv_t * priv =  (allwinner_h6_smhc_priv_t *)mmc->priv;
	allwinner_h6_smhc_t * regs = (allwinner_h6_smhc_t *)priv->base;
	return allwiner_smhc_send_cmd_common(regs, mmc, cmd, data);
}

static int32_t allwinner_mmc_core_init(struct mmc *mmc)
{
	allwinner_h6_smhc_priv_t * priv =  (allwinner_h6_smhc_priv_t *)mmc->priv;
	allwinner_h6_smhc_t * regs = (allwinner_h6_smhc_t *)priv->base;

	/* Reset controller */
	writel( BIT(8) | BIT(0), &regs->ctrl);
	udelay(1000);

	return 0;
}

static const struct mmc_ops allwinner_mmc_ops = {
	.send_cmd	=  allwinner_mmc_send_cmd_legacy,
	.set_ios	=  allwinner_mmc_set_ios_legacy,
	.init		=  allwinner_mmc_core_init,
};

static int32_t init_mmc_priv(const uint32_t sd_idx)
{
	if (sd_idx > CCU_SMHCX_MAX_ID) {
		return  -1;
	}

	memset(&mmc_priv[sd_idx],  0, sizeof(allwinner_h6_smhc_priv_t));
	mmc_priv[sd_idx].base  =  ALLWINNER_H6_SMHCX_REG(sd_idx);
	mmc_priv[sd_idx].sd_idx  =  sd_idx;
	struct mmc_config * cfg =  &mmc_priv[sd_idx].cfg;

	cfg->name  =  mmc_name[sd_idx];
	cfg->ops  =  &allwinner_mmc_ops;

	switch (sd_idx) {
		CCU_SMHC0_ID:
			cfg->caps = MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;
			cfg->voltages  =  MMC_VDD_32_33;
			break;

		CCU_SMHC1_ID:
			cfg->caps = MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;
			cfg->voltages  =  MMC_VDD_32_33 | MMC_VDD_165_195;
			break;


		CCU_SMHC2_ID:
			cfg->caps = MMC_MODE_4BIT | MMC_MODE_8BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;
			cfg->voltages  =  MMC_VDD_32_33 | MMC_VDD_165_195;
			break;
	}

	cfg->f_min = 400000;
	cfg->f_max = 52000000;

	return  0;

}


//初始化mmc设备
struct mmc * allwinner_mmc_init(int32_t sd_idx)
{
	struct mmc_config * cfg =  &mmc_priv[sd_idx].cfg;
	if ((sd_idx > CCU_SMHCX_MAX_ID)  || (sd_idx < 0)) {
		return  NULL;
	}

	memset(&mmc_priv[sd_idx],  0, sizeof(allwinner_h6_smhc_priv_t));

	if (init_mmc_priv(sd_idx)) {
		return  NULL;
	}

	if (mmc_clk_init(sd_idx,  24000000)) {
		return NULL;
	}
	
	return  mmc_create(cfg, &mmc_priv[sd_idx]);
}



#else


static int32_t allwinner_h6_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	allwinner_h6_smhc_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_smhc_priv_t * priv = dev_get_priv(dev);

	return allwiner_smhc_send_cmd_common(priv->base, &plat->mmc, cmd, data);
}

static int32_t allwinner_h6_set_ios(struct udevice *dev)
{
	allwinner_h6_smhc_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_smhc_priv_t * priv = dev_get_priv(dev);

	return allwiner_smhc_set_ios_common(priv->base, &plat->mmc);
}

static int32_t allwinner_h6_get_cd(struct udevice *dev)
{
	allwinner_h6_smhc_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_smhc_priv_t * priv = dev_get_priv(dev);

	return  allwiner_smhc_getcd_common(priv->base);
}


static int32_t allwinner_h6_wait_data0(struct udevice *dev, int32_t state,
			       int32_t timeout_us)
{
	allwinner_h6_smhc_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_smhc_priv_t * priv = dev_get_priv(dev);
	return  allwiner_smhc_wait_dat0_common(priv->base,  state, timeout_us);
}

static const struct udevice_id allwinner_h6_smhc_match[] = {
	{ .compatible = "allwinner, h6-v200-smhc" },
	{ }
};

static const struct dm_mmc_ops allwinner_h6_smhc_ops = {
	// .reinit  =  ,
	.send_cmd = allwinner_h6_send_cmd,
	.set_ios = allwinner_h6_set_ios,
	.get_cd  =  allwinner_h6_get_cd,
	.wait_data0 = allwinner_h6_wait_data0,

};



static int32_t allwinner_h6_smhc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	allwinner_h6_smhc_plat_t *plat = dev_get_plat(dev);
	allwinner_h6_smhc_priv_t *priv = dev_get_priv(dev);

	upriv->mmc  =  &plat->mmc_info;
	plat->mmc_info.cfg = &plat->cfg;
	plat->mmc_info.dev = dev;

	return 0;
}



static int32_t allwinner_h6_smhc_bind(struct udevice *dev)
{
	allwinner_h6_smhc_plat_t * plat = dev_get_plat(dev);
	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

U_BOOT_DRIVER(allwinner_h6_smhc) = {
	.name = "allwinner, h6-v200-smhc",
	.id = UCLASS_MMC,
	.of_match = allwinner_smhc_match,
	.bind = allwinner_h6_smhc_bind,
	.probe = allwinner_h6_smhc_probe,
	.priv_auto	= sizeof(allwinner_h6_smhc_priv_t),
	.plat_auto	= sizeof(allwinner_h6_smhc_plat_t),
	.ops = &allwinner_smhc_ops,
};


#endif


