/*
 * main.c
 *
 */

#include "chip.h"
#include "uart.h"
#include "xprintf.h"
#include "systick.h"
#include "spifi.h"

#define COLUMN_COUNT    16

static char hro[COLUMN_COUNT + 1];

static void mem_dump(uint8_t *pstart, uint32_t size);

int main (void)
{
    /* initialize the output string */
    hro[COLUMN_COUNT + 1] = '\0';

    /* initialize xprintf xfunc_out() function pointer */
    xfunc_out = uart_writec;
    xprintf("Testing SPIFI ...\n");

    spifi_init();
    mem_dump((uint8_t *) (SPIFLASH_BASE_ADDRESS + 0 * 256), 256);
    mem_dump((uint8_t *) (SPIFLASH_BASE_ADDRESS + 1 * 256), 256);
    mem_dump((uint8_t *) (SPIFLASH_BASE_ADDRESS + 2 * 256), 256);
    mem_dump((uint8_t *) (SPIFLASH_BASE_ADDRESS + 3 * 256), 256);

    while (1) {
    }

    /* main() should never reach this point */
    return 0;
}

void mem_dump(uint8_t *pstart, uint32_t size)
{
    int     counter;
    uint8_t ch;

    xprintf("\nDump from 0x%08X --------------------------\n", pstart);

    counter = COLUMN_COUNT;
    xprintf("%08X:", pstart);

    while (size) {
        ch = *pstart;
        xprintf(" %02X", ch);
        if (ch < 0x21 || ch > 0x7E) {
            ch = '.';
        }
        hro[COLUMN_COUNT - counter] = ch;
        pstart++;
        counter--;
        size--;
        if ((size * counter) == 0) {
            xprintf("  %s", hro);
            if (size == 0) {
                xprintf("\n");
            }
            else {
                counter = COLUMN_COUNT;
                xprintf("\n%08X:", pstart);
            }
        }
    }
    xprintf("Done -------------------------------------\n");
}

