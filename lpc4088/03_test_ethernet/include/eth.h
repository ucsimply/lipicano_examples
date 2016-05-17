/*
 * eth.h
 * low level ethernet control
 */

#ifndef _ETH_H_
#define _ETH_H_

void eth_init(void);
void eth_display_phy_regs(void);
int eth_packet_get(void **pbuff);
void *eth_txbuff_get(void);
void eth_packet_send(int size);
void eth_inc_rxidx(void);

#endif /* _ETH_H_ */

