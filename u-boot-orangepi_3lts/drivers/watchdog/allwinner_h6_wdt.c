

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <wdt.h>
#include <dm/device_compat.h>
#include <dm/device.h>
#include <asm/io.h>
#include <linux/bitops.h>

#define  ALLWINNER_WDT_IRQ_EN                   BIT(0)
#define  ALLWINNER_WDT_IRQ_PENDING              BIT(0)
#define  ALLWINNER_WDT_CTRL_KEY                 GENMASK(12,  1)
#define  ALLWINNER_WDT_CTRL_RESTART             BIT(0)
#define  ALLWINNER_WDT_CFG_CONFIG               GENMASK(1,   0)
#define  ALLWINNER_WDT_MODE_VALUE               GENMASK(7,   4)
#define  ALLWINNER_WDT_MODE_EN                  BIT(0)

#define  ALLWINNER_WDT_CTRL_KEY_FLAG            (0xa57ul<<1)


typedef  struct {
    u32  irq_en;
    u32  sr;
    u32  ctrl;
    u32  cfg;
    u32  mode;
} __attribute__((packed))  allwinner_h6_wdt_t;


typedef  struct {
    fdt_addr_t  base;
} allwinner_h6_wdt_plat_t;


static int allwinner_h6_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	allwinner_h6_wdt_plat_t *plat = dev_get_plat(dev);
    assert(plat);

    allwinner_h6_wdt_t * regs =  (allwinner_h6_wdt_t *)plat->base;

    ulong  time_s  =  timeout_ms / 1000;
    if (timeout_ms % 1000) {
        time_s++;
    }

    if ( time_s > 16 ) {
        dev_err(dev, "timeout_ms [%llu] don't support!", timeout_ms);
        return  -EINVAL;
    }

    uint32_t  inv_value = 0;

    if (time_s  < 1) {
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

    clrbits_32(&regs->mode,  ALLWINNER_WDT_MODE_EN);

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
	{ .compatible = "allwinner,H6-v200-wdt" },
	{ }
};

static int allwinner_h6_wdt_probe(struct udevice *dev)
{
    int32_t  ret  =  0;
    allwinner_h6_wdt_plat_t *plat = dev_get_plat(dev);
    assert(plat);

    plat->base  =   devfdt_get_addr(dev);

    if (plat->base ==  FDT_ADDR_T_NONE) {
        dev_err(dev, "get reg addr for dev [%s] failed!", dev->name);
        ret =  -EINVAL;
    }

	return  ret;
}

U_BOOT_DRIVER(allwinner_h6_wdt) = {
	.name = "allwinner_h6_wdt",
	.id = UCLASS_WDT,
	.of_match = allwinner_h6_wdt_ids,
	.probe = allwinner_h6_wdt_probe,
	.plat_auto = sizeof(allwinner_h6_wdt_plat_t),
	.ops = &allwinner_h6_wdt_ops,
};







