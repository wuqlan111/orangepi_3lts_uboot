#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>


typedef struct {
    uint32_t  cfg[4];
    uint32_t  data;
    uint32_t  drive[2];
    uint32_t  pull[2];
} pinmux_cfg_t;


int32_t  set_gpio_pin_func(const uint32_t bank, const uint32_t pin, const uint32_t func)
{
    pinmux_cfg_t * cfg_regs = (pinmux_cfg_t *)GPIO_BANK_ADDR(bank);
    if ( (bank > GPIO_MAX_BANK) || (pin > 31)  || (func > 7)) {
        return  -1;
    }

    uint32_t index  =  pin  >> 3;
    uint32_t shift  =  (pin & 0x7) << 2;

    uint32_t flag  =  func << shift;
    uint32_t mask  =  0x7 << shift;

    clrsetbits_32(&cfg_regs->cfg[index],  mask,  flag);

    return  0;

}


int32_t  set_gpio_pin_pull(const uint32_t bank, const uint32_t pin, const uint32_t pull_type)
{
    pinmux_cfg_t * cfg_regs = (pinmux_cfg_t *)GPIO_BANK_ADDR(bank);
    if ( (bank > GPIO_MAX_BANK) ||  (pin > 31) || (pull_type > GPIO_PULL_MAX)) {
        return  -1;
    }

    uint32_t index  =  pin  >> 4;
    uint32_t shift  =  (pin & 0xf) << 1;

    uint32_t flag  =  pull_type << shift;
    uint32_t mask  =  0x3 << shift;

    clrsetbits_32(&cfg_regs->pull[index],  mask,  flag);

    return  0;

}


int32_t  set_gpio_pin_drive(const uint32_t bank, const uint32_t pin, const uint32_t drive_level)
{
    pinmux_cfg_t * cfg_regs = (pinmux_cfg_t *)GPIO_BANK_ADDR(bank);
    if ( (bank > GPIO_MAX_BANK) ||  (pin > 31) || 
                    (drive_level > GPIO_DRIVE_MAX_LEVEL)) {
        return  -1;
    }

    uint32_t index  =  pin  >> 4;
    uint32_t shift  =  (pin & 0xf) << 1;

    uint32_t  flag  =  drive_level << shift;
    uint32_t  mask  =  0x3 << shift;

    clrsetbits_32(&cfg_regs->drive[index],  mask,  flag);

    return  0;

}


int32_t  set_gpio_pin_value(const uint32_t bank, const uint32_t pin, const uint32_t value)
{
    pinmux_cfg_t * cfg_regs = (pinmux_cfg_t *)GPIO_BANK_ADDR(bank);
    if ( (bank > GPIO_MAX_BANK) ||  (pin > 31) ) {
        return  -1;
    }

    uint32_t  flag  =  1 << pin;

    if (value) {
        setbits_32(&cfg_regs->data,  flag);
    } else {
        clrbits_32(&cfg_regs->data,  flag);
    }

    return  0;

}


int32_t  get_gpio_pin_value(const uint32_t bank, const uint32_t pin, uint32_t * const value)
{
    pinmux_cfg_t * cfg_regs = (pinmux_cfg_t *)GPIO_BANK_ADDR(bank);
    if ( (bank > GPIO_MAX_BANK) ||  (pin > 31) ) {
        return  -1;
    }

    uint32_t  mask  =  1 << pin;
    *value  =  readl(&cfg_regs->data) & mask ? 1:  0;

    return  0;

}



int32_t allwinner_gpio_output_value(const uint32_t bank, const uint32_t pin, 
                                    const uint32_t pull_type,  const uint32_t value )

{
    if (set_gpio_pin_func(bank, pin,  ALLWINNER_H6_PINMUX_OUTPUT)) {
        return  -1;
    }

    set_gpio_pin_pull(bank,  pin,  pull_type);
    set_gpio_pin_value(bank,  pin,  value);

    return  0;

}

#define  TYPE_CASE(x)    case (x):  return #x;

char  *  pinmux_func_2_str(const uint32_t func)
{
    switch (func) {
        TYPE_CASE(ALLWINNER_H6_PINMUX_INPUT)
        TYPE_CASE(ALLWINNER_H6_PINMUX_OUTPUT)
        TYPE_CASE(ALLWINNER_H6_PINMUX_AF2)
        TYPE_CASE(ALLWINNER_H6_PINMUX_AF3)
        TYPE_CASE(ALLWINNER_H6_PINMUX_AF4)
        TYPE_CASE(ALLWINNER_H6_PINMUX_AF5)
        TYPE_CASE(ALLWINNER_H6_PINMUX_AF6)
        TYPE_CASE(ALLWINNER_H6_PINMUX_DISABLE)
    }

    return "invalid_pinmux_func";

}


int32_t  check_pinctrl_name_vld(const char * name, uint32_t * const bank, uint32_t * const pin)
{
	if (name[0] != 'P' || (name[1] < 'A')  || (name[1] > 'M')) {
		return -EINVAL;
	}

	uint32_t tmp_bank = GPIOA + name[1] - 'A';
    ulong  tmp_pin = simple_strtoul(&name[2], NULL, 10);

    if (tmp_pin > 31) {
        return -EINVAL;
    }

    if (bank) {
        *bank = tmp_bank;
    }

    if (pin) {
        *pin = tmp_pin;
    }

    return  0;

}






