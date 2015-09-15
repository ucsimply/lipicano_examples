/*
 * crp.c
 *
 */
#include <stdint.h>

/* Patterns for Code Read Protection (CRP) are explained
 * in LPC11xx User Manuals
 */

#define CRP_NO_CRP      0xFFFFFFFF
#define CRP_NO_ISP      0x4E697370
#define CRP_CRP1        0x12345678
#define CRP_CRP2        0x87654321

/* *****************************************************
 * DANGER - CRP3 WILL LOCK PART TO ALL READS and WRITES
 * *****************************************************
 */
#define CRP_CRP3_CONSUME_PART        0x43218765

__attribute__ ((section(".crp"))) const uint32_t crp_code = CRP_NO_CRP;
