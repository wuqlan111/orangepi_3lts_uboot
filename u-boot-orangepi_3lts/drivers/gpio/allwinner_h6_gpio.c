#define  LOG_CATEGORY UCLASS_GPIO

#include <config.h>
#include <common.h>
#include <stdint.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <linux/io.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>

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
	uint32_t   gpio_count;
    uint32_t   pins;
} allwinner_h6_gpio_plat_t;


static allwinner_h6_gpio_config_t * allwinner_pio_get_port_cfg(const uint32_t port)
{
	if (port > GPIO_MAX_BANK) {
		return  NULL;
	}
	
	return  (allwinner_h6_gpio_config_t * )GPIO_BANK_ADDR(port);	
}


static int32_t _allwinner_h6_gpio_direction_input(allwinner_h6_gpio_config_t * const config_regs,  
														const uint32_t pin)
{

	uint32_t  offset  =  pin  >> 3;
	uint32_t  shift   =  (pin % 8) << 2;

	clrsetbits_32(&config_regs->cfgx[offset],  0x7 << shift, 
									ALLWINNER_H6_GPIO_INPUT << shift);

	return  0;
}


static int32_t _allwinner_h6_gpio_direction_output(allwinner_h6_gpio_config_t * const config_regs, const uint32_t pin,
				const int32_t val)
{
	uint32_t  offset  =  pin  >> 3;
	uint32_t  shift   =  (pin % 8) << 2;

	clrsetbits_32(&config_regs->cfgx[offset],  0x7 << shift, 
									ALLWINNER_H6_GPIO_OUTPUT << shift);
	if (val) {
		setbits_32(&config_regs->dat,  1 << pin);
	} else {
		clrbits_32(&config_regs->dat,  1 << pin);
	}
	
	return 0;
}


static int32_t _allwinner_h6_gpio_get_value(allwinner_h6_gpio_config_t * const config_regs, const uint32_t pin)
{
	return  readl(&config_regs->dat) & (1 << pin)? 1: 0;
}

static int32_t _allwinner_h6_gpio_set_value(allwinner_h6_gpio_config_t * const config_regs, 
								const uint32_t pin, const int32_t val)
{
	if (val) {
		setbits_le32(&config_regs->dat,  1 << pin);		
	} else {
		clrbits_le32(&config_regs->dat,  1 << pin);
	}

	return 0;
}

static int32_t _allwinner_h6_gpio_get_function(allwinner_h6_gpio_config_t * const config_regs, const uint32_t  pin)
{
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



#if CONFIG_IS_ENABLED(DM_GPIO)


static int32_t allwinner_h6_gpio_direction_input(struct udevice *dev, uint32_t pin)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (pin >= plat->gpio_count) {
		dev_err(dev, "pin_id [%u] invalid", pin);
		return -EINVAL;
	}

	return  _allwinner_h6_gpio_direction_input(config_regs,  pin);
}


static int32_t allwinner_h6_gpio_direction_output(struct udevice *dev, uint32_t pin,
				    int32_t val)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;
	
	if (pin >= plat->gpio_count) {
		dev_err(dev, "pin_id [%u] invalid", pin);
		return -EINVAL;
	}

	return  _allwinner_h6_gpio_direction_output(config_regs,  pin,  val);
}


static int32_t allwinner_h6_gpio_get_value(struct udevice *dev, uint32_t pin)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (pin >= plat->gpio_count) {
		dev_err(dev, "pin_id [%u] invalid", pin);
		return -EINVAL;
	}

	return  _allwinner_h6_gpio_get_value(config_regs,  pin);
}


static int32_t allwinner_h6_gpio_set_value(struct udevice *dev, uint32_t pin, int32_t val)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (pin >= plat->gpio_count) {
		dev_err(dev, "pin_id [%u] invalid", pin);
		return -EINVAL;
	}

	return  _allwinner_h6_gpio_set_value(config_regs,  pin, val);
}


