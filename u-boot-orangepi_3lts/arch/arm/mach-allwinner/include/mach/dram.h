
#ifndef _ARCH_ASM_MACH_DRAM_H
#define _ARCH_ASM_MACH_DRAM_H

/*
 * H6 dram controller register and constant defines
 *
 * (C) Copyright 2017  Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <stdint.h>
#include <linux/kernel.h>
#include <linux/bitops.h>

typedef enum {
	SUNXI_DRAM_TYPE_DDR3 = 3,
	SUNXI_DRAM_TYPE_DDR4,
	SUNXI_DRAM_TYPE_LPDDR2 = 6,
	SUNXI_DRAM_TYPE_LPDDR3,
}sunxi_dram_type_e;

static inline uint32_t sunxi_dram_is_lpddr(int32_t type)
{
	return type >= SUNXI_DRAM_TYPE_LPDDR2;
}

/*
 * The following information is mainly retrieved by disassembly and some FPGA
 * test code of sun50iw3 platform.
 */
typedef struct {
	uint32_t cr;			/* 0x000 control register */
	uint8_t reserved_0x004[4];	/* 0x004 */
	uint32_t unk_0x008;		/* 0x008 */
	uint32_t tmr;		/* 0x00c timer register */
	uint8_t reserved_0x010[4];	/* 0x010 */
	uint32_t unk_0x014;		/* 0x014 */
	uint8_t reserved_0x018[8];	/* 0x018 */
	uint32_t maer0;		/* 0x020 master enable register 0 */
	uint32_t maer1;		/* 0x024 master enable register 1 */
	uint32_t maer2;		/* 0x028 master enable register 2 */
	uint8_t reserved_0x02c[468];	/* 0x02c */
	uint32_t bwcr;		/* 0x200 bandwidth control register */
	uint8_t reserved_0x204[12];	/* 0x204 */
	/*
	 * The last master configured by BSP libdram is at 0x49x, so the
	 * size of this struct array is set to 41 (0x29) now.
	 */
	struct {
		uint32_t cfg0;		/* 0x0 */
		uint32_t cfg1;		/* 0x4 */
		uint8_t reserved_0x8[8];	/* 0x8 */
	} master[41];		/* 0x210 + index * 0x10 */
}sunxi_mctl_com_reg_t;

/*
 * The following register information are retrieved from some similar DRAM
 * controllers, including the DRAM controllers in Allwinner A23/A80 SoCs,
 * Rockchip RK3328 SoC, NXP i.MX7 SoCs and Xilinx Zynq UltraScale+ SoCs.
 *
 * The DRAM controller in Allwinner A23/A80 SoCs and NXP i.MX7 SoCs seems
 * to be older than the one in Allwinner H6, as the DRAMTMG9 register
 * is missing in these SoCs. (From the product specifications of these
 * SoCs they're not capable of DDR4)
 *
 * Information sources:
 * - dram_sun9i.h and dram_sun8i_a23.h in the same directory.
 * - sdram_rk3328.h from the RK3328 TPL DRAM patchset
 * - i.MX 7Solo Applications Processor Reference Manual (IMX7SRM)
 * - Zynq UltraScale+ MPSoC Register Reference (UG1087)
 */
