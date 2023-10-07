#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <log.h>



int32_t  uart_clk_init(const uint32_t uart_id)
{
    if (uart_id > CCU_UART3_ID) {
        return  -1;
    }

    /*open uartx clock*/
    setbits_32(CCU_UARTX_GATE_REG,  (1 << uart_id));

    /*de-assert reset */
    setbits_32(CCU_UARTX_GATE_REG,  (1 << (uart_id + 16)));
    return   0;

}












