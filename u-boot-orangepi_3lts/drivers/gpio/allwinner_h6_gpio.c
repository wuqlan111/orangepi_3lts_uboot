#define  LOG_CATEGORY UCLASS_GPIO

#include <config.h>
#include <common.h>
#include <stdint.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device.h>
#include <dm/pinctrl.h>
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
	uint32_t   pinctrl_bank;
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
	uint32_t  shift   =  (pin & 0x7) << 2;

	clrsetbits_32(&config_regs->cfgx[offset],  0x7 << shift, 
									ALLWINNER_H6_PINMUX_INPUT << shift);

	return  0;
}


static int32_t _allwinner_h6_gpio_direction_output(allwinner_h6_gpio_config_t * const config_regs, const uint32_t pin,
				const int32_t val)
{
	uint32_t  offset  =  pin  >> 3;
	uint32_t  shift   =  (pin & 0x7) << 2;

	clrsetbits_32(&config_regs->cfgx[offset],  0x7 << shift, 
									ALLWINNER_H6_PINMUX_OUTPUT << shift);
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
	uint32_t  shift   =  (pin & 0x7) << 2;
	uint32_t  pin_func  =  (readl(&config_regs->cfgx[offset]) >> shift) & 0x7;

	if (pin_func ==  ALLWINNER_H6_PINMUX_OUTPUT) {
		return  GPIOF_OUTPUT;
	} else if (pin_func ==  ALLWINNER_H6_PINMUX_INPUT ) {
		return  GPIOF_INPUT;
	} else if (pin_func == ALLWINNER_H6_PINMUX_DISABLE) {
		return  GPIOF_UNUSED;
	}

	return GPIOF_FUNC;
}



#if CONFIG_IS_ENABLED(DM_GPIO)


static int32_t allwinner_h6_gpio_direction_input(struct udevice *dev, uint32_t offset)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (offset >= plat->gpio_count) {
		dev_err(dev, "pin_id [%u] invalid", pin);
		return -EINVAL;
	}

	char * cur_pin = NULL;
	uint32_t  cur_bank, cur_pin_offset;
	ret = dev_read_string_index(dev, "gpio-pins", offset, &cur_pin);
	if (ret || check_pinctrl_name_vld(cur_pin, &cur_bank, &cur_pin_offset) ) {
		return  -EINVAL | ret;
	}
	
	return  _allwinner_h6_gpio_direction_input(config_regs,  cur_pin_offset);
}


static int32_t allwinner_h6_gpio_direction_output(struct udevice *dev, uint32_t offset,
				    int32_t val)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;
	
	if (pin >= plat->gpio_count) {
		dev_err(dev, "pin_id [%u] invalid", pin);
		return -EINVAL;
	}

	char * cur_pin = NULL;
	uint32_t  cur_bank, cur_pin_offset;
	ret = dev_read_string_index(dev, "gpio-pins", offset, &cur_pin);
	if (ret || check_pinctrl_name_vld(cur_pin, &cur_bank, &cur_pin_offset) ) {
		return  -EINVAL | ret;
	}

	return  _allwinner_h6_gpio_direction_output(config_regs,  cur_pin_offset,  val);
}


static int32_t allwinner_h6_gpio_get_value(struct udevice *dev, uint32_t offset)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (pin >= plat->gpio_count) {
		dev_err(dev, "pin_id [%u] invalid", pin);
		return -EINVAL;
	}

	char * cur_pin = NULL;
	uint32_t  cur_bank, cur_pin_offset;
	ret = dev_read_string_index(dev, "gpio-pins", offset, &cur_pin);
	if (ret || check_pinctrl_name_vld(cur_pin, &cur_bank, &cur_pin_offset) ) {
		return  -EINVAL | ret;
	}

	return  _allwinner_h6_gpio_get_value(config_regs,  cur_pin_offset);
}


static int32_t allwinner_h6_gpio_set_value(struct udevice *dev, uint32_t offset, int32_t val)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (pin >= plat->gpio_count) {
		dev_err(dev, "pin_id [%u] invalid", pin);
		return -EINVAL;
	}

	char * cur_pin = NULL;
	uint32_t  cur_bank, cur_pin_offset;
	ret = dev_read_string_index(dev, "gpio-pins", offset, &cur_pin);
	if (ret || check_pinctrl_name_vld(cur_pin, &cur_bank, &cur_pin_offset) ) {
		return  -EINVAL | ret;
	}

	return  _allwinner_h6_gpio_set_value(config_regs,  cur_pin_offset, val);
}


static int32_t allwinner_h6_gpio_get_function(struct udevice *dev, uint32_t  offset)
{
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	allwinner_h6_gpio_config_t *  config_regs  =  
									(allwinner_h6_gpio_config_t * )plat->config_base;

	if (pin >= plat->gpio_count) {
		dev_err(dev, "pin_id [%u] invalid", pin);
		return -EINVAL;
	}

	char * cur_pin = NULL;
	uint32_t  cur_bank, cur_pin_offset;
	ret = dev_read_string_index(dev, "gpio-pins", offset, &cur_pin);
	if (ret || check_pinctrl_name_vld(cur_pin, &cur_bank, &cur_pin_offset) ) {
		return  -EINVAL | ret;
	}

	return  _allwinner_h6_gpio_get_function(config_regs,  cur_pin_offset);
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
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);
	struct gpio_dev_priv * uc_priv  =  dev_get_uclass_priv(dev);
	if (!uc_priv) {
		dev_err(dev, "get gpio uc priv failed!\n");
		_DBG_PRINTF("uc_priv is null!\n");
		return  -1;
	}

	char * bank_name  =  dev_read_string(dev, "gpio-bank-name");
	if (!uc_priv->name) {
		uc_priv->bank_name  =  strdup(bank_name);		
	}

	uc_priv->gpio_count  =  plat->gpio_count;
	_DBG_PRINTF("gpio:\tbank -- %s,\tpins -- %u\n", uc_priv->bank_name, uc_priv->gpio_count);

	return 0;
}

static int32_t allwinner_h6_gpio_of_to_plat(struct udevice *dev)
{
	int32_t  ret  =  0;
	allwinner_h6_gpio_plat_t * plat = dev_get_plat(dev);

	assert(plat != NULL);

	ret = dev_read_string_count(dev , "gpio-pins");
	if (ret < 0) {
		return  ret;
	}
	plat->ngpios = ret;

	char * last_pin = NULL, *cur_pin = NULL;
	for (int32_t i = 0; i < ret; i++) {
		uint32_t  cur_bank, cur_pin_offset;
		ret = dev_read_string_index(dev, "gpio-pins", i, &cur_pin);
		if (ret || check_pinctrl_name_vld(cur_pin, &cur_bank, &cur_pin_offset) ) {
			return  -EINVAL | ret;
		}

		if ((last_pin != NULL) && (last_pin[1] != cur_pin[1])) {
			_DBG_PRINTF("not in samme bank!\n");
			return -EINVAL;
		}

		last_pin = cur_pin;
		plat->pinctrl_bank = cur_bank;
	}

	plat->config_base  =  GPIO_BANK_ADDR(plat->pinctrl_bank);
	// plat->interrupt_base  =  ;
	ret = dev_read_u32(dev, "ngpios",  &plat->gpio_count);
	if (ret) {
		dev_err(dev, "get npios prop failed!\n");
	}

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



