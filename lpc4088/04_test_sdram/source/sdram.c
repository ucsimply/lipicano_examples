/*
 * sdram.c
 * SDRAM initialization
 */

#include <stdio.h>
#include <string.h>
#include "chip.h"
#include "sdram.h"


/* CPU clock = 120 MHz, EMC clock = 1/2 CPU clock = 60 MHz */
#define EMC_FREQ_HZ         60000000
#define EMC_CLK_NS          16.67

#define CLK_DELAY           7

#define SDR_CMD_NOP         0x00000183
#define SDR_CMD_PALL        0x00000103
#define SDR_CMD_MODE        0x00000083
#define SDR_CMD_NORMAL      0x00000000

/* settings for AS4C16M16S-7BCN device
 * - 256Mbit, 16M x 16bit, 4 Banks x 4M words x 16bit
 * - 13 bit row address A[0..12] = 8192 rows
 * - 9 bit column address A[0..8] = 512 columns
 */
#define SDR_ROWS            8192
#define SDR_RAS_CLK         2
#define SDR_CAS_CLK         2
#define SDR_BURST_LEN_8     3
#define SDR_MODEREG         ((SDR_CAS_CLK << 4) | SDR_BURST_LEN_8)
#define SDR_ADDR_MODEREG    (EMC_ADDRESS_DYCS0 | (SDR_MODEREG << (9 + 2 + 1)))

#define SDR_REFRESH_NS  (64000000 / SDR_ROWS) // 7812.5
#define SDR_REFRESH     30                    // (7813 / 16.67) / 16
#define SDR_READ_CFG    1
#define SDR_RAS_CAS     ((SDR_CAS_CLK << 8) | SDR_RAS_CLK )
#define SDR_CONFIG      (EMC_DYN_CONFIG_DATA_BUS_16 | \
                        EMC_DYN_CONFIG_16Mx16_4BANKS_13ROWS_9COLS | \
                        EMC_DYN_CONFIG_MD_SDRAM)

/* ****************** */
/* 1 cycle = 16.67 ns */
/* ****************** */
/* Precharge Command Period                             21 ns */
#define SDR_tRP         1
/* Active to Precharge Command Period                   49 ns */
#define SDR_tRAS        2
/* Self Refresh Exit Time (use tXSR if not specified) */
#define SDR_tSREX       3
/* Last Data Out to Active Time                         1 CLK */
#define SDR_tAPR        1
/* Data In to Active Command Time                       CAS_CLK + tRP */
#define SDR_tDAL        3
/* Write Recovery Time                                  14 ns */
#define SDR_tWR         0
/* Active to Active Command Period                      63 ns */
#define SDR_tRC         3
/* Auto-refresh Period                                  63 ns */
#define SDR_tRFC        3
/* Exit Self Refresh  (tRC + tIS)                       63 + 1.5 ns */
#define SDR_tXSR        3
/* Active Bank A to Active Bank B Time                  14 ns */
#define SDR_tRRD        0
/* Load Mode register command to Active Command         14 ns*/
#define SDR_tMRD        0



static void pinmux_init(void);
static void emc_init(void);
static void emc_config(void);

void sdram_init(void)
{
    pinmux_init();
    emc_init();
    emc_config();
}


void pinmux_init(void)
{
    int i;

    /* set P4_0-14 as EMC_A[0:14] */
    for (i = 0; i < 15; i++) {
        LPC_IOCON->p[4][i] = (IOCON_FUNC1 | IOCON_FASTSLEW_EN);
    }
    /* set P3_0-15 as EMC_D[0:15] */
    for (i = 0; i < 16; i++) {
        LPC_IOCON->p[3][i] = (IOCON_FUNC1 | IOCON_FASTSLEW_EN);
    }
    /* set EMC SDRAM control signals */
    LPC_IOCON->p[2][16] = (IOCON_FUNC1 | IOCON_FASTSLEW_EN);    /* SDR_CAS */
    LPC_IOCON->p[2][17] = (IOCON_FUNC1 | IOCON_FASTSLEW_EN);    /* SDR_RAS */
    LPC_IOCON->p[2][18] = (IOCON_FUNC1 | IOCON_FASTSLEW_EN);    /* SDR_CLK */
    LPC_IOCON->p[2][20] = (IOCON_FUNC1 | IOCON_FASTSLEW_EN);    /* SDR_CS */
    LPC_IOCON->p[2][24] = (IOCON_FUNC1 | IOCON_FASTSLEW_EN);    /* SDR_CKE */
    LPC_IOCON->p[2][28] = (IOCON_FUNC1 | IOCON_FASTSLEW_EN);    /* SDR_DQML */
    LPC_IOCON->p[2][29] = (IOCON_FUNC1 | IOCON_FASTSLEW_EN);    /* SDR_DQMH */
    LPC_IOCON->p[4][25] = (IOCON_FUNC1 | IOCON_FASTSLEW_EN);    /* SDR_WE */

}

