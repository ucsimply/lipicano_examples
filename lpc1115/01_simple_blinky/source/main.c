/*
 * main.c
 *
 */

#include "chip.h"

int main (void)
{
    volatile int    i;
    int             led;

    led = 1;

    while (1) {

        /* toggle LEDs */
        switch (led) {
            case 1:
            case 2:
                LPC_GPIO[3].DATA[1 << 4] ^= (1 << 4);
                break;
            case 3:
            case 4:
                LPC_GPIO[3].DATA[1 << 5] ^= (1 << 5);
                break;
        }

        led++;
        if (led == 5)
            led = 1;

        /* wait a moment */
        for (i = 500000; i > 0; i--);

    }

    /* main() should never reach this point */
    return 0;
}

