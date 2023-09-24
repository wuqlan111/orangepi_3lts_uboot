
#include <common.h>
#include <dm.h>
#include <hwspinlock.h>
#include <log.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/libfdt.h>

#include "../gpio/allwinner_h6_gpio.h"

#define MAX_PINS_ONE_IP			70
#define MODE_BITS_MASK			3
#define OSPEED_MASK			3
#define PUPD_MASK			3
#define OTYPE_MSK			1
#define AFR_MASK			0xF

struct stm32_pinctrl_priv {
	struct hwspinlock hws;
	int pinctrl_ngpios;
	struct list_head gpio_dev;
};





















