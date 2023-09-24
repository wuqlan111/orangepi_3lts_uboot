

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <wdt.h>
#include <dm/device_compat.h>
#include <asm/io.h>
#include <linux/bitops.h>


#define  ALLWINNER_WDT_IRQ_EN                   BIT(0)
#define  ALLWINNER_WDT_IRQ_PENDING              BIT(0)
#define  ALLWINNER_WDT_CTRL_KEY                 GENMASK(12,  1)
#define  ALLWINNER_WDT_CTRL_RESTART             BIT(0)
#define  ALLWINNER_WDT_CFG_CONFIG               GENMASK(1,   0)
#define  ALLWINNER_WDT_MODE_VALUE               GENMASK(7,   4)
#define  ALLWINNER_WDT_MODE_EN                  BIT(0)

#define  ALLWINNER_WDT_CTRL_KEY_FLAG            (0xa57<<1u)


typedef  struct {
    u32  irq_en;
    u32  sr;
    u32  ctrl;
    u32  cfg;
    u32  mode;
} allwinner_h6_wdt_t;



typedef  struct {
    fdt_add_t  base;
} allwinner_h6_wdt_plat_t;


static int allwinner_h6_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	allwinner_h6_wdt_plat_t *plat = dev_get_plat(dev);
    assert(plat);

    allwinner_h6_wdt_t * regs =  (allwinner_h6_wdt_t *)plat->base;

    double  timeout_s  =  timeout_ms / 1000;
    ulong  time_s  =  (ulong) timeout_s;
    if ((double)time_s !=  timeout_s) {
        time_s++;
    }

    if ( (timeout_s < 0.5) || (timeout_s > 16) ) {
        dev_err(dev, "timeout_ms [%ul] don't support!", timeout_ms);
        return  -EINVAL;
    }

    uint32_t  inv_value = 0;

    if (timeout_s  ==  0.5) {
        inv_value  =  0;
    } else if (time_s <= 6) {
        inv_value  =  time_s;
    } else {
        uint32_t delta  =  (time_s - 6) >> 1;
        inv_value  =  6 + delta + (delta & 0x1);
    }
	
    uint32_t  flag  =  (inv_value << 4) | ALLWINNER_WDT_MODE_EN;
    writel(flag,  &regs->mode);

	return 0;
}


static int allwinner_h6_wdt_stop(struct udevice *dev)
{
	allwinner_h6_wdt_plat_t *plat = dev_get_plat(dev);
    assert(plat);

    allwinner_h6_wdt_t * regs =  (allwinner_h6_wdt_t *)plat->base;

	return 0;
}


static int allwinner_h6_wdt_reset(struct udevice *dev)
{
	allwinner_h6_wdt_plat_t *plat = dev_get_plat(dev);
    assert(plat);

    allwinner_h6_wdt_t * regs =  (allwinner_h6_wdt_t *)plat->base;

    uint32_t  flag  =  ALLWINNER_WDT_CTRL_KEY_FLAG | ALLWINNER_WDT_CTRL_RESTART;

    writel(flag,  &regs->ctrl);

	return 0;
}


static const struct wdt_ops allwinner_h6_wdt_ops = {
	.start = allwinner_h6_wdt_start,
	.reset = allwinner_h6_wdt_reset,
	.stop = allwinner_h6_wdt_stop,
};


static const struct udevice_id allwinner_h6_wdt_ids[] = {
	{ .compatible = "allwinner,h6-v200-wdt" },
	{ }
};

static int allwinner_h6_wdt_probe(struct udevice *dev)
{
    allwinner_h6_wdt_plat_t *plat = dev_get_plat(dev);
    assert(plat);

    plat->base  =   devfdt_get_addr(dev);

    if (plat->base ==  FDT_ADDR_T_NONE) {
        dev_err(dev, "get reg addr for dev [%s] failed!", dev->name);
        return  -EINVAL;
    }

	return 0;
}

U_BOOT_DRIVER(allwinner_h6_wdt) = {
	.name = "allwinner_h6_wdt",
	.id = UCLASS_WDT,
	.of_match = allwinner_h6_wdt_ids,
	.probe = allwinner_h6_wdt_probe,
	.plat_auto = sizeof(struct allwinner_h6_wdt_priv),
	.ops = &allwinner_h6_wdt_ops,
};







