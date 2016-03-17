/*
 * vectors.c - Exception vectors for LPC4088 uC
 *
 */

#define ATTRIB_WEAK __attribute__((weak, alias("dummy_handler")))

void dummy_handler(void) __attribute__((interrupt,section(".precrp")));

void reset_handler(void)        ATTRIB_WEAK;
void nmi_handler(void)          ATTRIB_WEAK;
void hardfault_handler(void)    ATTRIB_WEAK;
void memmanage_handler(void)    ATTRIB_WEAK;
void busfault_handler(void)     ATTRIB_WEAK;
void usagefault_handler(void)   ATTRIB_WEAK;
void svc_handler(void)          ATTRIB_WEAK;
void debugmon_handler(void)     ATTRIB_WEAK;
void pendsv_handler(void)       ATTRIB_WEAK;
void systick_handler(void)      ATTRIB_WEAK;

void wdt_handler(void)          ATTRIB_WEAK;
void timer0_handler(void)       ATTRIB_WEAK;
void timer1_handler(void)       ATTRIB_WEAK;
void timer2_handler(void)       ATTRIB_WEAK;
void timer3_handler(void)       ATTRIB_WEAK;
void uart0_handler(void)        ATTRIB_WEAK;
void uart1_handler(void)        ATTRIB_WEAK;
void uart2_handler(void)        ATTRIB_WEAK;
void uart3_handler(void)        ATTRIB_WEAK;
void pwm1_handler(void)         ATTRIB_WEAK;
void i2c0_handler(void)         ATTRIB_WEAK;
void i2c1_handler(void)         ATTRIB_WEAK;
void i2c2_handler(void)         ATTRIB_WEAK;
void ssp0_handler(void)         ATTRIB_WEAK;
void ssp1_handler(void)         ATTRIB_WEAK;
void pll0_handler(void)         ATTRIB_WEAK;
void rtc_handler(void)          ATTRIB_WEAK;
void eint0_handler(void)        ATTRIB_WEAK;
void eint1_handler(void)        ATTRIB_WEAK;
void eint2_handler(void)        ATTRIB_WEAK;
void eint3_handler(void)        ATTRIB_WEAK;
void adc_handler(void)          ATTRIB_WEAK;
void bod_handler(void)          ATTRIB_WEAK;
void usb_handler(void)          ATTRIB_WEAK;
void can_handler(void)          ATTRIB_WEAK;
void dma_handler(void)          ATTRIB_WEAK;
void i2s_handler(void)          ATTRIB_WEAK;
void eth_handler(void)          ATTRIB_WEAK;
void sdio_handler(void)         ATTRIB_WEAK;
void mcpwm_handler(void)        ATTRIB_WEAK;
void qei_handler(void)          ATTRIB_WEAK;
void pll1_handler(void)         ATTRIB_WEAK;
void usbactivity_handler(void)  ATTRIB_WEAK;
void canactivity_handler(void)  ATTRIB_WEAK;
void uart4_handler(void)        ATTRIB_WEAK;
void ssp2_handler(void)         ATTRIB_WEAK;
void lcd_handler(void)          ATTRIB_WEAK;
void gpio_handler(void)         ATTRIB_WEAK;
void pwm0_handler(void)         ATTRIB_WEAK;
void eeprom_handler(void)       ATTRIB_WEAK;

#define LPC4088_IRQCNT (15 + 41)

typedef void (*phandler_t)(void);
typedef struct {
    void        *stack_top;
    phandler_t  handlers[LPC4088_IRQCNT];
} vectab_t;

extern unsigned int __stack_top;

vectab_t vectors __attribute__ ((section(".vectors"))) = {
    &__stack_top,
    {
        reset_handler,          /* EXC  1 */
        nmi_handler,            /* EXC  2 */
        hardfault_handler,      /* EXC  3 */
        memmanage_handler,      /* EXC  4 */
        busfault_handler,       /* EXC  5 */
        usagefault_handler,     /* EXC  6 */
        0,                      /* EXC  7 */
        0,                      /* EXC  8 */
        0,                      /* EXC  9 */
        0,                      /* EXC 10 */
        svc_handler,            /* EXC 11 */
        debugmon_handler,       /* EXC 12 */
        0,                      /* EXC 13 */
        pendsv_handler,         /* EXC 14 */
        systick_handler,        /* EXC 15 */

        wdt_handler,            /* IRQ  0 */
        timer0_handler,         /* IRQ  1 */
        timer1_handler,         /* IRQ  2 */
        timer2_handler,         /* IRQ  3 */
        timer3_handler,         /* IRQ  4 */
        uart0_handler,          /* IRQ  5 */
        uart1_handler,          /* IRQ  6 */
        uart2_handler,          /* IRQ  7 */
        uart3_handler,          /* IRQ  8 */
        pwm1_handler,           /* IRQ  9 */
        i2c0_handler,           /* IRQ 10 */
        i2c1_handler,           /* IRQ 11 */
        i2c2_handler,           /* IRQ 12 */
        0,                      /* IRQ 13 */
        ssp0_handler,           /* IRQ 14 */
        ssp1_handler,           /* IRQ 15 */
        pll0_handler,           /* IRQ 16 */
        rtc_handler,            /* IRQ 17 */
        eint0_handler,          /* IRQ 18 */
        eint1_handler,          /* IRQ 19 */
        eint2_handler,          /* IRQ 20 */
        eint3_handler,          /* IRQ 21 */
        adc_handler,            /* IRQ 22 */
        bod_handler,            /* IRQ 23 */
        usb_handler,            /* IRQ 24 */
        can_handler,            /* IRQ 25 */
        dma_handler,            /* IRQ 26 */
        i2s_handler,            /* IRQ 27 */
        eth_handler,            /* IRQ 28 */
        sdio_handler,           /* IRQ 29 */
        mcpwm_handler,          /* IRQ 30 */
        qei_handler,            /* IRQ 31 */
        pll1_handler,           /* IRQ 32 */
        usbactivity_handler,    /* IRQ 33 */
        canactivity_handler,    /* IRQ 34 */
        uart4_handler,          /* IRQ 35 */
        ssp2_handler,           /* IRQ 36 */
        lcd_handler,            /* IRQ 37 */
        gpio_handler,           /* IRQ 38 */
        pwm0_handler,           /* IRQ 39 */
        eeprom_handler          /* IRQ 40 */
    }
};


void dummy_handler(void)
{
    while (1);
}
