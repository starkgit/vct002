#ifndef __UART_SYSLINK_H
#define __UART_SYSLINK_H
#include <stdbool.h>
#include "sys.h"






void uartslkInit(void);		/*���ڳ�ʼ��*/
bool uartslkTest(void);
bool uartslkGetDataWithTimout(u8 *c);	/*�ӽ��ն��ж�ȡ����(����ʱ����)*/
void uartslkSendData(u32 size, u8* data);	/*����ԭʼ����*/
void uartslkSendDataIsrBlocking(u32 size, u8* data);/*�жϷ�ʽ����ԭʼ����*/
int uartslkPutchar(int ch);		/*����һ���ַ�������*/
void uartslkSendDataDmaBlocking(u32 size, u8* data);/*ͨ��DMA����ԭʼ����*/
void uartslkIsr(void);		/*�����жϷ�����*/
void uartslkDmaIsr(void);	/*DMA�жϷ�����*/
void uartslkTxenFlowctrlIsr(void);

#endif /* __UART_SYSLINK_H */
