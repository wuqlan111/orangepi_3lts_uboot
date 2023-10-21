

#define  LOG_CATEGORY  UCLASS_SERIAL

#include <common.h>
#include <dm.h>
#include <log.h>
#include <serial.h>
#include <clk.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/types.h>
#include <dm/device_compat.h>
#include <linux/kernel.h>
#include <asm/arch/gpio.h>
#include <dm/device.h>
#include "serial_allwinner.h"

typedef struct {
    union {
        u32   rbr;
        u32   thr;
        u32   dll;
    };

    union {
        u32   dlh;
	    u32   ier;        
    };

    union {
	    u32   iir;
	    u32   fcr;        
    };
    u32  lcr;
	u32  mcr;
	u32  lsr;
	u32  msr;
	u32  sch;
    u32  rsv1[23];
	u32  usr;
	u32  tfl;
	u32  rfl;
	u32  hsk;
    u32  rsv2[6];
	u32  halt;
    u32  rsv3[2];
    u32  dbg_dll;
    u32  dbg_dlh;
    u32  rsv4[2];
    u32  ctl_485;
    u32  addr_match485;
    u32  bus_ilde_check;
    u32  tx_dly;
} __attribute__((packed)) allwinner_h6_uart_t;

check_member_typedef(allwinner_h6_uart_t,  mcr,  0x10);
check_member_typedef(allwinner_h6_uart_t,  rfl,  0x84);
check_member_typedef(allwinner_h6_uart_t,  tx_dly,  0xcc);

typedef  struct {
    ulong  clk_rate;
}allwinner_h6_uart_priv_t;


static int32_t  _ccu_get_uart_apb2_clk(ulong * clk)
{
	const uint32_t * apb2_reg = (const uint32_t *)0x03001524;
	const uint32_t flag = readl(apb2_reg);
	const uint32_t src_select = (flag >> 24) & 0x3;
	const uint32_t clk_n  =  (flag >> 8) & 0x3;
	const uint32_t clk_m  =  flag & 0x3;

	ulong src_rate  =  0;
	if (!src_select) {
		src_rate  =  24000000;
	} else if (src_select == 1) {
		src_rate  =  32768;
	} else {
		return  -1;
	}

	*clk =  (src_rate) / ( (1 << clk_n) * (clk_m+1) );

	return  0;

}



static int _allwinner_h6_serial_setbrg(allwinner_h6_uart_t * const uart_reg, 
										const ulong clk_rate, const int baudrate)
{
	assert(uart_reg != NULL);

	ulong  tmp_baudrate  =  baudrate  << 4;
	ulong  divisor  =  clk_rate  / tmp_baudrate;
	ulong  integer_part  =  divisor;

	if ( (baudrate <= 0)  ||  !divisor ) {
		log_err("baudrate[%d] invalid!\n", baudrate);
		return  -EINVAL;
	}

	if (clk_rate % tmp_baudrate) {
		ulong tmp2 =  baudrate * (divisor + 1) * (divisor << 5);
		integer_part  =  clk_rate >= tmp2 ? divisor+1: divisor;
	}

	if (integer_part > 0xffff) {
		log_err("baudrate[%d] is too small!\n", baudrate);
		return  -EINVAL;
	}

	setbits_32(&uart_reg->lcr,  ALLWINNER_UART_LCR_DLAB);
	writel(integer_part & 0xff,  &uart_reg->dll);
	writel(integer_part >> 8,   &uart_reg->dlh);

	clrbits_32(&uart_reg->lcr,  ALLWINNER_UART_LCR_DLAB);

	return 0;

}


static  int  _allwinner_h6_serial_init(allwinner_h6_uart_t * const uart_reg,
			const ulong usart_clk_rate, const int baudrate)
{
	int  ret = 0;
	assert(uart_reg != NULL);

	while (readl(&uart_reg->usr) & ALLWINNER_UART_SR_BUSY) {
		;
	}

	writel(0, &uart_reg->ier);
	writel(0, &uart_reg->fcr);
	writel(0x3,  &uart_reg->lcr);
	writel(0,  &uart_reg->mcr);

	ret = _allwinner_h6_serial_setbrg(uart_reg, usart_clk_rate, baudrate);

	return  ret;

}

