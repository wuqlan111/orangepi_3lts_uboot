

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <asm/io.h>
#include <dm/pinctrl.h>

/*
 * This driver works with very simple configuration that has the same name
 * for group and function. This way it is compatible with the Linux Kernel
 * driver.
 */

struct ast2500_pinctrl_priv {
	struct ast2500_scu *scu;
};

static int ast2500_pinctrl_probe(struct udevice *dev)
{
	struct ast2500_pinctrl_priv *priv = dev_get_priv(dev);

	priv->scu = ast_get_scu();

	return 0;
}

struct ast2500_group_config {
	char *group_name;
	/* Control register number (1-10) */
	unsigned reg_num;
	/* The mask of control bits in the register */
	u32 ctrl_bit_mask;
};













