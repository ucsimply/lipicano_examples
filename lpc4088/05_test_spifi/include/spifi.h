/*
 * spifi.h
 * SPIFI test
 */

#ifndef _SPIFI_H_
#define _SPIFI_H_

#define SPIFI_FLASH_SIZE      (1024 * 1024 * 8)       /* W25Q64 device */

extern void spifi_init(void);
extern void spifi_switch_to_cmd(void);
extern void spifi_switch_to_mem(void);

#endif /* _SPIFI_H_ */