static int _allwinner_h6_serial_getc(allwinner_h6_uart_t * const uart_reg)
{
	uint32_t  lsr = readl(&uart_reg->lsr);

	if ( ! (lsr & ALLWINNER_UART_LSR_DR)) {
		return  -EAGAIN;
	}

	clrbits_32(&uart_reg->lcr,  ALLWINNER_UART_LCR_DLAB);

	return readl(&uart_reg->rbr);
}



static int _allwinner_h6_serial_putc(allwinner_h6_uart_t * const uart_reg, const char ch)
{
	uint32_t  lsr = readl(&uart_reg->lsr);

	if ( ! (lsr & ALLWINNER_UART_LSR_THRE)) {
		return  -EAGAIN;
	}

	clrbits_32(&uart_reg->lcr,  ALLWINNER_UART_LCR_DLAB);
	writel(ch,  &uart_reg->thr);

	return  0;
}


static int _allwinner_h6_serial_pending(allwinner_h6_uart_t *  const uart_reg, const bool input)
{
	uint32_t  pending =  0;
	uint32_t  lsr = readl(&uart_reg->lsr); 
	if (input) {
		pending = lsr & ALLWINNER_UART_LSR_DR ? 1 : 0;	
	} else {
		pending = lsr & ALLWINNER_UART_LSR_THRE ? 0 : 1;	
	}

	return  pending;

}


#if CONFIG_IS_ENABLED(DM_SERIAL)

static int allwinner_h6_serial_setbrg(struct udevice *dev, int baudrate)
{
	int  ret =  0;
	allwinner_h6_uart_priv_t * priv = dev_get_priv(dev);
	allwinner_h6_uart_plat_t * plat = dev_get_plat(dev);
	assert(priv != NULL);
	assert(plat != NULL);

	allwinner_h6_uart_t * uart_reg  =  (allwinner_h6_uart_t * )plat->base;

	ret  =  _allwinner_h6_serial_setbrg(uart_reg, priv->clk_rate, baudrate);

	return  ret;
}


static int allwinner_h6_serial_pending(struct udevice *dev, bool input)
{
	allwinner_h6_uart_plat_t * plat = dev_get_plat(dev);
	assert(plat != NULL);

	allwinner_h6_uart_t * uart_reg  =  (allwinner_h6_uart_t * )plat->base;
	
	return  _allwinner_h6_serial_pending(uart_reg, input);

}


static int allwinner_h6_serial_getc(struct udevice *dev)
{
	allwinner_h6_uart_plat_t * plat = dev_get_plat(dev);
	assert(plat != NULL);

	allwinner_h6_uart_t * uart_reg  =  (allwinner_h6_uart_t * )plat->base;

	return  _allwinner_h6_serial_getc(uart_reg);
}



static int allwinner_h6_serial_putc(struct udevice *dev, const char ch)
{
	allwinner_h6_uart_plat_t * plat = dev_get_plat(dev);
	assert(plat != NULL);

	allwinner_h6_uart_t * uart_reg  =  (allwinner_h6_uart_t * )plat->base;

	return  _allwinner_h6_serial_putc(uart_reg,  ch);
}


