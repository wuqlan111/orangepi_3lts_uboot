#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <init.h>


#define EN_MASK		0x08000000	/* Enable timer */
#define SRSTEN_MASK	0x04000000	/* Enable soft reset */
#define CLKS_SHIFT	20		/* Clock period shift */
#define LD_SHIFT	0		/* Reload value shift */

void reset_cpu(void)
{

}

#if defined(CONFIG_DISPLAY_CPUINFO)

int32_t print_cpuinfo(void)
{

	return 0;
}

#endif