typedef struct {
	uint32_t mstr;		/* 0x000 */
	uint32_t statr;		/* 0x004 unused */
	uint32_t mstr1;		/* 0x008 unused */
	uint32_t unk_0x00c;		/* 0x00c */
	uint32_t mrctrl0;		/* 0x010 unused */
	uint32_t mrctrl1;		/* 0x014 unused */
	uint32_t mrstatr;		/* 0x018 unused */
	uint32_t mrctrl2;		/* 0x01c unused */
	uint32_t derateen;		/* 0x020 unused */
	uint32_t derateint;		/* 0x024 unused */
	uint8_t reserved_0x028[8];	/* 0x028 */
	uint32_t pwrctl;		/* 0x030 unused */
	uint32_t pwrtmg;		/* 0x034 unused */
	uint32_t hwlpctl;		/* 0x038 unused */
	uint8_t reserved_0x03c[20];	/* 0x03c */
	uint32_t rfshctl0;		/* 0x050 unused */
	uint32_t rfshctl1;		/* 0x054 unused */
	uint8_t reserved_0x058[8];	/* 0x05c */
	uint32_t rfshctl3;		/* 0x060 */
	uint32_t rfshtmg;		/* 0x064 */
	uint8_t reserved_0x068[104];	/* 0x068 reserved for ECC&CRC (from ZynqMP) */
	uint32_t init[8];		/* 0x0d0 */
	uint32_t dimmctl;		/* 0x0f0 unused */
	uint32_t rankctl;		/* 0x0f4 */
	uint8_t reserved_0x0f8[8];	/* 0x0f8 */
	uint32_t dramtmg[17];	/* 0x100 */
	uint8_t reserved_0x144[60];	/* 0x144 */
	uint32_t zqctl[3];		/* 0x180 */
	uint32_t zqstat;		/* 0x18c unused */
	uint32_t dfitmg0;		/* 0x190 */
	uint32_t dfitmg1;		/* 0x194 */
	uint32_t dfilpcfg[2];	/* 0x198 unused */
	uint32_t dfiupd[3];		/* 0x1a0 */
	uint32_t reserved_0x1ac;	/* 0x1ac */
	uint32_t dfimisc;		/* 0x1b0 */
	uint32_t dfitmg2;		/* 0x1b4 unused, may not exist */
	uint8_t reserved_0x1b8[8];	/* 0x1b8 */
	uint32_t dbictl;		/* 0x1c0 */
	uint8_t reserved_0x1c4[60];	/* 0x1c4 */
	uint32_t addrmap[12];	/* 0x200 */
	uint8_t reserved_0x230[16];	/* 0x230 */
	uint32_t odtcfg;		/* 0x240 */
	uint32_t odtmap;		/* 0x244 */
	uint8_t reserved_0x248[8];	/* 0x248 */
	uint32_t sched[2];		/* 0x250 */
	uint8_t reserved_0x258[180];	/* 0x258 */
	uint32_t dbgcmd;		/* 0x30c unused */
	uint32_t dbgstat;		/* 0x310 unused */
	uint8_t reserved_0x314[12];	/* 0x314 */
	uint32_t swctl;		/* 0x320 */
	uint32_t swstat;		/* 0x324 */
} sunxi_mctl_ctl_reg_t;

#define MSTR_DEVICETYPE_DDR3	BIT(0)
#define MSTR_DEVICETYPE_LPDDR2	BIT(2)
#define MSTR_DEVICETYPE_LPDDR3	BIT(3)
#define MSTR_DEVICETYPE_DDR4	BIT(4)
#define MSTR_DEVICETYPE_MASK	GENMASK(5, 0)
#define MSTR_2TMODE		BIT(10)
#define MSTR_BUSWIDTH_FULL	(0 << 12)
#define MSTR_BUSWIDTH_HALF	(1 << 12)
#define MSTR_ACTIVE_RANKS(x)	(((x == 2) ? 3 : 1) << 24)
#define MSTR_BURST_LENGTH(x)	(((x) >> 1) << 16)

/*
 * The following register information is based on Zynq UltraScale+
 * MPSoC Register Reference, as it's the currently only known
 * DDR PHY similar to the one used in H6; however although the
 * map is similar, the bit fields definitions are different.
 *
 * Other DesignWare DDR PHY's have similar register names, but the
 * offset and definitions are both different.
 */
