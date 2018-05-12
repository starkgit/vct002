#ifndef __UART_SYSLINK_H
#define __UART_SYSLINK_H
#include <stdbool.h>
#include "sys.h"






void uartslkInit(void);		/*串口初始化*/
bool uartslkTest(void);
bool uartslkGetDataWithTimout(u8 *c);	/*从接收队列读取数据(带超时处理)*/
void uartslkSendData(u32 size, u8* data);	/*发送原始数据*/
void uartslkSendDataIsrBlocking(u32 size, u8* data);/*中断方式发送原始数据*/
int uartslkPutchar(int ch);		/*发送一个字符到串口*/
void uartslkSendDataDmaBlocking(u32 size, u8* data);/*通过DMA发送原始数据*/
void uartslkIsr(void);		/*串口中断服务函数*/
void uartslkDmaIsr(void);	/*DMA中断服务函数*/
void uartslkTxenFlowctrlIsr(void);

#endif /* __UART_SYSLINK_H */
