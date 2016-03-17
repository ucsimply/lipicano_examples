/*
 * main.c
 *
 */

#include "chip.h"
#include "uart.h"
#include "xprintf.h"

#define DELAY_COUNT 4000000

int main (void)
{
    volatile int    i;
    int             counter;

    /* initialize xprintf xfunc_out() function pointer */
    xfunc_out = uart_writec;

    xprintf("\n%s\n", "Hello to all blinkers!");
    counter = 0;

    while (1) {

        /* toggle LEDs */
        if (counter & 1) {
            LPC_GPIO[1].SET = (1 << 2);
            LPC_GPIO[1].CLR = (1 << 3);
        }
        else {
            LPC_GPIO[1].CLR = (1 << 2);
            LPC_GPIO[1].SET = (1 << 3);
        }
        counter++;

        /* wait a moment */
        for (i = DELAY_COUNT; i > 0; i--) {
        }

        /* report blink counter to UART0 */
        xprintf("Counter = %d\n", counter);
    }

    /* main() should never reach this point */
    return 0;
}

