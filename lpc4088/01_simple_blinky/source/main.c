/*
 * main.c
 *
 */

#include "chip.h"

#define DELAY_COUNT 2000000

int main (void)
{
    volatile int    i;
    int             first;

    first = 0;

    while (1) {

        /* toggle LEDs */
        if (first) {
            LPC_GPIO[1].SET = (1 << 2);
            LPC_GPIO[1].CLR = (1 << 3);
        }
        else {
            LPC_GPIO[1].CLR = (1 << 2);
            LPC_GPIO[1].SET = (1 << 3);
        }
        first = ~first;

        /* wait a moment */
        for (i = DELAY_COUNT; i > 0; i--);

    }

    /* main() should never reach this point */
    return 0;
}

