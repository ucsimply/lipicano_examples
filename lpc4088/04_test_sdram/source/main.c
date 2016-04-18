/*
 * main.c
 *
 */

#include "chip.h"
#include "uart.h"
#include "xprintf.h"
#include "systick.h"
#include "sdram.h"

#define SDRAM_START     EMC_ADDRESS_DYCS0
#define SDRAM_SZB       (32*1024*1024)
#define SDRAM_SZW       (8*1024*1024)
#define TEST_ROUNDS     1000

#define INIT_PERIOD     50  /* 50 * 10 ms = 0.5 sec */

void systick_func(void);
static void timer_start(void);

static void sdram_fill_w(uint32_t *pstart, uint32_t size, uint32_t value);
static void sdram_fill_b(uint8_t *pstart, uint32_t size, uint8_t value);
static void sdram_fill_addr(uint32_t *pstart, uint32_t size);
static void sdram_test_w(uint32_t *pstart, uint32_t size, uint32_t value);
static void sdram_test_b(uint8_t *pstart, uint32_t size, uint8_t value);
static void sdram_test_addr(uint32_t *pstart, uint32_t size);
static void sdram_test_walk1s(uint32_t *pstart, uint32_t size);
static void sdram_dump(uint32_t *pstart, uint32_t size);

static uint32_t     err_total;
volatile static int flag_period_pass;
volatile static int period_setter;
volatile static int period_counter;


int main (void)
{
    int led;
    int round;

    /* initialize xprintf xfunc_out() function pointer */
    xfunc_out = uart_writec;
    xprintf("Testing SDRAM ...\n");

    round = 1;
    err_total = 0;
    while (round <= TEST_ROUNDS) {
        sdram_fill_w((uint32_t *) SDRAM_START, SDRAM_SZW, 0);
        sdram_test_w((uint32_t *) SDRAM_START, SDRAM_SZW, 0);
        sdram_fill_b((uint8_t *) SDRAM_START, SDRAM_SZB, 0);
        sdram_test_b((uint8_t *) SDRAM_START, SDRAM_SZB, 0);
        // sdram_dump((uint32_t *) (SDRAM_START + 0x100), 64);

        sdram_fill_w((uint32_t *) SDRAM_START, SDRAM_SZW, 0xAAAAAAAA);
        sdram_test_w((uint32_t *) SDRAM_START, SDRAM_SZW, 0xAAAAAAAA);
        sdram_fill_b((uint8_t *) SDRAM_START, SDRAM_SZB, 0x55);
        sdram_test_b((uint8_t *) SDRAM_START, SDRAM_SZB, 0x55);
        // sdram_dump((uint32_t *) (SDRAM_START + 0x100), 64);

        sdram_fill_w((uint32_t *) SDRAM_START, SDRAM_SZW, 0x55555555);
        sdram_test_w((uint32_t *) SDRAM_START, SDRAM_SZW, 0x55555555);
        sdram_fill_b((uint8_t *) SDRAM_START, SDRAM_SZB, 0xAA);
        sdram_test_b((uint8_t *) SDRAM_START, SDRAM_SZB, 0xAA);
        // sdram_dump((uint32_t *) (SDRAM_START + 0x100), 64);

        sdram_test_walk1s((uint32_t *) (SDRAM_START), 1024);
        // sdram_dump((uint32_t *) SDRAM_START, 64);

        sdram_fill_addr((uint32_t *) SDRAM_START, SDRAM_SZW);
        sdram_test_addr((uint32_t *) SDRAM_START, SDRAM_SZW);
        // sdram_dump((uint32_t *) (SDRAM_START + 0x100), 64);

        xprintf("Finished round=%d, err_total=%d ***********************\n\n",
                round, err_total);
        round++;
    }

    xprintf("\n\nBlinking forever ...\n");
    timer_start();
    led = 0;
    while (1) {
        if (flag_period_pass) {
            /* toggle LEDs */
            if (led) {
                LPC_GPIO[1].SET = (1 << 2);
                LPC_GPIO[1].CLR = (1 << 3);
            }
            else {
                LPC_GPIO[1].CLR = (1 << 2);
                LPC_GPIO[1].SET = (1 << 3);
            }
            led = ~led;
            flag_period_pass = 0;
        }
    }

    /* main() should never reach this point */
    return 0;
}

void sdram_fill_w(uint32_t *pstart, uint32_t size, uint32_t value)
{
    xprintf("Fill SDRAM-word from=0x%08X, size=%d, val=0x%08X ... ",
            pstart, size, value);
    while (size--) {
        *pstart++ = value;
    }
    xprintf("Done.\n");
}

void sdram_fill_b(uint8_t *pstart, uint32_t size, uint8_t value)
{
    xprintf("Fill SDRAM-byte from=0x%08X, size=%d, val=0x%02X ... ",
            pstart, size, value);
    while (size--) {
        *pstart++ = value;
    }
    xprintf("Done.\n");
}

