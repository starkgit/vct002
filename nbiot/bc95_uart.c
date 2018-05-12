/**
  ******************************************************************************
  * @file    bc95_uart.c
  * @author  stark
  * @version V1.0
  * @date    2017-12-31
  * @brief   .C file
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
#include "bc95_uart.h"
#include "user_lib.h"


const char receive_table[10]="+NSONMI:0,";

static Uart_Receive_Def BC95_Uart_Receive;


const Uart_Receive_Def* get_bc95_receive_param(void)
{
	return &BC95_Uart_Receive;
}


Uart_Receive_Def* set_bc95_receive_param(void)
{
	return &BC95_Uart_Receive;
}


void bc95_get_server_data_len(unsigned char rbuf)
{
	BC95_Uart_Receive.rbuf[BC95_Uart_Receive.rlen] = rbuf ;
	
	// check
	switch ( BC95_Uart_Receive.state )
	{
	    case RECEIVE_CHECK :
			if(BC95_Uart_Receive.rlen < 10)
			{
				if(receive_table[BC95_Uart_Receive.rlen] == BC95_Uart_Receive.rbuf[BC95_Uart_Receive.rlen])
				{
					BC95_Uart_Receive.rlen++;	
				}
				else
				{
					BC95_Uart_Receive.rlen = 0; 
				}
			}
			else  //>=10
			{
				if(rbuf == 0x0d || rbuf == 0x0a)
				{
					//	receive complete
					if(BC95_Uart_Receive.rlen <= 10)
					{
						BC95_Uart_Receive.rlen = 0;
					}
					else
					{
						char data[10]="";
						char len = BC95_Uart_Receive.rlen - 10;
						get_str_data(data,BC95_Uart_Receive.rbuf,10,len);
						BC95_Uart_Receive.server_send_len = (uint8_t) StringToInt(data);		
						BC95_Uart_Receive.rlen = 0;
						BC95_Uart_Receive.state = RECEIVE_COMPLETE;
					}
				}
				else
				{
					BC95_Uart_Receive.rlen++;	
					if(BC95_Uart_Receive.rlen > BC95_CHECK_SERVER_DATA_LEN)
					{
						BC95_Uart_Receive.rlen = 0;
						BC95_Uart_Receive.state = RECEIVE_COMPLETE;
					}
				}
			}
	        break;
	    case RECEIVE_COMPLETE:
	        break;
	    default:
	        break;
	}
}


#if 0
void bc95_rxd(unsigned char rbuf)
{
	switch ( BC95_Uart_Receive.state )
	{
	    case RECEIVE_START :
			if(rbuf == 0x0d)		// received 0x0d
	    	{
				BC95_Uart_Receive.state = RECEIVE_CHECK; 
			}
			else
			{
				BC95_Uart_Receive.rbuf[BC95_Uart_Receive.rlen] = rbuf ;
				BC95_Uart_Receive.rlen++;
				if(BC95_Uart_Receive.rlen > (BC95_MAX_UART_LEN-1))	// data overflow, rereceive
				{
					BC95_Uart_Receive.rlen = 0;						
				}
			}
	        break;
	    case RECEIVE_CHECK :
			if(rbuf == 0x0a && BC95_Uart_Receive.rlen != 0)		 	// received 0x0a
			{
				BC95_Uart_Receive.state = RECEIVE_COMPLETE;			// received ok
			}
			else
			{
				BC95_Uart_Receive.rlen = 0;
				BC95_Uart_Receive.state = RECEIVE_START;			// data error 
			}
	        break;
	    case RECEIVE_COMPLETE :
	        break;
	    default:
	        break;
	}
}
#endif












