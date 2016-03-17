/*
 * lowinit.c
 *
 */

#include "chip.h"
#include "uart.h"

void lowinit(void);

static void clk_init(void);
static void iopin_init(void);

void lowinit(void)
{
    clk_init();
    iopin_init();
    uart_init();
}

/*
 * PLL and clock initialization
 * - use external 12 MHz crystal
 * - main clock 120 MHz
 */
static void clk_init(void)
{
    uint32_t tmp;

    /* Power Boost - enable operation up to 120 MHz */
    LPC_SYSCTL->PBOOST = 0x3;

    /* clear the OSCRS bit - range is 1 MHz to 20 MHz */
    LPC_SYSCTL->SCS &= (~SYSCTL_OSCRANGE_15_25);

    /* enable the external crystal */
    if ((LPC_SYSCTL->SCS & SYSCTL_OSCSTAT) == 0) {
        LPC_SYSCTL->SCS |= SYSCTL_OSCEC;
    }
    while ((LPC_SYSCTL->SCS & SYSCTL_OSCSTAT) == 0);

    /* use Sysclk (not Main PLL) as the input to the CPU clock divider */
    LPC_SYSCTL->CCLKSEL = 1;

    /* disable the Main PLL (PLL0) */
    if ((LPC_SYSCTL->PLL[0].PLLSTAT & SYSCTL_PLLSTS_ENABLED) != 0) {
        /* PLLCON bits 31:1 - only zero should be written */
        LPC_SYSCTL->PLL[0].PLLCON = 0;
        LPC_SYSCTL->PLL[0].PLLFEED = 0xAA;
        LPC_SYSCTL->PLL[0].PLLFEED = 0x55;
    }

    /* switch the PLL Source to main oscillator */
    LPC_SYSCTL->CLKSRCSEL = SYSCTL_PLLCLKSRC_MAINOSC;

    /* setup Main PLL (PLL0)
     * MSEL = 9 (bits 4:0)
     * PSEL = 0 (bits 6:5)
     * FCCO = 12 MHz * (9+1) * 2 * (0+1) = 240 MHz
     * Fout = FCCO / ((0+1) * 2) = 120 MHz
     */
    LPC_SYSCTL->PLL[0].PLLCFG = (0 << 5) | (9) ;

    /* enable Main PLL */
    LPC_SYSCTL->PLL[0].PLLCON = SYSCTL_PLL_ENABLE;
    LPC_SYSCTL->PLL[0].PLLFEED = 0xAA;
    LPC_SYSCTL->PLL[0].PLLFEED = 0x55;

    /* set CPU clock divider to 1
     * Sysclk is still selected - the 8th bit is 0
     */
    LPC_SYSCTL->CCLKSEL = 1;

    /* wait for the PLL lock */
    while ((LPC_SYSCTL->PLL[0].PLLSTAT & SYSCTL_PLLSTS_LOCKED) == 0);

    /* select Main PLL */
    LPC_SYSCTL->CCLKSEL |= (1 << 8);

    /* adjust the flash accelerator
     * 100 - 120 MHz, power boost on
     */
    tmp = LPC_SYSCTL->FLASHCFG & 0xFFF;
    LPC_SYSCTL->FLASHCFG = 0x4000 | tmp;

    /* set peripheral clock to 60MHz */
    LPC_SYSCTL->PCLKSEL = 2;
}

static void iopin_init(void)
{
    /* enable IOCON, GPIO, and GPIO interrupts:
     * it's enabled after reset because the reset
     * value of the SYSCTL_CLOCK_GPIO bit is 1
    LPC_SYSCTL->PCONP |= (1 << SYSCTL_CLOCK_GPIO);
     */

    /* reset IOCON registers
     * not really needed after reset
    LPC_SYSCTL->RSTCON[1] = 1;
    LPC_SYSCTL->RSTCON[1] = 0;
     */

    /* set pin function */
    LPC_IOCON->p[1][2] = (IOCON_FUNC0 | IOCON_MODE_INACT);
    LPC_IOCON->p[1][3] = (IOCON_FUNC0 | IOCON_MODE_INACT);

    /* set direction of the pins as output */
    LPC_GPIO[1].DIR |= (1 << 2);
    LPC_GPIO[1].DIR |= (1 << 3);

    /* switch LEDs off */
    LPC_GPIO[1].SET = (1 << 2);
    LPC_GPIO[1].SET = (1 << 3);

}

