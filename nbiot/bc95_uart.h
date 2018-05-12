/**
  ******************************************************************************
  * @file    bc95_uart.h
  * @author  stark
  * @version V1.0
  * @date    2017-12-31
  * @brief   bc95_uart.c header file
  ******************************************************************************
  * @attention
  *  
  * THE PURPOSE OF THE TEAM: 
  * TO WIN THE CREDIBILITY OF THE CUSTOMER, TO WIN THE QUALITY OF LIFE.
  * 
  * Copyright (C), 2017, Shenzhen Viketor Technology Co.,Ltd.
  ******************************************************************************
  * 
  */  

/* Includes ------------------------------------------------------------------*/

#ifndef __BC95_UART_H__
#define __BC95_UART_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
#include "stm32f10x.h"
#define USER_BC95_UART

#define BC95_CHECK_SERVER_DATA_LEN 			20  	

typedef enum
{
#if 0
	RECEIVE_START,
#endif
	RECEIVE_CHECK,	
	RECEIVE_COMPLETE, // 
}E_Receive_STATE;



typedef struct
{
	E_Receive_STATE state;
	char rbuf[BC95_CHECK_SERVER_DATA_LEN];
	uint16_t rlen;
	uint16_t server_send_len;
}Uart_Receive_Def;


extern void bc95_get_server_data_len(unsigned char rbuf);
extern const Uart_Receive_Def* get_bc95_receive_param(void);
extern Uart_Receive_Def* set_bc95_receive_param(void);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __BC95_UART_H__ */
