/*
 * spifi.c
 * SPIFI test
 */

#include <stdio.h>
#include <string.h>
#include "chip.h"
#include "spifi.h"
#include "xprintf.h"

/*
 * SPIFI interface registers
 */
struct spifi_regs {
    uint32_t ctrl;
    uint32_t cmd;
    uint32_t addr;
    uint32_t idata;
    uint32_t climit;
    union {
        uint8_t  data8;
        uint16_t data16;
        uint32_t data32;
    };
    uint32_t mcmd;
    uint32_t stat;
};

#define SPIFI_REGS      ((volatile struct spifi_regs *) LPC_SPIFI_BASE)

/*
 * SPIFI registers bits
 */
#define SPIFI_CTRL_TIMEOUT          (100 << 0)
#define SPIFI_CTRL_CSHIGH           (10 << 16)
#define SPIFI_CTRL_MODE3            (1 << 23)
#define SPIFI_CTRL_RFCLK            (1 << 29)
#define SPIFI_CTRL_FBCLK            (1 << 30)

#define SPIFI_CMD_DATALEN_BIT       0
#define SPIFI_CMD_POLL              (1 << 14)
#define SPIFI_CMD_DOUT              (1 << 15)
#define SPIFI_CMD_INTLEN_1          (1 << 16)
#define SPIFI_CMD_INTLEN_2          (2 << 16)
#define SPIFI_CMD_INTLEN_3          (3 << 16)
#define SPIFI_CMD_INTLEN_4          (4 << 16)
#define SPIFI_CMD_FIELDF_SALL       (0 << 19)
#define SPIFI_CMD_FIELDF_QDTA       (1 << 19)
#define SPIFI_CMD_FIELDF_SOPC       (2 << 19)
#define SPIFI_CMD_FIELDF_QALL       (3 << 19)
#define SPIFI_CMD_FRAMEF_OPNA       (1 << 21)
#define SPIFI_CMD_FRAMEF_OP1A       (2 << 21)
#define SPIFI_CMD_FRAMEF_OP2A       (3 << 21)
#define SPIFI_CMD_FRAMEF_OP3A       (4 << 21)
#define SPIFI_CMD_FRAMEF_OP4A       (5 << 21)
#define SPIFI_CMD_FRAMEF_NOP3A      (6 << 21)
#define SPIFI_CMD_FRAMEF_NOP4A      (7 << 21)
#define SPIFI_CMD_OPCODE_BIT        24

#define SPIFI_STAT_MCINIT           (1 << 0)
#define SPIFI_STAT_CMD              (1 << 1)
#define SPIFI_STAT_RESET            (1 << 4)

/*
 * SPIFI Clock Selection register
 */
#define SPIFICLKSEL_SPIFIDIV        4
#define SPIFICLKSEL_SPIFISEL        (1 << 8)

/*
 * Winbond W25Q64FV OP codes
 */
#define OP_READ_JID                 0x9F
#define OP_WRITE_ENB                0x06
#define OP_READ_SREG1               0x05
#define OP_READ_SREG2               0x35
#define OP_WRITE_SREG               0x01
#define OP_READN                    0x03
#define OP_READF                    0x0B
#define OP_READQ                    0x6B

/*
 * Winbond W25Q64FV status regs
 */
#define SREG1_BUSY                  (1 << 0)
#define SREG1_WENBL                 (1 << 1)
#define SREG2_QENB                  (1 << 9)

static void iface_init(void);
static void chip_print_info(void);

void spifi_init(void)
{
    iface_init();
    spifi_switch_to_cmd();
    chip_print_info();
    spifi_switch_to_mem();
}

void spifi_switch_to_cmd(void)
{
    /* Reset the SPIFI interface */
    SPIFI_REGS->stat = SPIFI_STAT_RESET;
    while (SPIFI_REGS->stat & SPIFI_STAT_RESET);
}

void spifi_switch_to_mem(void)
{
    SPIFI_REGS->ctrl = SPIFI_CTRL_TIMEOUT | SPIFI_CTRL_CSHIGH;
    SPIFI_REGS->idata = 0xA5;
    SPIFI_REGS->mcmd =
        (OP_READQ << SPIFI_CMD_OPCODE_BIT) |
        SPIFI_CMD_FRAMEF_OP3A |
        SPIFI_CMD_FIELDF_QDTA |
        SPIFI_CMD_INTLEN_1;
}

void iface_init(void)
{
    /* set SPIFI pinmux */
    LPC_IOCON->p[0][15] = (IOCON_FUNC5 | IOCON_FASTSLEW_EN);    /* SPIFI_IO_2 */
    LPC_IOCON->p[0][16] = (IOCON_FUNC5 | IOCON_FASTSLEW_EN);    /* SPIFI_IO_3 */
    LPC_IOCON->p[0][17] = (IOCON_FUNC5 | IOCON_FASTSLEW_EN);    /* SPIFI_IO_1 */
    LPC_IOCON->p[0][18] = (IOCON_FUNC5 | IOCON_FASTSLEW_EN);    /* SPIFI_IO_0 */
    LPC_IOCON->p[0][22] = (IOCON_FUNC5 | IOCON_FASTSLEW_EN);    /* SPIFI_CLK  */
    LPC_IOCON->p[2][7]  = (IOCON_FUNC5 | IOCON_FASTSLEW_EN);    /* SPIFI_CS   */

    /* set SPIFI clock */
    LPC_SYSCTL->SPIFICLKSEL = SPIFICLKSEL_SPIFISEL | SPIFICLKSEL_SPIFIDIV;

    /* switch on SPIFI module */
    LPC_SYSCTL->PCONP |= (1 << SYSCTL_CLOCK_SPIFI);
}

void chip_print_info(void)
{
    uint8_t buff[3];

    SPIFI_REGS->ctrl = 0;
    SPIFI_REGS->cmd =
        (OP_READ_JID << SPIFI_CMD_OPCODE_BIT) |
        SPIFI_CMD_FRAMEF_OPNA | 3;
    buff[0] = SPIFI_REGS->data8;
    buff[1] = SPIFI_REGS->data8;
    buff[2] = SPIFI_REGS->data8;
    while (SPIFI_REGS->stat & SPIFI_STAT_CMD);

    xprintf("JEDEC ID: 0x%02X%02X%02X\n", buff[0], buff[1], buff[2]);

    SPIFI_REGS->cmd =
        (OP_READ_SREG1 << SPIFI_CMD_OPCODE_BIT) |
        SPIFI_CMD_FRAMEF_OPNA | 1;
    buff[0] = SPIFI_REGS->data8;
    while (SPIFI_REGS->stat & SPIFI_STAT_CMD);

    SPIFI_REGS->cmd =
        (OP_READ_SREG2 << SPIFI_CMD_OPCODE_BIT) |
        SPIFI_CMD_FRAMEF_OPNA | 1;
    buff[1] = SPIFI_REGS->data8;
    while (SPIFI_REGS->stat & SPIFI_STAT_CMD);

    xprintf("SREG1: 0x%02X, SREG2: %02X\n", buff[0], buff[1]);
}

