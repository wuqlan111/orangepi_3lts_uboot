

#include <common.h>
#include <stdint.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <log.h>
#include <asm/io.h>
#include <dm/pinctrl.h>
#include <asm/arch/gpio.h>

#define   PIN_CONFIG_STRING    "allwinner,pins" 
#define   PIN_CONFIG_FUNC_MAX           7

typedef struct  {
	char * base_name;
	fdt_addr_t  base;
	uint32_t  pin_count;
}allwinner_h6_pinctrl_priv_t;

typedef enum {
	ALLWINNER_PIN_CONFIG_OFFSET = 0,
	ALLWINNER_PIN_CONFIG_FUNC,
	ALLWINNER_PIN_CONFIG_MAX  =  ALLWINNER_PIN_CONFIG_FUNC,
} pin_config_type_e;


static  int32_t  _allwinner_h6_set_pinmux_func(uint32_t * const base, const uint32_t pin, 
								const uint32_t func)
{
	if (func > PIN_CONFIG_FUNC_MAX) {
		log_err("func invalid!");
		return  -1;
	}

	const  uint32_t  reg_offset  =  pin >> 3;
	const  uint32_t  reg_shift   =  (pin & 0x7) << 2;
	const  uint32_t  mask   =  0x7 << reg_shift;
	const  uint32_t  flag   =  func << reg_shift;

	_DBG_PRINTF("base_addr -- 0x%p,\tpin -- %u,\tfunc -- %u\n", base,  
							pin,  func);
	clrsetbits_32(&base[reg_offset],  mask,  flag);

	return  0;

}


static  int32_t  _allwinner_h6_get_pinmux_func(uint32_t * const base, const uint32_t pin, 
								uint32_t * const func)
{
	if (func == NULL) {
		log_err("func null!");
		return  -1;
	}

	const  uint32_t  reg_offset  =  pin >> 3;
	const  uint32_t  reg_shift   =  (pin & 0x7) << 2;
	const uint32_t  flag  =  readl(&base[reg_offset]);
	*func  =  (flag >> reg_shift ) & 0x7;

	return  0;

}



static int32_t allwinner_h6_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	int32_t  ret  =  0;
	allwinner_h6_pinctrl_priv_t * priv  =  dev_get_priv(dev);
	uint32_t * base_addr = (uint32_t *)priv->base;

	int32_t  size  = dev_read_size(config, PIN_CONFIG_STRING);
	if (size <  0) {
		dev_err(dev, "config dev [%s] no pin config!", config->name);
		return  size;
	}

	uint32_t  array_size  =   size >> 2;
	uint32_t  pin_count   =   array_size / ( ALLWINNER_PIN_CONFIG_MAX + 1);
	uint32_t  prop_data[array_size];
	memset(prop_data, 0,  size);

	ret  =  dev_read_u32_array(config,  PIN_CONFIG_STRING, prop_data, array_size);
	if (ret < 0) {
		dev_err(dev, "read config dev pin config failed!");
		return  ret;
	}

	for (uint32_t i  = 0; i < pin_count; i++) {
		uint32_t  tmp_index   =  i * (ALLWINNER_PIN_CONFIG_MAX + 1);
		uint32_t  pin_offset  =  prop_data[ tmp_index + ALLWINNER_PIN_CONFIG_OFFSET ];
		uint32_t  pin_func    =  prop_data[ tmp_index + ALLWINNER_PIN_CONFIG_FUNC ];

		if ( (pin_offset >= priv->pin_count) || (pin_func > PIN_CONFIG_FUNC_MAX) ) {
			dev_err(dev, "config dev pin offset [%u] invalid!", pin_offset);
			return  -1;
		}

		_allwinner_h6_set_pinmux_func(base_addr, pin_offset,  pin_func);

	}

	return   0;

}


static int32_t allwinner_h6_pinctrl_get_count(struct udevice *dev)
{
	allwinner_h6_pinctrl_priv_t * priv  =  dev_get_priv(dev);
	
	if (!priv) {
		dev_err(dev, "dev priv null\n");
		return  -1;
	}

	return  priv->pin_count;

}


static char pin_name_buffer[32];
static char * allwinner_h6_pinctrl_get_pin_name(struct udevice *dev,  uint32_t selector)
{
	allwinner_h6_pinctrl_priv_t * priv  =  dev_get_priv(dev);
	
	if (!priv) {
		dev_err(dev, "dev priv null\n");
		return  NULL;
	}

	if (selector >= priv->pin_count) {
		dev_err(dev, "pin selector invalid\n");
		return  NULL;
	}

	memset(pin_name_buffer, 0,  sizeof(pin_name_buffer));
	sprintf(pin_name_buffer, "%s%d", priv->base_name,  selector);

	return  pin_name_buffer;

}


static int32_t  allwinner_h6_pinctrl_get_pin_mux(struct udevice *dev, uint32_t pin,
			       char * buf, int32_t size)
{
	int32_t ret =  0;
	allwinner_h6_pinctrl_priv_t * priv  =  dev_get_priv(dev);
	uint32_t * const base  =  (uint32_t *)priv->base;
	
	if (!priv) {
		dev_err(dev, "dev priv null\n");
		return  -1;
	}

	if (pin >= priv->pin_count) {
		dev_err(dev, "pin selector invalid\n");
		return  -1;
	}

	uint32_t  func =  0;
	if (_allwinner_h6_get_pinmux_func(base,  pin,  &func)) {
		return  -1;
	}

	char * pinmux_str = pinmux_func_2_str(func);
	const int32_t len = strlen(pinmux_str);

	if (len >= size) {
		ret =  -1;
	} else {
		strcpy(buf, pinmux_str);
	}

	return  ret;

}


const struct pinctrl_ops allwinner_h6_pinctrl_ops  = {
	.set_state = allwinner_h6_pinctrl_set_state,
	.get_pin_muxing  =  allwinner_h6_pinctrl_get_pin_mux,
	.get_pins_count  = allwinner_h6_pinctrl_get_count,
	.get_pin_name  = allwinner_h6_pinctrl_get_pin_name,
};


static int32_t allwinner_h6_pinctrl_probe(struct udevice *dev)
{
	int32_t  ret  =  0;
	allwinner_h6_pinctrl_priv_t * priv  =  dev_get_priv(dev);

	priv->base  = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE) {
		dev_err(dev, "read dev addr failed!");
		return  -1;
	}

	const char * name = dev_read_string(dev,  "base-name");
	if (name == NULL) {
		dev_err(dev, "base-name prop need!");
		return -1;
	}

	priv->base_name  =  strdup(name);

	ret  =  dev_read_u32(dev, "ngpios",  &priv->pin_count);

	return  ret;
}


static const struct udevice_id allwinner_h6_pinctrl_match[] = {
	{ .compatible = "allwinner,H6-v200-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(allwinner_h6_pinctrl) = {
	.name = "allwinner-h6-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = allwinner_h6_pinctrl_match,
	.probe = allwinner_h6_pinctrl_probe,
	.priv_auto	= sizeof(allwinner_h6_pinctrl_priv_t),
	.ops = &allwinner_h6_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};






