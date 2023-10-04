
#include <common.h>
#include <cpu_func.h>
#include <mmc.h>
#include <serial.h>
#include <spl.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/spl.h>
#include <log.h>

#include <linux/compiler.h>

#ifdef CONFIG_SPL_BUILD

static int32_t allwinner_get_boot_source(uint32_t * const boot_source)
{
	boot0_file_header_t * const header =  (boot0_file_header_t *)SPL_ADDR;

    if (memcmp(headr->magic, BOOT0_MAGIC, 8)) {
        panic("invalid boot source\n");
        return  -1;
    }

    uint32_t boot_media = cpu_to_be32(header->boot_media) & 0xff;
	return  0;
}

/* The sunxi internal brom will try to loader external bootloader
 * from mmc0, nand flash, mmc2.
 */
uint32_t allwinner_get_boot_device(void)
{
    uint32_t  boot_source  =  0;
    uint32_t  boot_device  =  0;
	sunxi_get_boot_source(&boot_source);

	/*
	 * When booting from the SD card or NAND memory, the "eGON.BT0"
	 * signature is expected to be found in memory at the address 0x0004
	 * (see the "mksunxiboot" tool, which generates this header).
	 *
	 * When booting in the FEL mode over USB, this signature is patched in
	 * memory and replaced with something else by the 'fel' tool. This other
	 * signature is selected in such a way, that it can't be present in a
	 * valid bootable SD card image (because the BROM would refuse to
	 * execute the SPL in this case).
	 *
	 * This checks for the signature and if it is not found returns to
	 * the FEL code in the BROM to wait and receive the main u-boot
	 * binary over USB. If it is found, it determines where SPL was
	 * read from.
	 */
	switch (boot_source) {
        case ALLWINNER_INVALID_BOOT_SOURCE:
            return BOOT_DEVICE_BOARD;
        case ALLWINNER_BOOTED_FROM_MMC0:
        case ALLWINNER_BOOTED_FROM_MMC0_HIGH:
            return BOOT_DEVICE_MMC1;
        case ALLWINNER_BOOTED_FROM_NAND:
            return BOOT_DEVICE_NAND;
        case ALLWINNER_BOOTED_FROM_MMC2:
        case ALLWINNER_BOOTED_FROM_MMC2_HIGH:
            return BOOT_DEVICE_MMC2;
        case ALLWINNER_BOOTED_FROM_SPI:
            return BOOT_DEVICE_SPI;
	}

    panic("Unknown boot source %d\n", boot_source);
	return -1;

}

uint64_t spl_mmc_get_uboot_raw_sector(struct mmc *mmc)
{
	uint64_t sector = CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR;
    uint32_t  boot_source =  0;

    allwinner_get_boot_source(&boot_source);

    if ( (boot_source == SUNXI_BOOTED_FROM_MMC0_HIGH) || 
                (boot_source == SUNXI_BOOTED_FROM_MMC0_HIGH )) {
        sector += (128 - 8) * 2;
    }

	return  sector;
}

uint32_t spl_boot_device(void)
{
	return allwinner_get_boot_device();
}


void __weak allwinner_spl_board_init(void)
{

}

void board_init_f(ulong dummy)
{
    int32_t  ret  =  0;

	ret = spl_init();
    if (ret) {
        panic("spl_init failed!\n");
        return;
    }

	preloader_console_init();

	_DBG_PRINTF("preloader_console_init finished, start allwinner_spl_board_init!\n");
	allwinner_spl_board_init();
	_DBG_PRINTF("allwinner_spl_board_init finished!\n");


}


#endif

