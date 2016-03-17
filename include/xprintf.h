/*----------------------------*/
/* XPRINTF  (C)ChaN, 2011     */
/*----------------------------*/


#ifndef _XPRINTF_H_
#define _XPRINTF_H_

extern void (*xfunc_out)(unsigned char);
extern void xprintf (const char* fmt, ...);
extern int xsprintf (char* buff, const char* fmt, ...);


#endif /* _XPRINTF_H_ */

