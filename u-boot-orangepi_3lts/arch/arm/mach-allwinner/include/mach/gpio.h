#ifndef _ARCH_ASM_MACH_GPIO_H
#define _ARCH_ASM_MACH_GPIO_H

#include  <stdint.h>

#define  GPIO_BASE_ADDR       (0x0300b000u)
#define  GPIO_BASE_R_ADDR     (0x07022000u)

enum {
    GPIOA  =  0,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE,
    GPIOF,
    GPIOG,
    GPIOH,
    GPIOL,
    GPIOM,
    GPIO_MAX_BANK = GPIOM,
};


#define  GPIO_CFG_REG_STEP              (0x24u)


#define  GPIO_BANK_ADDR(n)    ( (n) >= GPIOL ? GPIO_BASE_R_ADDR + ((n) - GPIOL) * GPIO_CFG_REG_STEP: \
                            GPIO_BASE_ADDR + (n) * GPIO_CFG_REG_STEP )


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

typedef enum {
	ALLWINNER_H6_GPIO_AF0 = 0,
	ALLWINNER_H6_GPIO_AF1,
	ALLWINNER_H6_GPIO_AF2,
	ALLWINNER_H6_GPIO_AF3,
	ALLWINNER_H6_GPIO_AF4,
	ALLWINNER_H6_GPIO_AF5,
	ALLWINNER_H6_GPIO_AF6,
	ALLWINNER_H6_GPIO_AF7,
	ALLWINNER_H6_GPIO_MAX_AF  =  ALLWINNER_H6_GPIO_AF7
} allwinner_h6_gpio_af_e;


#define  ALLWINNER_H6_GPIO_INPUT       ALLWINNER_H6_GPIO_AF0
#define  ALLWINNER_H6_GPIO_OUTPUT      ALLWINNER_H6_GPIO_AF1
#define  ALLWINNER_H6_GPIO_DISABLE     ALLWINNER_H6_GPIO_AF7


int32_t  set_gpio_pin_func(const uint32_t bank, const uint32_t pin, const uint32_t func);
int32_t  set_gpio_pin_pull(const uint32_t bank, const uint32_t pin, const uint32_t pull_type);
int32_t  set_gpio_pin_drive(const uint32_t bank, const uint32_t pin, const uint32_t drive_level);
int32_t  get_gpio_pin_value(const uint32_t bank, const uint32_t pin, uint32_t * const value);
int32_t  allwinner_gpio_output_value(const uint32_t bank, const uint32_t pin, 
                                    const uint32_t pull_type,  const uint32_t value );

typedef struct {
    uint32_t  bank:4;
    uint32_t  pin:6;
    uint32_t  pull_type:2;
    uint32_t  drive_level:2;
} gpio_pin_cfg_t;


#endif


