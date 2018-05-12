#ifndef _BSP_USART_FIFO_H_
#define _BSP_USART_FIFO_H_
#include "stm32f10x.h"
#include "stdbool.h"

/* 定义使能的串口, 0 表示不使能（不增加代码大小）， 1表示使能 */
/*
	【串口1】 GPS模块
		PA9/USART1_TX	   
		PA10/USART1_RX

	【串口2】NRF51822模块 
		PA2/USART2_TX
		PA3/USART2_RX 

	【串口3】 NBIOT模块
		PB10/USART3_TX
		PB11/USART3_RX
*/
#define	UART1_FIFO_EN	1
#define	UART2_FIFO_EN	1
#define	UART3_FIFO_EN	1


/* 定义端口号 */
typedef enum
{
	COM1 = 0,	/* USART1  PA9, PA10 */
	COM2 = 1,	/* USART2, PA2, PA3 */
	COM3 = 2,	/* USART3, PB10, PB11 */
}COM_PORT_E;




#if UART1_FIFO_EN == 1		// bt  
	#define UART1_BAUD					9600
	#define UART1_BUFFER_SIZE_TRANSMIT	32
	#define UART1_BUFFER_SIZE_RECEIVE	32
	typedef struct
	{
	    volatile uint32_t head_transmit;
	    volatile uint32_t tail_transmit;
	    volatile uint32_t head_receive;
	    volatile uint32_t tail_receive;
	     uint8_t buffer_transmit[UART1_BUFFER_SIZE_TRANSMIT];
	     uint8_t buffer_receive[UART1_BUFFER_SIZE_RECEIVE];
	}UART1_DEF;
#endif

// TFT , TX > RX ,  TX(display) RX(Read key)
#if UART2_FIFO_EN == 1
	#define UART2_BAUD			115200
	#define UART2_BUFFER_SIZE_TRANSMIT	1024
	#define UART2_BUFFER_SIZE_RECEIVE	30
	typedef struct
	{
		volatile uint32_t head_transmit;
		volatile uint32_t tail_transmit;
		volatile uint32_t head_receive;
		volatile uint32_t tail_receive;
		volatile uint8_t buffer_transmit[UART2_BUFFER_SIZE_TRANSMIT];
		volatile uint8_t buffer_receive[UART2_BUFFER_SIZE_RECEIVE];
	}UART2_DEF;
#endif

#if UART3_FIFO_EN == 1 	// NBIOT , RX > TX
	#define UART3_BAUD			9600
	#define UART3_BUFFER_SIZE_TRANSMIT	100
	#define UART3_BUFFER_SIZE_RECEIVE	500
	typedef struct
	{
		volatile uint32_t head_transmit;
		volatile uint32_t tail_transmit;
		volatile uint32_t head_receive;
		volatile uint32_t tail_receive;
		volatile uint8_t buffer_transmit[UART3_BUFFER_SIZE_TRANSMIT];
		volatile uint8_t buffer_receive[UART3_BUFFER_SIZE_RECEIVE];
	}BC95_UART3_DEF;
#endif






enum
{
    UART_SUCCEED = 0,
    UART_FAILED  = 1
};




// user add  for gps , to uart1
extern char rev_buf[80]; //接收缓存
extern uint8_t num;
extern bool rev_stop,isRxInt; //停止接收,接受中断


void UartInit(void);
#if UART1_FIFO_EN == 1
void Uart1SetBaud(uint32_t _baud);
uint8_t uart1_transmit(const void * data,uint32_t len);
int32_t uart1_receiveValidBufferSize(void);
int32_t uart1_readReceiveBuffer(void * outputBuffer,int32_t outputBufferLength);
#endif

#if UART2_FIFO_EN == 1
void Uart2SetBaud(uint32_t _baud);
uint8_t uart2_transmit(const void * data,uint32_t len);
int32_t uart2_receiveValidBufferSize(void);
int32_t uart2_readReceiveBuffer(void * outputBuffer,int32_t outputBufferLength);
#endif

#if UART3_FIFO_EN == 1
void Uart3SetBaud(uint32_t _baud);
uint8_t uart3_transmit(const void * data,uint32_t len);
int32_t uart3_receiveValidBufferSize(void);
int32_t uart3_readReceiveBuffer(void * outputBuffer,int32_t outputBufferLength);
#endif




#endif