static int allwinner_h6_serial_enable_clk(struct udevice *dev)
{
	allwinner_h6_uart_priv_t *priv = dev_get_priv(dev);
	struct clk clk_uart = {0};
	ulong clk_rate =  0;
	int ret =  0;

	_DBG_PRINTF("start get clk by index\n");
	ret = clk_get_by_index(dev, 0, &clk_uart);
	if (ret) {
		_DBG_PRINTF("get clk propory failed!\n");
		return -EINVAL;		
	}

	_DBG_PRINTF("start get clk rate\n");
	clk_rate = clk_get_rate(&clk_uart);

	_DBG_PRINTF("uart_clk -- %lu\n", clk_rate);
	int64_t tmp_ret = clk_rate;
	if (tmp_ret  <= 0) {
		_DBG_PRINTF("get clk rate failed!\n");
		return -EINVAL;		
	}

	priv->clk_rate  =  clk_rate;

	clk_free(&clk_uart);

	return 0;
}


static int  allwinner_h6_serial_probe(struct udevice *dev)
{
	int ret  =  0;
	fdt_addr_t addr_base  =  0;

	_DBG_PRINTF("allwinner_h6_serial_probe\n");

	allwinner_h6_uart_plat_t *plat = dev_get_plat(dev);
	allwinner_h6_uart_priv_t *priv = dev_get_priv(dev);
	assert(plat != NULL);
	assert(priv != NULL);

	addr_base = dev_read_addr(dev);
	if (addr_base == FDT_ADDR_T_NONE) {
		_DBG_PRINTF("read addr failed!\n");
		return -ENODEV;		
	}

	_DBG_PRINTF("addr_base -- %#lx\n", addr_base);
	plat->base = (uint32_t)addr_base;

	_DBG_PRINTF("start enable serial clk\n");
	ret = allwinner_h6_serial_enable_clk(dev);
	if (ret) {
		_DBG_PRINTF("enable serial clk failed!\n");
		return ret;
	}

	ret  =  _allwinner_h6_serial_init((allwinner_h6_uart_t *)plat->base, 
								priv->clk_rate, gd->baudrate);
	return   ret;
}


static const struct dm_serial_ops allwinner_h6_serial_ops = {
	.putc = allwinner_h6_serial_putc,
	.pending = allwinner_h6_serial_pending,
	.getc = allwinner_h6_serial_getc,
	.setbrg = allwinner_h6_serial_setbrg,
};


static const struct udevice_id allwinner_h6_serial_ids[] = {
    { .compatible = "allwinner, H6-v200-uart", .data  =  0 },
};


U_BOOT_DRIVER(serial_allwinner) = {
	.name	= "serial_allwinner_h6",
	.id	= UCLASS_SERIAL,
	.of_match = allwinner_h6_serial_ids,
	.plat_auto	= sizeof(allwinner_h6_uart_plat_t),
	.probe  = allwinner_h6_serial_probe,
	.ops	= &allwinner_h6_serial_ops,
	.flags  = DM_FLAG_PRE_RELOC,
	.priv_auto	= sizeof(allwinner_h6_uart_priv_t),
};


#else

#define ALLWINNER_UART0_BASE   (0x05000000u)

static int32_t check_uartx_cfg(allwinner_h6_uart_t * const regs, const ulong usart_clk_rate, const int baudrate)
{
	int32_t  ret  =  0;
	if ( (usart_clk_rate != 24000000) && (baudrate !=  115200)) {
		return  -1;
	}

	setbits_32(&regs->lcr,  ALLWINNER_UART_LCR_DLAB);
	uint32_t  cfg_div  =  readl(&regs->dll) |  ( readl(&regs->dlh) << 8 );
	clrbits_32(&regs->lcr,  ALLWINNER_UART_LCR_DLAB);

	ret  = (cfg_div != 13) || (readl(&regs->lcr) != 0x3 ) ? -1:  0;

	return  ret;

}


static  void _allwinner_uartx_test(allwinner_h6_uart_t * const regs)
{
	int32_t  ret =  0;
	char * info = "allwinner uartx initial finish\r\n";
	const uint32_t len  = strlen(info);

	for (uint32_t i =  0; i < len; i ++) {
		do {
			ret  =  _allwinner_h6_serial_putc(regs, info[i]);
		} while (ret == -EAGAIN);
	}

}


