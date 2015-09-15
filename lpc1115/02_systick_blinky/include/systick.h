/*
 * systick.h
 *
 */

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

enum {
    SYSTICK_DISABLE,
    SYSTICK_ENABLE
};

void systick_init(void);
void systick_ienb(int enable);

#endif /* _SYSTICK_H_ */
