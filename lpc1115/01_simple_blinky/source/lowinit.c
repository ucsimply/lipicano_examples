/*
 * lowinit.c
 *
 */

#include "chip.h"

#define SYSPLLCTRL_MSEL    3
#define SYSPLLCTRL_PSEL    (1 << 5)

void lowinit(void);

static void clk_init(void);
static void iopin_init(void);

void lowinit(void)
{
    clk_init();
    iopin_init();
}

/* PLL and clock initialization - main clock 48 MHz */
static void clk_init(void)
{
    volatile int i;

    /* System oscillator - not bypassed, low freq */
    LPC_SYSCTL->SYSOSCCTRL = 0;

    /* Powerup system oscillator - clear PDRUNCFG bit 5 */
    LPC_SYSCTL->PDRUNCFG &= 0xFFDF;

    /* Wait about 500 us */
    for (i = 0; i < 2500; i++);

    /* Set system PLL input to main oscillator */
    LPC_SYSCTL->SYSPLLCLKSEL  = SYSCTL_PLLCLKSRC_MAINOSC;
    LPC_SYSCTL->SYSPLLCLKUEN  = 0;
    LPC_SYSCTL->SYSPLLCLKUEN  = 1;

    /* Power down PLL - set PDRUNCFG bit 7 */
    /* Not necessary - it should be down after reset
    LPC_SYSCTL->PDRUNCFG |= 0xC880;
     */

    /* Setup PLL for main oscillator
     * FCLKOUT = 48 MHz
     * FCLKIN  = 12 MHz
     * MSEL    = 3 (bits 4:0)
     * PSEL    = 1 (bits 6:5)
     */
    LPC_SYSCTL->SYSPLLCTRL = SYSPLLCTRL_PSEL | SYSPLLCTRL_MSEL;

    /* Powerup system PLL  - clear PDRUNCFG bit 7 */
    LPC_SYSCTL->PDRUNCFG &= 0xFF7F;

    /* Wait for PLL to lock */
    while ((LPC_SYSCTL->SYSPLLSTAT & 1) == 0);

    /* Set system clock divider to 1 */
    LPC_SYSCTL->SYSAHBCLKDIV  = 1;

    /* Set main clock source to the system PLL */
    LPC_SYSCTL->MAINCLKSEL  = SYSCTL_MAINCLKSRC_PLLOUT;
    LPC_SYSCTL->MAINCLKUEN  = 0;
    LPC_SYSCTL->MAINCLKUEN  = 1;

    /* Setup FLASH access to 2 clocks
     * Bits 31:2 must be written back exactly as read. */
    LPC_FMC->FLASHTIM = (LPC_FMC->FLASHTIM & ~0x03) | FLASHTIM_50MHZ_CPU;
}

static void iopin_init(void)
{
    /* Enable IOCON clock */
    LPC_SYSCTL->SYSAHBCLKCTRL |= (1 << SYSCTL_CLOCK_IOCON);

    /* Enable GPIO clock */
    LPC_SYSCTL->SYSAHBCLKCTRL |= (1 << SYSCTL_CLOCK_GPIO);

    /* set pin function */
    LPC_IOCON->REG[IOCON_PIO3_4] = IOCON_FUNC0 | IOCON_MODE_INACT;
    LPC_IOCON->REG[IOCON_PIO3_5] = IOCON_FUNC0 | IOCON_MODE_INACT;

    /* set direction of the pins as output */
    LPC_GPIO[3].DIR |= (1UL << 4);
    LPC_GPIO[3].DIR |= (1UL << 5);

    /* switch LEDs off */
    LPC_GPIO[3].DATA[1 << 4] = 1 << 4;
    LPC_GPIO[3].DATA[1 << 5] = 1 << 5;

}

