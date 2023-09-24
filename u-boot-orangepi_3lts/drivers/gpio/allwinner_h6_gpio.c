
#include <config.h>
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <linux/io.h>
#include <asm/gpio.h>

#include  "allwinner_h6_gpio.h"

typedef struct {
    u32  cfgx[4];
    u32  dat;
    u32  drvx[2];
    u32  pullx[2];
} allwinner_h6_gpio_config_t;


typedef struct {
    u32  cfgx[4];
    u32  ctl;
    u32  sta;
    u32  deb;
} allwinner_h6_gpio_interrupt_t;


typedef struct {
    fdt_addr_t  config_base;
    fdt_addr_t  interrupt_base;
    uint32_t   pins:8;
} allwinner_h6_gpio_plat_t;


static int allwinner_h6_gpio_direction_input(struct udevice *dev, unsigned pin)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (pin > ALLWINNER_H6_GPIO_MAX_PIN) {
		return -EINVAL;
	}

	uint32_t  offset  =  pin  >> 3;
	uint32_t  shift   =  (pin % 8) << 2;

	clrsetbits_32(&config_regs->cfgx[offset],  0x7 << shift, 
									ALLWINNER_H6_GPIO_INPUT << shift);

	return  0;
}


static int allwinner_h6_gpio_direction_output(struct udevice *dev, unsigned pin,
				     int val)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (pin > ALLWINNER_H6_GPIO_MAX_PIN) {
		return -EINVAL;
	}

	if (val) {
		setbits_32(&config_regs->dat,  1 << pin);
	} else {
		clrbits_32(&config_regs->dat,  1 << pin);
	}

	uint32_t  offset  =  pin  >> 3;
	uint32_t  shift   =  (pin % 8) << 2;

	clrsetbits_32(&config_regs->cfgx[offset],  0x7 << shift, 
									ALLWINNER_H6_GPIO_OUTPUT << shift);
	return 0;
}


static int allwinner_h6_gpio_get_value(struct udevice *dev, unsigned pin)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (pin > ALLWINNER_H6_GPIO_MAX_PIN) {
		return -EINVAL;
	}

	return  readl(&config_regs->dat) & (1 << pin)? 1: 0;
}


static int allwinner_h6_gpio_set_value(struct udevice *dev, unsigned pin, int val)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (pin > ALLWINNER_H6_GPIO_MAX_PIN) {
		return -EINVAL;
	}

	if (val) {
		setbits_le32(&config_regs->dat,  1 << pin);		
	} else {
		clrbits_le32(&config_regs->dat,  1 << pin);
	}

	return 0;
}


static int allwinner_h6_gpio_get_function(struct udevice *dev, uint32_t  pin)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (pin > ALLWINNER_H6_GPIO_MAX_PIN) {
		return -EINVAL;
	}

	uint32_t  offset  =  pin  >> 3;
	uint32_t  shift   =  (pin % 8) << 2;
	uint32_t  pin_func  =  (readl(&config_regs->cfgx[offset]) >> shift) & 0x7;

	if (pin_func ==  ALLWINNER_H6_GPIO_OUTPUT) {
		return  GPIOF_OUTPUT;
	} else if (pin_func ==  ALLWINNER_H6_GPIO_INPUT ) {
		return  GPIOF_INPUT;
	} else if (pin_func == ALLWINNER_H6_GPIO_OUTPUT) {
		return  GPIOF_UNUSED;
	}

	return GPIOF_FUNC;
}


static const struct dm_gpio_ops allwinner_h6_gpio_ops = {
	.direction_input	= allwinner_h6_gpio_direction_input,
	.direction_output	= allwinner_h6_gpio_direction_output,
	.get_value		= allwinner_h6_gpio_get_value,
	.set_value		= allwinner_h6_gpio_set_value,
	.get_function   = allwinner_h6_gpio_get_function,
};


static int allwinner_h6_gpio_probe(struct udevice *dev)
{

	return 0;
}

static int allwinner_h6_gpio_of_to_plat(struct udevice *dev)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);

	assert(plat != NULL);

	plat->config_base  =  dev_read_addr_index(dev,  0);
	plat->interrupt_base  =  dev_read_addr_index(dev,  1);

	// plat->gpio_count = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
	// 	"altr,gpio-bank-width", 32);
	// plat->bank_name = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
	// 	"gpio-bank-name", NULL);

	return 0;
}



static const struct udevice_id allwinner_h6_gpio_ids[] = {
	{ .compatible = "allwinner, H6-v200-gpio" , .data = 0},
};

U_BOOT_DRIVER(allwinner_h6_gpio) = {
	.name		= "allwinner_h6_gpio",
	.id		= UCLASS_GPIO,
	.of_match	= allwinner_h6_gpio_ids,
	.ops		= &allwinner_h6_gpio_ops,
	.of_to_plat = allwinner_h6_gpio_of_to_plat,
	.plat_auto	= sizeof(allwinner_h6_gpio_plat_t),
	.probe		= allwinner_h6_gpio_probe,
};



