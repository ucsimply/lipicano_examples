/*
 * eth.c
 * low level ethernet control
 */

#include <stdio.h>
#include <string.h>
#include "chip.h"
#include "eth.h"

/* locally administered MAC address */
#define MACADDR_12  0x0210
#define MACADDR_34  0x2030
#define MACADDR_56  0x4050


/* use peripheral SRAM for descriptors, status arrays and buffers */
#define PRPH_SRAM_BASE  (0x20000000UL)

#define TX_DESC_CNT 4
#define RX_DESC_CNT 4

#define RX_DESC_BASE    PRPH_SRAM_BASE
#define RX_STAT_BASE    (RX_DESC_BASE + RX_DESC_CNT * sizeof(ENET_RXDESC_T))
#define TX_DESC_BASE    (RX_STAT_BASE + RX_DESC_CNT * sizeof(ENET_RXSTAT_T))
#define TX_STAT_BASE    (TX_DESC_BASE + TX_DESC_CNT * sizeof(ENET_TXDESC_T))
#define RX_BUF_BASE     (TX_STAT_BASE + TX_DESC_CNT * sizeof(ENET_TXSTAT_T))
#define TX_BUF_BASE     (RX_BUF_BASE  + RX_DESC_CNT * ENET_ETH_MAX_FLEN)

#define RX_BUF(i)       (RX_BUF_BASE + ENET_ETH_MAX_FLEN * i)
#define TX_BUF(i)       (TX_BUF_BASE + ENET_ETH_MAX_FLEN * i)

static ENET_RXDESC_T *prx_descs = (ENET_RXDESC_T *) RX_DESC_BASE;
static ENET_RXSTAT_T *prx_stats = (ENET_RXSTAT_T *) RX_STAT_BASE;
static ENET_TXDESC_T *ptx_descs = (ENET_TXDESC_T *) TX_DESC_BASE;
static ENET_TXSTAT_T *ptx_stats = (ENET_TXSTAT_T *) TX_STAT_BASE;

/* TX/RX indices */
static int32_t rx_cons_idx;
static int32_t tx_prod_idx;

static void pinmux_init(void);
static void ethmac_init(void);
static void phy_init(void);
static int phy_read(int reg);
static void phy_write(int reg, int value);
static void macaddr_set(void);
static void descripts_init(void);
static void mem_clear(void *addr, size_t size);

void eth_init(void)
{
    pinmux_init();
    ethmac_init();
    phy_init();
    macaddr_set();
    descripts_init();

    /* enable RX/TX */
    LPC_ETHERNET->CONTROL.COMMAND |= ENET_COMMAND_TXENABLE;
    LPC_ETHERNET->CONTROL.COMMAND |= ENET_COMMAND_RXENABLE;
    LPC_ETHERNET->MAC.MAC1 |= ENET_MAC1_RXENABLE;

}

int eth_packet_get(void **pbuff)
{
    uint32_t    prod_idx;
    int         size;

    prod_idx = LPC_ETHERNET->CONTROL.RX.PRODUCEINDEX;
    if (rx_cons_idx != prod_idx) {
        *pbuff = (void *) prx_descs[rx_cons_idx].Packet;
        size = (int32_t) (ENET_RINFO_SIZE(prx_stats[rx_cons_idx].StatusInfo));
    }
    else {
        *pbuff = NULL;
        size = 0;
    }
    return size;
}

void *eth_txbuff_get(void)
{
    uint32_t cons_idx;

    cons_idx = LPC_ETHERNET->CONTROL.TX.CONSUMEINDEX;
    if ((cons_idx == 0) && (tx_prod_idx == (TX_DESC_CNT - 1))) {
        return NULL;
    }
    if (cons_idx == (tx_prod_idx + 1)) {
        return NULL;
    }
    return (void *) ptx_descs[tx_prod_idx].Packet;
}