void sdram_fill_addr(uint32_t *pstart, uint32_t size)
{
    xprintf("Fill SDRAM-addr from=0x%08X, size=%d ... ",
            pstart, size);
    while (size--) {
        *pstart = (uint32_t) pstart;
        pstart++;
    }
    xprintf("Done.\n");
}

void sdram_test_w(uint32_t *pstart, uint32_t size, uint32_t value)
{
    uint32_t    mem_val;
    uint32_t    err_cnt;

    xprintf("Test SDRAM-word from=0x%08X, size=%d, val=0x%08X ...",
            pstart, size, value);
    err_cnt = 0;
    while (size--) {
        mem_val = *pstart;
        if (mem_val != value) {
            err_cnt++;
            xprintf("\nError: at=0x%08X, mem=0x%08X",
                    pstart, mem_val);
        }
        pstart++;
    }
    err_total += err_cnt;
    xprintf("%cDone, err=%d, err_total=%d\n",
            err_cnt ? '\n' : ' ', err_cnt, err_total);
}

void sdram_test_b(uint8_t *pstart, uint32_t size, uint8_t value)
{
    uint8_t    mem_val;
    uint32_t   err_cnt;

    xprintf("Test SDRAM-byte from=0x%08X, size=%d, val=0x%02X ...",
            pstart, size, value);
    err_cnt = 0;
    while (size--) {
        mem_val = *pstart;
        if (mem_val != value) {
            err_cnt++;
            xprintf("\nError: at=0x%08X, mem=0x%02X",
                    pstart, mem_val);
        }
        pstart++;
    }
    err_total += err_cnt;
    xprintf("%cDone, err=%d, err_total=%d\n",
            err_cnt ? '\n' : ' ', err_cnt, err_total);
}

void sdram_test_addr(uint32_t *pstart, uint32_t size)
{
    uint32_t   mem_val;
    uint32_t   err_cnt;

    xprintf("Test SDRAM-addr from=0x%08X, size=%d ...",
            pstart, size);
    err_cnt = 0;
    while (size--) {
        mem_val = *pstart;
        if (mem_val != (uint32_t) pstart) {
            err_cnt++;
            xprintf("\nError: at=0x%08X, mem=0x%08X",
                    pstart, mem_val);
        }
        pstart++;
    }
    err_total += err_cnt;
    xprintf("%cDone, err=%d, err_total=%d\n",
            err_cnt ? '\n' : ' ', err_cnt, err_total);
}

void sdram_test_walk1s(uint32_t *pstart, uint32_t size)
{
    uint32_t    mem_val;
    uint32_t    mem_read;
    uint32_t    err_cnt;
    uint32_t    i;
    int         bit_cnt;
    uint32_t    *psdr;

    xprintf("Test SDRAM-walk-ones from=0x%08X, size=%d ...",
            pstart, size);
    /* fill SDRAM */
    mem_val = 1;
    bit_cnt = 0;
    psdr = pstart;
    for (i = 0; i < size; i++) {
        *psdr++ = mem_val;
        mem_val <<= 1;
        bit_cnt++;
        if (bit_cnt == 32) {
            mem_val = 1;
            bit_cnt = 0;
        }
    }

    /* test SDRAM */
    err_cnt = 0;
    mem_val = 1;
    bit_cnt = 0;
    psdr = pstart;
    for (i = 0; i < size; i++) {
        mem_read = *psdr;
        if (mem_read != mem_val) {
            err_cnt++;
            xprintf("\nError: at=0x%08X, mem_read=0x%08X, mem_val=0x%08X",
                    psdr, mem_read, mem_val);
        }
        psdr++;
        mem_val <<= 1;
        bit_cnt++;
        if (bit_cnt == 32) {
            mem_val = 1;
            bit_cnt = 0;
        }
    }
    err_total += err_cnt;
    xprintf("%cDone, err=%d, err_total=%d\n",
            err_cnt ? '\n' : ' ', err_cnt, err_total);
}

void sdram_dump(uint32_t *pstart, uint32_t size)
{
    int counter;
    xprintf("Dump SDRAM: -------------------\n");

    counter = 8;
    xprintf("%08X:", pstart);
    while (size--) {
        xprintf("  %08X", *pstart);
        pstart++;
        counter--;
        if (!counter) {
            counter = 8;
            xprintf("\n%08X:", pstart);
        }
    }
    xprintf("\nDone -----------------\n");
}

void timer_start(void)
{
    flag_period_pass = 0;
    period_setter = INIT_PERIOD;
    period_counter = 0;

    /* init systick timer */
    systick_init();
    /* enable systick interrupt */
    systick_ienb(SYSTICK_ENABLE);
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
