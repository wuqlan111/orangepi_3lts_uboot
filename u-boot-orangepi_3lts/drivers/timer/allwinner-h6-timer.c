#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <asm/io.h>
#include <linux/bitops.h>


#define  ALLWINNER_TIMERX_CTRL_MODE        BIT(7)
#define  ALLWINNER_TIMERX_CTRL_CLK_SRC     GENMASK(6,  4)
#define  ALLWINNER_TIMERX_CTRL_CLK_SRC     GENMASK(3,  2)
#define  ALLWINNER_TIMERX_CTRL_RELOAD      BIT(1)
#define  ALLWINNER_TIMERX_CTRL_EN          BIT(0)



typedef  struct {
    u32  ctrl;
    u32  intv_value;
    u32  cur_value;
} allwinner_h6_timer_t;


typedef  struct {
    fdt_addr_t  base;
    ulong clk_rate;
} allwinner_h6_timer_plat_t;


static u64 allwinner_h6_timer_get_count(struct udevice *dev)
{
	allwinner_h6_timer_plat_t * plat = dev_get_plat(dev);
    assert(plat  !=  NULL);

	allwinner_h6_timer_t *  regs =  (allwinner_h6_timer_t *)plat->base;

	return readl(&regs->cur_value);
}

static int allwinner_h6_timer_probe(struct udevice *dev)
{
	allwinner_h6_timer_plat_t * plat = dev_get_plat(dev);
    assert(plat  !=  NULL);

	allwinner_h6_timer_t *  regs =  (allwinner_h6_timer_t *)plat->base;

    writel(0xffffffff,  &regs->intv_value);
    writel(0,  &regs->ctrl);

    setbits_32(&regs->ctrl,  ALLWINNER_TIMERX_CTRL_RELOAD);

    while (readl(&regs->ctrl) & ALLWINNER_TIMERX_CTRL_RELOAD) ;

    setbits_32(&regs->ctrl,  ALLWINNER_TIMERX_CTRL_EN);

	return 0;
}



static const struct timer_ops allwinner_h6_timer_ops = {
	.get_count = allwinner_h6_timer_get_count,
};

static const struct udevice_id allwinner_h6_timer_ids[] = {
	{ .compatible = "allwinner,h6-v200-timer" },
	{}
};

U_BOOT_DRIVER(allwinner_h6_timer) = {
	.name	= "allwinner_h6_timer",
	.id	= UCLASS_TIMER,
	.of_match = allwinner_h6_timer_ids,
	// .of_to_plat = allwinner_h6_timer_of_to_plat,
	.plat_auto	= sizeof(allwinner_h6_timer_plat_t),
	.probe = allwinner_h6_timer_probe,
	.ops	= &allwinner_h6_timer_ops,
};


