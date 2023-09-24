#include  <common.h>
#include  <dm.h>
#include  <rtc.h>
#include  <asm/io.h>
#include  <linux/bitops.h>

#define  ALLWINNER_LOSC_CTRL_KEY               GENMASK(31,  16)
#define  ALLWINNER_LOSC_CTRL_AUTO_SWT          BIT(14)
#define  ALLWINNER_LOSC_CTRL_ALM_DD            BIT(9)
#define  ALLWINNER_LOSC_CTRL_RTC_HH            BIT(8)
#define  ALLWINNER_LOSC_CTRL_RTC_YY            BIT(7)

#define  ALLWINNER_INTOSC__CLK_PRESCAL         GENMASK(15,  0)

#define  ALLWINNER_YYMMDD_LEAP                 BIT(22)
#define  ALLWINNER_YYMMDD_YEAR                 GENMASK(21,  16)
#define  ALLWINNER_YYMMDD_MONTH                GENMASK(11,  8)
#define  ALLWINNER_YYMMDD_DAY                  GENMASK(4,   0)

#define  ALLWINNER_HHMMSS_WKNO                 GENMASK(31,  29)
#define  ALLWINNER_HHMMSS_HOUR                 GENMASK(20,  16)
#define  ALLWINNER_HHMMSS_MINUTE               GENMASK(13,  8)
#define  ALLWINNER_HHMMSS_SECOND               GENMASK(5,   0)



typedef  struct  {
    u32  losc_ctrl;
    u32  losc_auto_swt;
    u32  intosc_clk_prescal;
    u32  intosc_clk_cali;
    u32  yy_mm_dd;
    u32  hh_mm_ss;
} allwinner_h6_rtc_t;


typedef  struct {
    fdt_addr_t  base;
    ulong  year_base;
} allwinner_h6_rtc_plat_t;



static int allwinner_h6_rtc_get(struct udevice *dev, struct rtc_time *time)
{
	allwinner_h6_rtc_plat_t  * plat  = dev_get_plat(dev);
    assert(plat != NULL);
    assert(time != NULL);

    allwinner_h6_rtc_t * regs  =  (allwinner_h6_rtc_t * )plat->base;

    uint32_t  yymmdd  =  readl(&regs->yy_mm_dd);
    uint32_t  ddmmss  =  readl(&regs->hh_mm_ss);

    time->tm_year   =   ((yymmdd & ALLWINNER_YYMMDD_YEAR) >> 16) + plat->year_base;
    time->tm_mon    =   (yymmdd & ALLWINNER_YYMMDD_MONTH) >> 8;
    time->tm_mday   =    yymmdd & ALLWINNER_YYMMDD_DAY;

    time->tm_wday   =   (ddmmss & ALLWINNER_HHMMSS_WKNO) >> 29;
    time->tm_hour   =   (ddmmss & ALLWINNER_HHMMSS_HOUR) >> 16;
    time->tm_min    =   (ddmmss & ALLWINNER_HHMMSS_MINUTE) >> 8;
    time->tm_sec    =   ddmmss & ALLWINNER_HHMMSS_SECOND;

    time->tm_yday   =  0;
    time->tm_isdst  =  0;

	return  0;
}

static int allwinner_h6_rtc_set(struct udevice *dev, const struct rtc_time *time)
{
	allwinner_h6_rtc_plat_t  * plat  = dev_get_plat(dev);
    assert(plat != NULL);
    assert(time != NULL);

    allwinner_h6_rtc_t * regs  =  (allwinner_h6_rtc_t * )plat->base;

    uint32_t  year  =  time->tm_year  %  50;
    uint32_t  is_leap_year   =  !(time->tm_year % 400) ||  
                        (!(time->tm_year % 4) && (time->tm_year % 100)) ? 1:  0;
    plat->year_base   =  time->tm_year -  year;

    uint32_t  yymmdd  =  (is_leap_year << 22) | (year << 16) | 
                                    (time->tm_mon << 8) | time->tm_mday;

    uint32_t  hhmmss  =  (time->tm_wday << 29) | (time->tm_hour << 16) | 
                                (time->tm_min << 8) | time->tm_sec;

    writel(yymmdd,  &regs->yy_mm_dd);
    writel(hhmmss,  &regs->hh_mm_ss);

	return 0;
}



static int allwinner_h6_rtc_probe(struct udevice *dev)
{
    struct clk clk = {0};
	allwinner_h6_rtc_plat_t  * plat  = dev_get_plat(dev);
    assert(plat != NULL);

    plat->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE) {
		return -EINVAL;        
    }

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret) {
		return ret;        
    }

	ret = clk_enable(&clk);
	if (ret) {
		clk_free(&clk);
	}

	return ret;
}


static const struct rtc_ops allwinner_h6_rtc_ops = {
	.get = allwinner_h6_rtc_get,
	.set = allwinner_h6_rtc_set,
	// .reset = allwinner_h6_rtc_reset,
};

static const struct udevice_id allwinner_h6_rtc_ids[] = {
	{ .compatible = "allwinner,h6-rtc" },
	{ }
};

U_BOOT_DRIVER(rtc_allwinner_h6) = {
	.name	= "rtc-allwinner-h6",
	.id	= UCLASS_RTC,
	.probe	= allwinner_h6_rtc_probe,
	.of_match = allwinner_h6_rtc_ids,
	.ops	= &allwinner_h6_rtc_ops,
	.plat_auto	= sizeof(allwinner_h6_rtc_plat_t),
};