int32_t allwinner_uartx_init(void)
{
	int32_t  ret  =  0;
	ulong clk =  0;
	allwinner_h6_uart_t * const regs =  (allwinner_h6_uart_t * )ALLWINNER_UART0_BASE;

	ret = _ccu_get_uart_apb2_clk(&clk);
	if (ret) {
		return ret;
	}

	ret  =  _allwinner_h6_serial_init(regs,  clk,  gd->baudrate);

	_allwinner_uartx_test(regs);

	return  ret;

}


int32_t allwinner_uartx_setbrg(void)
{
	int32_t  ret  =  0;
	ulong clk =  0;
	allwinner_h6_uart_t * const regs =  (allwinner_h6_uart_t * )ALLWINNER_UART0_BASE;

	ret = _ccu_get_uart_apb2_clk(&clk);
	if (ret) {
		return ret;
	}

	return  _allwinner_h6_serial_setbrg(regs,  clk,  gd->baudrate);

}


int32_t allwinner_uartx_tstc(void)
{
	allwinner_h6_uart_t * const regs =  (allwinner_h6_uart_t * )ALLWINNER_UART0_BASE;

	return  _allwinner_h6_serial_pending(regs,  1);

}


int32_t allwinner_uartx_getc(void)
{
	int32_t  ret  =  0;
	allwinner_h6_uart_t * const regs =  (allwinner_h6_uart_t * )ALLWINNER_UART0_BASE;

	do {
		ret  =  _allwinner_h6_serial_getc(regs);
	} while (ret == -EAGAIN);

	return  ret;

}


void allwinner_uartx_putc(const char c)
{
	int32_t  ret  =  0;
	allwinner_h6_uart_t * const regs =  (allwinner_h6_uart_t * )ALLWINNER_UART0_BASE;

	if (c ==  '\n') {
		allwinner_uartx_putc('\r');
	}

	do {
		ret = _allwinner_h6_serial_putc(regs,  c);
	} while (ret == -EAGAIN);

}


static struct serial_device serial_uart0 = {
	.name   =  "uart0",
	.start  =  allwinner_uartx_init,
	.stop   =  NULL,
	.getc   =  allwinner_uartx_getc,
	.tstc   =  allwinner_uartx_tstc,
	.putc   =  allwinner_uartx_putc,
	.puts   =  default_serial_puts,
};


__weak struct serial_device * default_serial_console(void)
{
	return  &serial_uart0;
}

void allwinner_serial_initialize(void)
{
	serial_register(&serial_uart0);
}


#endif


#ifdef CONFIG_DEBUG_UART_ALLWINNER

#include <debug_uart.h>

#if defined(CONFIG_SPL_BUILD)
static inline void _debug_uart_putc(int ch){}
static inline void _debug_uart_init(void){}

#else

static inline void _debug_uart_putc(int ch)
{

	allwinner_h6_uart_t * uart_reg = (allwinner_h6_uart_t *)CONFIG_VAL(DEBUG_UART_BASE);

	while ( !(readl(&uart_reg->lsr) & ALLWINNER_UART_LSR_THRE) )  ;

	clrbits_32(&uart_reg->lcr,  ALLWINNER_UART_LCR_DLAB);
	writel(ch,  &uart_reg->thr);
	
}


static inline void _debug_uart_init(void)
{
	ulong apb2_clk =  0;
	allwinner_h6_uart_t * uart_reg = (allwinner_h6_uart_t *)CONFIG_VAL(DEBUG_UART_BASE);

	if (_ccu_get_uart_apb2_clk(&apb2_clk)) {
		return;
	}

	_allwinner_h6_serial_init(uart_reg, apb2_clk, CONFIG_BAUDRATE);

	allwinner_gpio_output_value( GPIOL, 4,  GPIO_PULL_DISABLE,  0);
	allwinner_gpio_output_value( GPIOL, 7,  GPIO_PULL_DISABLE,  1);

}
#endif


DEBUG_UART_FUNCS
#endif