void eth_packet_send(int size)
{
    if (size > 0) {
        ptx_descs[tx_prod_idx].Control = ENET_TCTRL_SIZE(size) | ENET_TCTRL_LAST;
        /* increment TX produce index */
        tx_prod_idx++;
        if (tx_prod_idx > LPC_ETHERNET->CONTROL.TX.DESCRIPTORNUMBER) {
            tx_prod_idx = 0;
        }
        LPC_ETHERNET->CONTROL.TX.PRODUCEINDEX = tx_prod_idx;
    }
}

void eth_inc_rxidx(void)
{
    rx_cons_idx++;
    if (rx_cons_idx > LPC_ETHERNET->CONTROL.RX.DESCRIPTORNUMBER) {
        rx_cons_idx = 0;
    }
    LPC_ETHERNET->CONTROL.RX.CONSUMEINDEX = rx_cons_idx;
}

void pinmux_init(void)
{
    /* IOCON must be enabled - already done in lowinit.c:iopin_init() */
    LPC_IOCON->p[1][0]  = (IOCON_FUNC1 | IOCON_MODE_INACT); /* ENET_TXD0 */
    LPC_IOCON->p[1][1]  = (IOCON_FUNC1 | IOCON_MODE_INACT); /* ENET_TXD1 */
    LPC_IOCON->p[1][4]  = (IOCON_FUNC1 | IOCON_MODE_INACT); /* ENET_TX_EN */
    LPC_IOCON->p[1][8]  = (IOCON_FUNC1 | IOCON_MODE_INACT); /* ENET_CRS_DV */
    LPC_IOCON->p[1][9]  = (IOCON_FUNC1 | IOCON_MODE_INACT); /* ENET_RXD0 */
    LPC_IOCON->p[1][10] = (IOCON_FUNC1 | IOCON_MODE_INACT); /* ENET_RXD1 */
    LPC_IOCON->p[1][14] = (IOCON_FUNC1 | IOCON_MODE_INACT | /* ENET_RX_ERR */
                            IOCON_DIGMODE_EN);
    LPC_IOCON->p[1][15] = (IOCON_FUNC1 | IOCON_MODE_INACT); /* ENET_RX_CLK */
    LPC_IOCON->p[1][16] = (IOCON_FUNC1 | IOCON_MODE_INACT | /* ENET_MDC */
                            IOCON_DIGMODE_EN);
    LPC_IOCON->p[1][17] = (IOCON_FUNC1 | IOCON_MODE_INACT | /* ENET_MDIO */
                            IOCON_DIGMODE_EN);
}

void ethmac_init(void)
{
    volatile int i;

    /* enable ENET */
    LPC_SYSCTL->PCONP |= (1 << SYSCTL_CLOCK_ENET);

    /* reset ENET */
    LPC_SYSCTL->RSTCON[0] = (1 << SYSCTL_RESET_ENET);
    LPC_SYSCTL->RSTCON[0] = 0;
    LPC_ETHERNET->MAC.MAC1 =
            ENET_MAC1_RESETTX | ENET_MAC1_RESETMCSTX | ENET_MAC1_RESETRX |
            ENET_MAC1_RESETMCSRX | ENET_MAC1_SIMRESET | ENET_MAC1_SOFTRESET;
    LPC_ETHERNET->CONTROL.COMMAND =
            ENET_COMMAND_REGRESET | ENET_COMMAND_TXRESET |
            ENET_COMMAND_RXRESET | ENET_COMMAND_PASSRUNTFRAME;
    /* wait a moment for MAC to be ready */
    for (i = 0; i < 100; i++) {
    }

    /* configure MAC:
     * - full duplex
     * - 100 Mbps
     * - inter-frame gap
     * - use default values
     */
    LPC_ETHERNET->MAC.MAC1 = ENET_MAC1_PARF;
    LPC_ETHERNET->MAC.MAC2 =
            ENET_MAC2_FULLDUPLEX | ENET_MAC2_CRCEN | ENET_MAC2_PADCRCEN;

    /* using RMII */
    LPC_ETHERNET->CONTROL.COMMAND =
            ENET_COMMAND_FULLDUPLEX | ENET_COMMAND_PASSRUNTFRAME |
            ENET_COMMAND_RMII;

    LPC_ETHERNET->MAC.IPGT = ENET_IPGT_FULLDUPLEX;
    LPC_ETHERNET->MAC.IPGR = ENET_IPGR_P2_DEF;
    LPC_ETHERNET->MAC.SUPP = ENET_SUPP_100Mbps_SPEED;
    LPC_ETHERNET->MAC.MAXF = ENET_ETH_MAX_FLEN;
    LPC_ETHERNET->MAC.CLRT = ENET_CLRT_DEF;

    /* Setup default filter */
    LPC_ETHERNET->CONTROL.COMMAND |= ENET_COMMAND_PASSRXFILTER;

    /* Clear all MAC interrupts */
    LPC_ETHERNET->MODULE_CONTROL.INTCLEAR = 0xFFFF;

    /* Disable MAC interrupts */
    LPC_ETHERNET->MODULE_CONTROL.INTENABLE = 0;

    /*  MII config register
     *  - set clock divider = 120 / 2.5 [MHz] = 48
     *  - reset
     */
    LPC_ETHERNET->MAC.MCFG = ENET_MCFG_CLOCKSEL(48) | ENET_MCFG_RES_MII;
    LPC_ETHERNET->MAC.MCFG &= ~(ENET_MCFG_RES_MII);

}

