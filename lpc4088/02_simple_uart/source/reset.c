/*
 * reset.c
 *
 */

/* values are defined in the linker script */
extern unsigned int __data_load;
extern unsigned int __data_begin;
extern unsigned int __data_end;
extern unsigned int __bss_begin;
extern unsigned int __bss_end;

extern void main(void);
extern void lowinit(void);

void reset_handler(void) __attribute__((interrupt,section(".precrp")));

void reset_handler(void)
{
    unsigned int *src;
    unsigned int *trg;

    /* copy initialized data from FLASH to RAM */
    src = &__data_load;
    trg = &__data_begin;
    while (trg < &__data_end) {
        *trg++ = *src++;
    }

    /* clear unitialized data */
    trg = &__bss_begin;
    while (trg < &__bss_end) {
        *trg++ = 0;
    }

    /* call low level hardware init */
    lowinit();

    /* call main() */
    main();

    /* main should never return */
    while (1);
}