typedef struct {
	uint32_t ver;		/* 0x000 guess based on similar PHYs */
	uint32_t pir;		/* 0x004 */
	uint8_t reserved_0x008[8];	/* 0x008 */
	/*
	 * The ZynqMP manual didn't document PGCR1, however this register
	 * exists on H6 and referenced by libdram.
	 */
	uint32_t pgcr[8];		/* 0x010 */
	/*
	 * By comparing the hardware and the ZynqMP manual, the PGSR seems
	 * to start at 0x34 on H6.
	 */
	uint8_t reserved_0x030[4];	/* 0x030 */
	uint32_t pgsr[3];		/* 0x034 */
	uint32_t ptr[7];		/* 0x040 */
	/*
	 * According to ZynqMP reference there's PLLCR0~6 in this area,
	 * but they're tagged "Type B PLL Only" and H6 seems to have
	 * no them.
	 * 0x080 is not present in ZynqMP reference but it seems to be
	 * present on H6.
	 */
	uint8_t reserved_0x05c[36];	/* 0x05c */
	uint32_t unk_0x080;		/* 0x080 */
	uint8_t reserved_0x084[4];	/* 0x084 */
	uint32_t dxccr;		/* 0x088 */
	uint8_t reserved_0x08c[4];	/* 0x08c */
	uint32_t dsgcr;		/* 0x090 */
	uint8_t reserved_0x094[4];	/* 0x094 */
	uint32_t odtcr;		/* 0x098 */
	uint8_t reserved_0x09c[4];	/* 0x09c */
	uint32_t aacr;		/* 0x0a0 */
	uint8_t reserved_0x0a4[32];	/* 0x0a4 */
	uint32_t gpr1;		/* 0x0c4 */
	uint8_t reserved_0x0c8[56];	/* 0x0c8 */
	uint32_t dcr;		/* 0x100 */
	uint8_t reserved_0x104[12];	/* 0x104 */
	uint32_t dtpr[7];		/* 0x110 */
	uint8_t reserved_0x12c[20];	/* 0x12c */
	uint32_t rdimmgcr[3];	/* 0x140 */
	uint8_t reserved_0x14c[4];	/* 0x14c */
	uint32_t rdimmcr[5];		/* 0x150 */
	uint8_t reserved_0x164[4];	/* 0x164 */
	uint32_t schcr[2];		/* 0x168 */
	uint8_t reserved_0x170[16];	/* 0x170 */
	/*
	 * The ZynqMP manual documents MR0~7, 11~14 and 22.
	 */
	uint32_t mr[23];		/* 0x180 */
	uint8_t reserved_0x1dc[36];	/* 0x1dc */
	uint32_t dtcr[2];		/* 0x200 */
	uint32_t dtar[3];		/* 0x208 */
	uint8_t reserved_0x214[4];	/* 0x214 */
	uint32_t dtdr[2];		/* 0x218 */
	uint8_t reserved_0x220[16];	/* 0x220 */
	uint32_t dtedr0;		/* 0x230 */
	uint32_t dtedr1;		/* 0x234 */
	uint32_t dtedr2;		/* 0x238 */
	uint32_t vtdr;		/* 0x23c */
	uint32_t catr[2];		/* 0x240 */
	uint8_t reserved_0x248[8];
	uint32_t dqsdr[3];		/* 0x250 */
	uint32_t dtedr3;		/* 0x25c */
	uint8_t reserved_0x260[160];	/* 0x260 */
	uint32_t dcuar;		/* 0x300 */
	uint32_t dcudr;		/* 0x304 */
	uint32_t dcurr;		/* 0x308 */
	uint32_t dculr;		/* 0x30c */
	uint32_t dcugcr;		/* 0x310 */
	uint32_t dcutpr;		/* 0x314 */
	uint32_t dcusr[2];		/* 0x318 */
	uint8_t reserved_0x320[444];	/* 0x320 */
	uint32_t rankidr;		/* 0x4dc */
	uint32_t riocr[6];		/* 0x4e0 */
	uint8_t reserved_0x4f8[8];	/* 0x4f8 */
	uint32_t aciocr[6];		/* 0x500 */
	uint8_t reserved_0x518[8];	/* 0x518 */
	uint32_t iovcr[2];		/* 0x520 */
	uint32_t vtcr[2];		/* 0x528 */
	uint8_t reserved_0x530[16];	/* 0x530 */
	uint32_t acbdlr[17];		/* 0x540 */
	uint32_t aclcdlr;		/* 0x584 */
	uint8_t reserved_0x588[24];	/* 0x588 */
	uint32_t acmdlr[2];		/* 0x5a0 */
	uint8_t reserved_0x5a8[216];	/* 0x5a8 */
	struct {
		uint32_t zqcr;	/* 0x00 only the first one valid */
		uint32_t zqpr[2];	/* 0x04 */
		uint32_t zqdr[2];	/* 0x0c */
		uint32_t zqor[2];	/* 0x14 */
		uint32_t zqsr;	/* 0x1c */
	} zq[2];		/* 0x680, 0x6a0 */
	uint8_t reserved_0x6c0[64];	/* 0x6c0 */
	struct {
		uint32_t gcr[7];		/* 0x00 */
		uint8_t reserved_0x1c[36];	/* 0x1c */
		uint32_t bdlr0;		/* 0x40 */
		uint32_t bdlr1;		/* 0x44 */
		uint32_t bdlr2;		/* 0x48 */
		uint8_t reserved_0x4c[4];	/* 0x4c */
		uint32_t bdlr3;		/* 0x50 */
		uint32_t bdlr4;		/* 0x54 */
		uint32_t bdlr5;		/* 0x58 */
		uint8_t reserved_0x5c[4];	/* 0x5c */
		uint32_t bdlr6;		/* 0x60 */
		uint8_t reserved_0x64[28];	/* 0x64 */
		uint32_t lcdlr[6];		/* 0x80 */
		uint8_t reserved_0x98[8];	/* 0x98 */
		uint32_t mdlr[2];		/* 0xa0 */
		uint8_t reserved_0xa8[24];	/* 0xa8 */
		uint32_t gtr0;		/* 0xc0 */
		uint8_t reserved_0xc4[12];	/* 0xc4 */
		/*
		 * DXnRSR0 is not documented in ZynqMP manual but
		 * it's used in libdram.
		 */
		uint32_t rsr[4];		/* 0xd0 */
		uint32_t gsr[4];		/* 0xe0 */
		uint8_t reserved_0xf0[16];	/* 0xf0 */
	} dx[4];		/* 0x700, 0x800, 0x900, 0xa00 */
}sunxi_mctl_phy_reg_t;

