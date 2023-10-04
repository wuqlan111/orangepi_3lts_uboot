

#define  LOG_CATEGORY  UCLASS_SERIAL

#include <common.h>
#include <dm.h>
#include <log.h>
#include <serial.h>
#include <serial.h>
#include <debug_uart.h>
#include <clk.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/types.h>
#include <dm/device_compat.h>
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
    u32  rsv1[24];
	u32  usr;
	u32  tfl;
	u32  rfl;
	u32  hsk;
    u32  rsv2[7];
	u32  halt;
    u32  rsv3[3];
    u32  dbg_dll;
    u32  dbg_dlh;
    u32  rsv4[2];
    u32  ctl_485;
    u32  addr_match485;
    u32  bus_ilde_check;
    u32  tx_dly;
}allwinner_h6_uart_t;


typedef  struct {
    ulong  clk_rate;
}allwinner_h6_uart_priv_t;

#if  defined(CONFIG_DEBUG_UART_ALLWINNER) || !defined(CONFIG_DM_SERIAL) 

static int32_t  _ccu_get_uart_apb2_clk(ulong * clk)
{
	const uint32_t * apb2_reg = (const uint32_t *)0x03000524;
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

#endif

static int _allwinner_h6_serial_setbrg(allwinner_h6_uart_t * uart_reg, 
											ulong clk_rate, int baudrate)
{
	assert(uart_reg != NULL);

	if ( baudrate <= 0) {
		log_err("baudrate[%d] invalid!\n", baudrate);
		return  -EINVAL;
	}

	ulong  tmp_baudrate  =  clk_rate  << 4;
	ulong  divisor  =  clk_rate  / tmp_baudrate;
	ulong  integer_part  =  divisor;

	if (clk_rate % tmp_baudrate) {
		integer_part +=1;
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


static  int  _allwinner_h6_serial_init(allwinner_h6_uart_t * uart_reg,
			ulong usart_clk_rate, int baudrate)
{
	int  ret = 0;
	assert(uart_reg != NULL);

	while (readl(&uart_reg->usr) & ALLWINNER_UART_SR_BUSY)
		;

	writel(0, &uart_reg->ier);
	writel(0, &uart_reg->fcr);
	writel(0xb,  &uart_reg->lcr);
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

	return readl(&uart_reg->rbr);
}



static int _allwinner_h6_serial_putc(allwinner_h6_uart_t * const uart_reg, const char ch)
{
	uint32_t  lsr = readl(&uart_reg->lsr);

	if ( ! (lsr & ALLWINNER_UART_LSR_THRE)) {
		return  -EAGAIN;
	}

	writel(ch,  &uart_reg->thr);

	return  0;
}


static int _allwinner_h6_serial_pending(allwinner_h6_uart_t *  const uart_reg, const bool input)
{
	uint32_t  pending =  0;
	uint32_t  lsr = readl(&uart_reg->lsr);
	uint32_t  fcr = readl(&uart_reg->fcr);

	if (fcr & ALLWINNER_UART_FCR_FIFOE) {
		uint32_t  fifo_level  =  input? readl(&uart_reg->rfl): 
										readl(&uart_reg->tfl);
		pending = fifo_level & 0xff ? 1:  0;
	} else if (input) {
		pending = lsr & ALLWINNER_UART_LSR_DR ? 1 : 0;	
	} else {
		pending = lsr & ALLWINNER_UART_LSR_TEMT ? 0 : 1;			
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

	ret = clk_get_by_index(dev, 0, &clk_uart);
	if (ret) {
		return -EINVAL;		
	}

	clk_rate = clk_get_rate(&clk_uart);
	if (!clk_rate) {
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

	allwinner_h6_uart_plat_t *plat = dev_get_plat(dev);
	allwinner_h6_uart_priv_t *priv = dev_get_priv(dev);
	assert(plat != NULL);
	assert(priv != NULL);

	addr_base = dev_read_addr(dev);
	if (addr_base == FDT_ADDR_T_NONE) {
		return -ENODEV;		
	}

	plat->base = (uint32_t)addr_base;

	ret = allwinner_h6_serial_enable_clk(dev);
	if (ret) {
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

#define ALLWINNER_UART0_BASE   CFG_MXC_UART_BASE

int32_t allwinner_uartx_init(void)
{
	int32_t  ret  =  0;
	ulong clk =  0;
	allwinner_h6_uart_t * const regs =  (allwinner_h6_uart_t * )ALLWINNER_UART0_BASE;

	ret = _ccu_get_uart_apb2_clk(&clk);
	if (ret) {
		return ret;
	}

	return  _allwinner_h6_serial_init(regs,  clk,  gd->baudrate);

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

#define  CCU_BASE_ADDR    0x03000000

// static int32_t  _ccu_get_uart_apb2_clk(ulong * clk)
// {
// 	const uint32_t * apb2_reg = (const uint32_t *)0x03000524;
// 	const uint32_t flag = readl(apb2_reg);
// 	const uint32_t src_select = (flag >> 24) & 0x3;
// 	const uint32_t clk_n  =  (flag >> 8) & 0x3;
// 	const uint32_t clk_m  =  flag & 0x3;

// 	ulong src_rate  =  0;
// 	if (!src_select) {
// 		src_rate  =  24000000;
// 	} else if (src_select == 1) {
// 		src_rate  =  32768;
// 	} else {
// 		return  -1;
// 	}

// 	*clk =  (src_rate) / ( (1 << clk_n) * (clk_m+1) );

// 	return  0;

// }

static inline void _debug_uart_putc(int ch)
{
	allwinner_h6_uart_t * uart_reg = (allwinner_h6_uart_t *)CONFIG_VAL(DEBUG_UART_BASE);

	while (!(readl(&uart_reg->lsr) & ALLWINNER_UART_LSR_THRE))
		;

	writel(ch, &uart_reg->thr);
}


static inline void _debug_uart_init(void)
{
	ulong apb2_clk =  0;
	allwinner_h6_uart_t * uart_reg = (allwinner_h6_uart_t *)CONFIG_VAL(DEBUG_UART_BASE);

	if (_ccu_get_uart_apb2_clk(&apb2_clk)) {
		return;
	}

	_allwinner_h6_serial_init(uart_reg, apb2_clk, CONFIG_BAUDRATE);

	char test[] = "heehe\r\n";
	for (int32_t i =  0; i < strlen(test); i++) {
		_debug_uart_putc(test[i]);
	}


}


DEBUG_UART_FUNCS
#endif