void emc_init(void)
{
    /* enable EMC */
    LPC_SYSCTL->PCONP |= (1 << SYSCTL_CLOCK_EMC);
    /* set EMC delays */
    LPC_SYSCTL->EMCDLYCTL = (CLK_DELAY) | (CLK_DELAY << 8) |
                            (CLK_DELAY << 16 | (CLK_DELAY << 24));
    /* set emc_clk = 1/2 cclk (i.e. 60 MHz */
    LPC_SYSCTL->EMCCLKSEL = 1;
    /* reset EMC */
    LPC_SYSCTL->RSTCON[0] = (1 << SYSCTL_RESET_EMC);
    LPC_SYSCTL->RSTCON[0] = 0;
    /* set little endian mode - default on reset */
    LPC_EMC->CONFIG = 0;
    /* enable EMC, normal memory map, normal mode */
    LPC_EMC->CONTROL = 1;
}

void emc_config(void)
{
    int i;
    volatile uint32_t tmp;

    LPC_EMC->DYNAMICRASCAS0    = SDR_RAS_CAS;
    LPC_EMC->DYNAMICCONFIG0    = SDR_CONFIG;
    LPC_EMC->DYNAMICREADCONFIG = SDR_READ_CFG;

    LPC_EMC->DYNAMICRP   = SDR_tRP;
    LPC_EMC->DYNAMICRAS  = SDR_tRAS;
    LPC_EMC->DYNAMICSREX = SDR_tSREX;
    LPC_EMC->DYNAMICAPR  = SDR_tAPR;
    LPC_EMC->DYNAMICDAL  = SDR_tDAL;
    LPC_EMC->DYNAMICWR   = SDR_tWR;
    LPC_EMC->DYNAMICRC   = SDR_tRC;
    LPC_EMC->DYNAMICRFC  = SDR_tRFC;
    LPC_EMC->DYNAMICXSR  = SDR_tXSR;
    LPC_EMC->DYNAMICRRD  = SDR_tRRD;
    LPC_EMC->DYNAMICMRD  = SDR_tMRD;

    /* wait 100us */
    for (i = 0; i < 1000; i++);

    /* NOP command */
    LPC_EMC->DYNAMICCONTROL = SDR_CMD_NOP;
    /* wait 200us */
    for (i = 0; i < 1000; i++);

    /* PALL command */
    LPC_EMC->DYNAMICCONTROL = SDR_CMD_PALL;

    /* ( 2 * 16 ) -> 32 clock cycles */
    LPC_EMC->DYNAMICREFRESH = 2;
    /* wait 200us */
    for (i = 0; i < 100; i++);

    LPC_EMC->DYNAMICREFRESH = SDR_REFRESH;

    /* MODE command */
    LPC_EMC->DYNAMICCONTROL = SDR_CMD_MODE;
    /* reading from the address sets SDRAM Mode register */
    tmp = *((volatile uint32_t *)(SDR_ADDR_MODEREG));
    tmp++;

    /* NORMAL command */
    LPC_EMC->DYNAMICCONTROL = SDR_CMD_NORMAL;

    /* enable buffer */
    LPC_EMC->DYNAMICCONFIG0 |= 1 << 19;

    /* wait 200us */
    for (i = 0; i < 1000; i++);
}

