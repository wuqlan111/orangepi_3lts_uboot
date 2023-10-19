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
#include <asm/arch/smhc.h>


/*init board mmc gpio*/
int32_t board_mmc_init(struct bd_info *bis)
{
    int32_t  ret  =  0;
    _DBG_PRINTF("board_mmc_init\n");

    for (int32_t i  =  0; i < 6; i++) {
        set_gpio_pin_func(GPIOF,   i,  ALLWINNER_H6_GPIO_AF2);
        set_gpio_pin_pull(GPIOF,   i,  GPIO_PULL_UP);
        set_gpio_pin_drive(GPIOF,  i,  GPIO_DRIVE_LEVEL2);
    }

    if (!allwinner_mmc_init(CCU_SMHC0_ID)) {
        ret  =  -1;
    }

    _DBG_PRINTF("mmc init %s\n", ret ? "failed": "success");

	return  ret;
}


int32_t dram_init(void)
{
    gd->ram_size = 0x80000000;

    _DBG_PRINTF("%u MB\n",  gd->ram_size >> 20 );
	return  0;
}