static int32_t allwinner_h6_gpio_get_function(struct udevice *dev, uint32_t  pin)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (pin >= plat->gpio_count) {
		dev_err(dev, "pin_id [%u] invalid", pin);
		return -EINVAL;
	}

	return  _allwinner_h6_gpio_get_function(config_regs,  pin);
}


static const struct dm_gpio_ops allwinner_h6_gpio_ops = {
	.direction_input	= allwinner_h6_gpio_direction_input,
	.direction_output	= allwinner_h6_gpio_direction_output,
	.get_value		= allwinner_h6_gpio_get_value,
	.set_value		= allwinner_h6_gpio_set_value,
	.get_function   = allwinner_h6_gpio_get_function,
};


static int32_t allwinner_h6_gpio_probe(struct udevice *dev)
{

	return 0;
}

static int32_t allwinner_h6_gpio_of_to_plat(struct udevice *dev)
{
	int32_t  ret  =  0;
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);

	assert(plat != NULL);

	plat->config_base  =  dev_read_addr_index(dev,  0);
	plat->interrupt_base  =  dev_read_addr_index(dev,  1);
	ret = dev_read_s32(dev, "allwinner,gpio-bank-width",  &plat->gpio_count);
	// plat->bank_name = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
	// 	"gpio-bank-name", NULL);

	return  ret;
}



static const struct udevice_id allwinner_h6_gpio_ids[] = {
	{ .compatible = "allwinner,H6-v200-gpio"},
};

U_BOOT_DRIVER(allwinner_h6_gpio) = {
	.name		= "allwinner_h6_gpio",
	.id		= UCLASS_GPIO,
	.of_match	= allwinner_h6_gpio_ids,
	.ops		= &allwinner_h6_gpio_ops,
	.of_to_plat = allwinner_h6_gpio_of_to_plat,
	.plat_auto	= sizeof(allwinner_h6_gpio_plat_t),
	.probe		= allwinner_h6_gpio_probe,
	.flags   =   DM_FLAG_PRE_RELOC,
};

#else

#define  GPIO_TO_PORT(gpio)   ((gpio) >> 5)
#define  GPIO_TO_PIN(gpio)    ((gpio) & 0x1f)

int32_t gpio_request(uint32_t gpio, const char *label)
{
	return 0;
}

int32_t gpio_free(uint32_t gpio)
{
	return 0;
}

int32_t gpio_direction_input(uint32_t gpio)
{
	uint32_t  port = GPIO_TO_PORT(gpio);
	uint32_t  pin  =  GPIO_TO_PIN(gpio);

	allwinner_h6_gpio_config_t *  config_regs  =  allwinner_pio_get_port_cfg(port);
	if (config_regs == NULL) {
		return  -1;
	}

	return  _allwinner_h6_gpio_direction_input(config_regs,  pin);
}

int32_t gpio_direction_output(uint32_t gpio, int32_t value)
{
	uint32_t  port = GPIO_TO_PORT(gpio);
	uint32_t  pin  =  GPIO_TO_PIN(gpio);

	allwinner_h6_gpio_config_t *  config_regs  =  allwinner_pio_get_port_cfg(port);
	if (config_regs == NULL) {
		return  -1;
	}

	return  _allwinner_h6_gpio_direction_output(config_regs,  pin,  0);

}

int32_t gpio_get_value(uint32_t gpio)
{
	uint32_t  port = GPIO_TO_PORT(gpio);
	uint32_t  pin  =  GPIO_TO_PIN(gpio);

	allwinner_h6_gpio_config_t *  config_regs  =  allwinner_pio_get_port_cfg(port);
	if (config_regs == NULL) {
		return  -1;
	}

	return  _allwinner_h6_gpio_get_value(config_regs,  pin);

}

int32_t gpio_set_value(uint32_t gpio, int32_t value)
{
	uint32_t  port = GPIO_TO_PORT(gpio);
	uint32_t  pin  =  GPIO_TO_PIN(gpio);

	allwinner_h6_gpio_config_t *  config_regs  =  allwinner_pio_get_port_cfg(port);
	if (config_regs == NULL) {
		return  -1;
	}

	return  _allwinner_h6_gpio_set_value(config_regs,  pin,  value);

}

#endif



