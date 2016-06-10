/*
 * systick.c
 *
 */

#include "chip.h"
#include "systick.h"

#define SYSTICK_RELOAD    (1200000 - 1)    // 10 ms for 120 MHz clock

void systick_handler(void) __attribute__((interrupt));

void systick_init(void)
{
      SysTick->LOAD  = SYSTICK_RELOAD;
      SysTick->VAL   = 0;
      SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                       SysTick_CTRL_ENABLE_Msk;
}

void systick_ienb(int enable)
{
    if (enable)
        SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    else
        SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

/* SysTick interrupt handler */
void systick_handler(void)
{
}

