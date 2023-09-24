#ifndef _DT_BINDINGS_ALLWINNER_H6_PINFUNC_H
#define _DT_BINDINGS_ALLWINNER_H6_PINFUNC_H





#define AF0	0x1
#define AF1	0x2
#define AF2	0x3
#define AF3	0x4
#define AF4	0x5
#define AF5	0x6
#define AF6	0x7
#define AF7	0x8
#define AF8	0x9
#define AF9	0xa



/* define Pins number*/
#define PIN_NO(port, line)	(((port) - 'A') * 0x10 + (line))

#define STM32_PINMUX(port, line, mode) (((PIN_NO(port, line)) << 8) | (mode))

/*  package information */
#define STM32MP_PKG_AA	0x1
#define STM32MP_PKG_AB	0x2
#define STM32MP_PKG_AC	0x4
#define STM32MP_PKG_AD	0x8







#endif
