#ifndef _ARCH_ASM_MACH_GPIO_H
#define _ARCH_ASM_MACH_GPIO_H

#include  <stdint.h>

#define  GPIO_BASE_ADDR       (0x0300b048u)
#define  GPIO_BASE_R_ADDR     (0x07022000u)

enum {
    GPIOC  =  0,
    GPIOD,
    GPIOF,
    GPIOG,
    GPIOH,
    GPIOL,
    GPIOM,
    GPIO_MAX_BANK = GPIOM,
};

#define  GPIO_BANK_ADDR(n)    ( (n) >= GPIOL ? GPIO_BASE_R_ADDR + ((n) - GPIOL) * 4: \
                            GPIO_BASE_ADDR + (n) * 4 )


enum {
    GPIO_PULL_DISABLE =  0,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN,
    GPIO_PULL_MAX = GPIO_PULL_DOWN,
};


enum {
    GPIO_DRIVE_LEVEL0 =  0,
    GPIO_DRIVE_LEVEL1,
    GPIO_DRIVE_LEVEL2,
    GPIO_DRIVE_LEVEL3,
    GPIO_DRIVE_MAX_LEVEL = GPIO_DRIVE_LEVEL3,
};


int32_t  set_gpio_pin_func(const uint32_t bank, const uint32_t pin, const uint32_t func);
int32_t  set_gpio_pin_pull(const uint32_t bank, const uint32_t pin, const uint32_t pull_type);
int32_t  set_gpio_pin_drive(const uint32_t bank, const uint32_t pin, const uint32_t drive_level);

typedef struct {
    uint32_t  bank:4;
    uint32_t  pin:6;
    uint32_t  pull_type:2;
    uint32_t  drive_level:2;
} gpio_pin_cfg_t;


#endif


