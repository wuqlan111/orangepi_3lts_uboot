#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <dm/device.h>
#include <dm/device_compat.h>

typedef  struct {
    u32  ctrl;
    u32  cnt_low;
    u32  cnt_high;
} __attribute__((packed)) allwinner_h6_timer_t;


typedef  struct {
    fdt_addr_t  base;
    ulong clk_rate;
} allwinner_h6_timer_plat_t;


static uint64_t allwinner_h6_timer_get_count(struct udevice *dev)
{
	allwinner_h6_timer_plat_t * plat = dev_get_plat(dev);
    assert(plat  !=  NULL);

	allwinner_h6_timer_t *  regs =  (allwinner_h6_timer_t *)plat->base;

	uint32_t  low = readl(&regs->cnt_low);
	uint32_t  high  =  readl(&regs->cnt_high);
	uint64_t  cnt  =  ( (uint64_t)high << 32 ) | low;

	return  cnt;
}

static int allwinner_h6_timer_probe(struct udevice *dev)
{
	allwinner_h6_timer_plat_t * plat = dev_get_plat(dev);
	struct timer_dev_priv * uc_priv = dev_get_uclass_priv(dev);

    assert(plat  !=  NULL);

	plat->clk_rate = uc_priv->clock_rate = 24000000;
	plat->base  = dev_read_addr(dev);

	return 0;
}


static const struct timer_ops allwinner_h6_timer_ops = {
	.get_count = allwinner_h6_timer_get_count,
};

static const struct udevice_id allwinner_h6_timer_ids[] = {
	{ .compatible = "allwinner,H6-v200-counter64" },
	{}
};

U_BOOT_DRIVER(allwinner_h6_counter) = {
	.name	= "allwinner_h6_counter",
	.id	= UCLASS_TIMER,
	.of_match = allwinner_h6_timer_ids,
	.plat_auto	= sizeof(allwinner_h6_timer_plat_t),
	.probe = allwinner_h6_timer_probe,
	.ops	= &allwinner_h6_timer_ops,
	.flags   =   DM_FLAG_PRE_RELOC,
};