#define PIR_INIT	BIT(0)
#define PIR_ZCAL	BIT(1)
#define PIR_CA		BIT(2)
#define PIR_PLLINIT	BIT(4)
#define PIR_DCAL	BIT(5)
#define PIR_PHYRST	BIT(6)
#define PIR_DRAMRST	BIT(7)
#define PIR_DRAMINIT	BIT(8)
#define PIR_WL		BIT(9)
#define PIR_QSGATE	BIT(10)
#define PIR_WLADJ	BIT(11)
#define PIR_RDDSKW	BIT(12)
#define PIR_WRDSKW	BIT(13)
#define PIR_RDEYE	BIT(14)
#define PIR_WREYE	BIT(15)
#define PIR_VREF	BIT(17)
#define PIR_CTLDINIT	BIT(18)
#define PIR_DQS2DQ	BIT(20)
#define PIR_DCALPSE	BIT(29)
#define PIR_ZCALBYP	BIT(30)

#define DCR_LPDDR3	(1 << 0)
#define DCR_DDR3	(3 << 0)
#define DCR_DDR4	(4 << 0)
#define DCR_DDR8BANK	BIT(3)
#define DCR_DDR2T	BIT(28)

/*
 * The delay parameters allow to allegedly specify delay times of some
 * unknown unit for each individual bit trace in each of the four data bytes
 * the 32-bit wide access consists of. Also three control signals can be
 * adjusted individually.
 */
#define NR_OF_BYTE_LANES	(32 / BITS_PER_BYTE)
/* The eight data lines (DQn) plus DM, DQS, DQS/DM/DQ Output Enable and DQSN */
#define WR_LINES_PER_BYTE_LANE	(BITS_PER_BYTE + 4)
/*
 * The eight data lines (DQn) plus DM, DQS, DQS/DM/DQ Output Enable, DQSN,
 * Termination and Power down
 */
#define RD_LINES_PER_BYTE_LANE	(BITS_PER_BYTE + 6)
typedef struct {
	uint32_t clk;
	sunxi_dram_type_e type;
	uint8_t cols;
	uint8_t rows;
	uint8_t ranks;
	uint8_t bus_full_width;
	const uint8_t dx_read_delays[NR_OF_BYTE_LANES][RD_LINES_PER_BYTE_LANE];
	const uint8_t dx_write_delays[NR_OF_BYTE_LANES][WR_LINES_PER_BYTE_LANE];
}dram_para_t;


static inline uint32_t ns_to_t(const uint32_t nanoseconds)
{
	const uint32_t ctrl_freq = CONFIG_DRAM_CLK / 2;

	return DIV_ROUND_UP(ctrl_freq * nanoseconds, 1000);
}

void mctl_set_timing_params(dram_para_t *para);



#endif


