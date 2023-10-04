
#ifndef	_ARCH_ASM_MACH_SPL_H
#define	_ARCH_ASM_MACH_SPL_H

#include  <stdint.h>
#include  <linux/bitops.h>

#define  BOOT0_MAGIC		"eGON.BT0"
#define  SPL_SIGNATURE		"SPL" /* marks "sunxi" SPL header */
#define  SPL_MAJOR_BITS		3
#define  SPL_MINOR_BITS		5
#define  SPL_MAJOR_MASK     GENMASK(7,3)
#define  SPL_MINOR_MASK     GENMASK(2,0)
#define  SPL_VERSION(maj, min)    (((maj) << SPL_MINOR_BITS) | ( (min) & SPL_MINOR_MASK)) 

#define SPL_HEADER_VERSION	SPL_VERSION(0, 2)

#define SPL_ENV_HEADER_VERSION	SPL_VERSION(0, 1)
#define SPL_DT_HEADER_VERSION	SPL_VERSION(0, 2)
#define SPL_DRAM_HEADER_VERSION	SPL_VERSION(0, 3)

#define SPL_ADDR		CONFIG_SUNXI_SRAM_ADDRESS

/* The low 8-bits of the 'boot_media' field in the SPL header */
#define ALLWINNER_BOOTED_FROM_MMC0	0
#define ALLWINNER_BOOTED_FROM_NAND	1
#define ALLWINNER_BOOTED_FROM_MMC2	2
#define ALLWINNER_BOOTED_FROM_SPI	3
#define ALLWINNER_BOOTED_FROM_MMC0_HIGH	0x10
#define ALLWINNER_BOOTED_FROM_MMC2_HIGH	0x12

/* boot head definition of allwinner H6 soc*/
typedef struct boot_file_head {
	uint32_t b_instruction;	/* one intruction jumping to real code */
	uint8_t magic[8];	/* ="eGON.BT0" or "eGON.BT1", not C-style str */
	uint32_t check_sum;	/* generated by PC */
	uint32_t length;	/* generated by PC */

	union {
		uint32_t pub_head_size;
		uint8_t spl_signature[4];
	};
	uint32_t fel_script_address;	/* since v0.1, set by sunxi-fel */
	/*
	 * If the fel_uEnv_length member below is set to a non-zero value,
	 * it specifies the size (byte count) of data at fel_script_address.
	 * At the same time this indicates that the data is in uEnv.txt
	 * compatible format, ready to be imported via "env import -t".
	 */
	uint32_t fel_uEnv_length;	/* since v0.1, set by sunxi-fel */
	/*
	 * Offset of an ASCIIZ string (relative to the SPL header), which
	 * contains the default device tree name (CONFIG_DEFAULT_DEVICE_TREE).
	 * This is optional and may be set to NULL. Is intended to be used
	 * by flash programming tools for providing nice informative messages
	 * to the users.
	 */
	uint32_t dt_name_offset;	/* since v0.2, set by mksunxiboot */
	uint32_t dram_size;		/* in MiB, since v0.3, set by SPL */
	uint32_t boot_media;		/* written here by the boot ROM */
	uint32_t string_pool[13];	/* since v0.2, filled by mksunxiboot */
	/* The header must be a multiple of 32 bytes (for VBAR alignment) */
} __attribute__((pack)) boot0_file_header_t;











#endif

