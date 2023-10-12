#include <common.h>
#include <dm.h>
#include <env.h>
#include <hang.h>
#include <init.h>
#include <mmc.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/cpu.h>


/*init board mmc gpio*/
int32_t board_mmc_init(struct bd_info *bis)
{
    _DBG_PRINTF("board_mmc_init\n");

    for (int32_t i  =  0; i < 6; i++) {
        set_gpio_pin_func(GPIOF,   i,  ALLWINNER_H6_GPIO_AF2);
        set_gpio_pin_pull(GPIOF,   i,  GPIO_PULL_UP);
        set_gpio_pin_drive(GPIOF,  i,  GPIO_DRIVE_LEVEL2);
    }

	return  0;
}