void phy_init(void)
{
    /* using default hw config */
    phy_write(0, 0);
    phy_read(0);
}

int phy_read(int reg)
{
    return 0;
}

void phy_write(int reg, int value)
{
}

static void macaddr_set(void)
{
    LPC_ETHERNET->MAC.SA[0] = MACADDR_12;
    LPC_ETHERNET->MAC.SA[1] = MACADDR_34;
    LPC_ETHERNET->MAC.SA[2] = MACADDR_56;
}

void descripts_init(void)
{
    int i;

    /* clear descriptor and status arrays */
    mem_clear(prx_descs, RX_DESC_CNT * sizeof(ENET_RXDESC_T));
    mem_clear(prx_stats, RX_DESC_CNT * sizeof(ENET_RXSTAT_T));
    mem_clear(ptx_descs, TX_DESC_CNT * sizeof(ENET_TXDESC_T));
    mem_clear(ptx_stats, TX_DESC_CNT * sizeof(ENET_TXSTAT_T));

    rx_cons_idx = 0;
    tx_prod_idx = 0;

    /* initialize lists */
    for (i = 0; i < RX_DESC_CNT; i++) {
        prx_descs[i].Packet = (uint32_t) RX_BUF(i);
        prx_descs[i].Control = ENET_RCTRL_SIZE(ENET_ETH_MAX_FLEN);
        prx_stats[i].StatusInfo = 0;
        prx_stats[i].StatusHashCRC = 0;
    }
    for (i = 0; i < TX_DESC_CNT; i++) {
        ptx_descs[i].Packet = (uint32_t) TX_BUF(i);
        ptx_descs[i].Control = 0;
        ptx_stats[i].StatusInfo = 0;
    }

    /* set RX lists addresses */
    LPC_ETHERNET->CONTROL.RX.DESCRIPTOR = (uint32_t) prx_descs;
    LPC_ETHERNET->CONTROL.RX.DESCRIPTORNUMBER = RX_DESC_CNT - 1;
    LPC_ETHERNET->CONTROL.RX.STATUS = (uint32_t) prx_stats;
    LPC_ETHERNET->CONTROL.RX.CONSUMEINDEX = 0;

    /* set TX lists addresses */
    LPC_ETHERNET->CONTROL.TX.DESCRIPTOR = (uint32_t) ptx_descs;
    LPC_ETHERNET->CONTROL.TX.DESCRIPTORNUMBER = TX_DESC_CNT - 1;
    LPC_ETHERNET->CONTROL.TX.STATUS = (uint32_t) ptx_stats;
    LPC_ETHERNET->CONTROL.TX.PRODUCEINDEX = 0;
}

static void mem_clear(void *addr, size_t size)
{
    uint8_t *pdata = (uint8_t *) addr;
    while (size--) {
        *pdata++ = 0;
    }
}
