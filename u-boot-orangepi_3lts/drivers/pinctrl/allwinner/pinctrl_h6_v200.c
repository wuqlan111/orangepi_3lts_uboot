

#include <common.h>
#include <stdint.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <log.h>
#include <asm/io.h>
#include <dm/pinctrl.h>

#define   PIN_CONFIG_STRING    "allwinner,pins" 
#define   PIN_CONFIG_FUNC_MAX           7

typedef struct  {
	struct udevice *dev;
	fdt_addr_t  base;
	uint32_t  pin_count;
}allwinner_h6_pinctrl_priv_t;

typedef enum {
	ALLWINNER_PIN_CONFIG_OFFSET = 0,
	ALLWINNER_PIN_CONFIG_FUNC,
	ALLWINNER_PIN_CONFIG_MAX  =  ALLWINNER_PIN_CONFIG_FUNC,
} pin_config_type_e;


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

		uint32_t  reg_offset  =  pin_offset >> 3;
		uint32_t  reg_shift   =  (pin_offset % 8) << 2;

		uint32_t  mask   =  0x7 << reg_shift;
		uint32_t  flag   =  pin_func << reg_shift;

		clrsetbits_32(base_addr + reg_offset, mask,  flag);

	}

	return   0;

}


const struct pinctrl_ops allwinner_h6_pinctrl_ops  = {
	.set_state = allwinner_h6_pinctrl_set_state,
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
	.of_match = of_match_ptr(allwinner_h6_pinctrl_match),
	.probe = allwinner_h6_pinctrl_probe,
	.priv_auto	= sizeof(allwinner_h6_pinctrl_priv_t),
	.ops = &allwinner_h6_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};






