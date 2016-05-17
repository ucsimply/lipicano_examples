/*
 * main.c
 *
 */

#include "chip.h"
#include "uart.h"
#include "eth.h"
#include "xprintf.h"
#include "systick.h"

#define INIT_PERIOD     100  /* 100 * 10 ms = 1 sec */

void systick_func(void);

static void timer_start(void);
static void packet_send(void);
static void packet_recv(void);

volatile static int  flag_period_pass;
volatile static int  period_setter;
volatile static int  period_counter;

int main (void)
{
    int led;

    /* initialize xprintf xfunc_out() function pointer */
    xfunc_out = uart_writec;

    eth_display_phy_regs();
    xprintf("\n%s\n", "Waiting for ethernet packets ...\n");

    timer_start();
    led = 0;

    while (1) {
        packet_recv();
        if (flag_period_pass) {
            /* toggle LEDs */
            if (led) {
                LPC_GPIO[1].SET = (1 << 2);
                LPC_GPIO[1].CLR = (1 << 3);
                packet_send();
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

void packet_recv(void)
{
    int size;
    unsigned char *pbuff;

    size = eth_packet_get((void **) &pbuff);
    if (size) {
        xprintf("Packet: size=%4d, type=%02x%02x, "
                "dst=%02x:%02x:%02x:%02x:%02x:%02x, "
                "src=%02x:%02x:%02x:%02x:%02x:%02x\n",
                size, pbuff[12], pbuff[13],
                pbuff[0], pbuff[1], pbuff[2], pbuff[3], pbuff[4], pbuff[5],
                pbuff[6], pbuff[7], pbuff[8], pbuff[9], pbuff[10], pbuff[11]);
        eth_inc_rxidx();
    }
}

void packet_send(void)
{
    uint8_t *pbuff;
    int i, pos;

    if ((pbuff = eth_txbuff_get()) != NULL) {
        /* broadcast destination */
        for (pos = 0; pos < 6; pos++) {
            pbuff[pos] = 0xFF;
        }
        /* our MAC address */
        pbuff[pos++] = 0x02;
        pbuff[pos++] = 0x10;
        pbuff[pos++] = 0x20;
        pbuff[pos++] = 0x30;
        pbuff[pos++] = 0x40;
        pbuff[pos++] = 0x50;
        /* length = 128 byte */
        pbuff[pos++] = 0;
        pbuff[pos++] = 128;
        /* some data */
        for (i = 0; i < 128; i++) {
            pbuff[pos++] = i + 1;
        }
        eth_packet_send(128 + 2 + 12);
    }
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
