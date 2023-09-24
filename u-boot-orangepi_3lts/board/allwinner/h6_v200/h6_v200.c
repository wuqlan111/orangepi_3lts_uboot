
#include <common.h>
#include <dm.h>
#include <stdint.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>


static struct mm_region allwinner_h6_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xf0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xf0000000UL,
		.phys = 0xf0000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},  {
		.virt = 0x900000000,
		.phys = 0x900000000,
		.size = 0x150000000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},  {
		/* List terminator */
		0,
	}
};

struct mm_region * mem_map = allwinner_h6_mem_map;


int32_t board_init(void)
{
	return 0;
}

int32_t dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

int32_t dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;

	return 0;
}

/* Nothing to be done here as handled by PSCI interface */
void reset_cpu(void)
{
}


int32_t print_cpuinfo(void)
{
	return 0;
}



