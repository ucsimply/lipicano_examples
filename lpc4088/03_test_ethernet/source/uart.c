/*
 * uart.c
 * simple output on UART0
 */

#include <stdio.h>
#include <string.h>
#include "chip.h"
#include "uart.h"

#define FRGDIV_VAL  1
#define DIV_VAL     0xFF
#define MULT_VAL    47
#define BRG_VAL     (22 - 1)


void uart_init(void)
{
    uint32_t div;
    uint32_t divh;
    uint32_t divl;
    uint32_t clkin;
    uint32_t baudrate;

    /* enable UART0 pins
     * - TxD on pin P0_2
     * - RxD on pin P0_3
     */
    LPC_IOCON->p[0][2] = (IOCON_FUNC1 | IOCON_MODE_INACT);
    LPC_IOCON->p[0][3] = (IOCON_FUNC1 | IOCON_MODE_INACT);

    /* enable UART0 clock
     * it's enabled after reset because the reset
     * value of the SYSCTL_CLOCK_UART0 bit is 1
     */
    LPC_SYSCTL->PCONP |= (1 << SYSCTL_CLOCK_UART0);

    /* Enable and reset FIFOs */
    LPC_UART0->FCR = (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS);

    /* Disable Tx */
    LPC_UART0->TER1 = 0;
    /* Disable interrupts */
    LPC_UART0->IER = 0;

    /* Disable fractional divider */
    LPC_UART0->FDR = 0x10;

    /*** set USART0 baud rate to 115200 ***/

    clkin = 60000000;
    baudrate = 115200;
    div = clkin / (baudrate * 16);

    /* High and low halves of the divider */
    divh = div / 256;
    divl = div - (divh * 256);

    /* Enable access to Divisor Latches */
    LPC_UART0->LCR |= UART_LCR_DLAB_EN;

    /* Set LSB and MSB divisor latch registers */
    LPC_UART0->DLL = divl;
    LPC_UART0->DLM = divh;

    /* Disable access to Divisor Latches */
    LPC_UART0->LCR &= ~UART_LCR_DLAB_EN;

    /* configure communication parameters */
    LPC_UART0->LCR = (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT);

    /* configure FIFOs */
    LPC_UART0->FCR = (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2);

    /* enable transmition */
    LPC_UART0->TER1 = UART_TER1_TXEN;
}

void uart_writec(unsigned char ch)
{
    while (!(LPC_UART0->LSR & UART_LSR_THRE))
        ;
    LPC_UART0->THR = (uint32_t) ch;
}
