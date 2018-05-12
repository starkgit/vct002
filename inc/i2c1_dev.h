#ifndef _I2C1_DEV_H
#define _I2C1_DEV_H






extern void iicDevInit(void);
extern unsigned char iicDevReadByte(unsigned char DevAdd,unsigned char RegAdd);
extern void iicDevWriteByte(unsigned char DevAdd,unsigned char RegAdd,unsigned char data);
extern void iicDevRead(unsigned char DevAdd,unsigned char RegAdd,
								unsigned char len ,unsigned char *rbuf);
extern void iicDevWrite(unsigned char DevAdd,unsigned char RegAdd,
								unsigned char len,unsigned char *wbuf);







#endif






