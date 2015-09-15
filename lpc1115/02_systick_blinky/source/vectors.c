/*
 * vectors.c - Exception vectors for LPC1115 uC
 *
 */

#define ATTRIB_WEAK __attribute__((weak, alias("dummy_handler")))

void dummy_handler(void) __attribute__((interrupt,section(".precrp")));

void reset_handler(void)     ATTRIB_WEAK;
void nmi_handler(void)       ATTRIB_WEAK;
void hardfault_handler(void) ATTRIB_WEAK;
void svc_handler(void)       ATTRIB_WEAK;
void pendsv_handler(void)    ATTRIB_WEAK;
void systick_handler(void)   ATTRIB_WEAK;

void wakeup_handler(void)    ATTRIB_WEAK;
void can_handler(void)       ATTRIB_WEAK;
void i2c_handler(void)       ATTRIB_WEAK;
void ssp1_handler(void)      ATTRIB_WEAK;
void ct16b0_handler(void)    ATTRIB_WEAK;
void ct16b1_handler(void)    ATTRIB_WEAK;
void ct32b0_handler(void)    ATTRIB_WEAK;
void ct32b1_handler(void)    ATTRIB_WEAK;
void ssp0_handler(void)      ATTRIB_WEAK;
void uart_handler(void)      ATTRIB_WEAK;
void adc_handler(void)       ATTRIB_WEAK;
void wdt_handler(void)       ATTRIB_WEAK;
void bod_handler(void)       ATTRIB_WEAK;
void pio0_handler(void)      ATTRIB_WEAK;
void pio1_handler(void)      ATTRIB_WEAK;
void pio2_handler(void)      ATTRIB_WEAK;
void pio3_handler(void)      ATTRIB_WEAK;

#define CM0_IRQCNT (15 + 32)

typedef void (*phandler_t)(void);
typedef struct {
    void        *stack_top;
    phandler_t  handlers[CM0_IRQCNT];
} vectab_t;

extern unsigned int __stack_top;

vectab_t vectors __attribute__ ((section(".vectors"))) = {
    &__stack_top,
    {
        reset_handler,          /* EXC  1 */
        nmi_handler,            /* EXC  2 */
        hardfault_handler,      /* EXC  3 */
        0,                      /* EXC  4 */
        0,                      /* EXC  5 */
        0,                      /* EXC  6 */
        0,                      /* EXC  7 */
        0,                      /* EXC  8 */
        0,                      /* EXC  9 */
        0,                      /* EXC 10 */
        svc_handler,            /* EXC 11 */
        0,                      /* EXC 12 */
        0,                      /* EXC 13 */
        pendsv_handler,         /* EXC 14 */
        systick_handler,        /* EXC 15 */

        wakeup_handler,         /* IRQ  0 */
        wakeup_handler,         /* IRQ  1 */
        wakeup_handler,         /* IRQ  2 */
        wakeup_handler,         /* IRQ  3 */
        wakeup_handler,         /* IRQ  4 */
        wakeup_handler,         /* IRQ  5 */
        wakeup_handler,         /* IRQ  6 */
        wakeup_handler,         /* IRQ  7 */
        wakeup_handler,         /* IRQ  8 */
        wakeup_handler,         /* IRQ  9 */
        wakeup_handler,         /* IRQ 10 */
        wakeup_handler,         /* IRQ 11 */
        wakeup_handler,         /* IRQ 12 */
        can_handler,            /* IRQ 13 */
        ssp1_handler,           /* IRQ 14 */
        i2c_handler,            /* IRQ 15 */
        ct16b0_handler,         /* IRQ 16 */
        ct16b1_handler,         /* IRQ 17 */
        ct32b0_handler,         /* IRQ 18 */
        ct32b1_handler,         /* IRQ 19 */
        ssp0_handler,           /* IRQ 20 */
        uart_handler,           /* IRQ 21 */
        0,                      /* IRQ 22 */
        0,                      /* IRQ 23 */
        adc_handler,            /* IRQ 24 */
        wdt_handler,            /* IRQ 25 */
        bod_handler,            /* IRQ 26 */
        0,                      /* IRQ 27 */
        pio3_handler,           /* IRQ 28 */
        pio2_handler,           /* IRQ 29 */
        pio1_handler,           /* IRQ 30 */
        pio0_handler            /* IRQ 31 */
    }
};

void dummy_handler(void)
{
    while (1);
}
