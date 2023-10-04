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

    uint32_t  flag  =  (1 << (16 + uart_id))  | (1 << uart_id);
    setbits_32(CCU_UARTX_GATE_REG,  flag);
    return   0;

}












