/*
 * main.c
 *
 */

#include "chip.h"
#include "systick.h"

#define INIT_PERIOD     50

void systick_func(void);

volatile static int  flag_period_pass;
volatile static int  period_setter;
volatile static int  period_counter;

int main (void)
{
    int led;

    led = 0;

    flag_period_pass = 0;
    period_setter = INIT_PERIOD;
    period_counter = 0;

    /* init systick timer */
    systick_init();
    /* enable systick interrupt */
    systick_ienb(SYSTICK_ENABLE);

    while (1) {
        /* toggle LEDs */
        if (led) {
            LPC_GPIO[3].DATA[1 << 5] = (1 << 5);
            LPC_GPIO[3].DATA[1 << 4] = 0;
        }
        else {
            LPC_GPIO[3].DATA[1 << 5] = 0;
            LPC_GPIO[3].DATA[1 << 4] = (1 << 4);
        }
        led = ~led;

        /* wait for period passed flag */
        while (!flag_period_pass);
        /* clear the flag */
        flag_period_pass = 0;
    }

    /* main() should never reach this point */
    return 0;
}

/* this function is called from
 * systick.c:systick_handler()
 * every 10 milliseconds
 */
void systick_func(void)
{
    if (period_counter == 0) {
        flag_period_pass = 1;
        period_counter = period_setter;
    } else {
        period_counter--;
    }
}
