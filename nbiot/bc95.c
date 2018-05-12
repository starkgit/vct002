/**
  ******************************************************************************
  * @file    bc95.c
  * @author  stark
  * @version V1.0
  * @date    2017-11-20
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

#include "bc95.h"
#include "bc95_uart.h"
#include "string.h"
#include "uart_fifo.h"
#include "bc95_udp_protocol.h"


/*FreeRTOS?????*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "debug_assert.h"

#include "stabilizer_types.h"
#include "user_lib.h"
#include "math.h"
#include "g_public.h"
#include "algorithm.h"


// HARDWARE - > UART3 (UART3_FIFO_EN)

#define USARTx_printf(...)
#define usart_send_str(...)




#define BC95_PWR_ON_TEST

//#undef  BC95_PWR_ON_TEST

#define UE_LOCAL_UDP_PORT        3005
//#define SERVER_HOST_UDP_IP       "106.14.171.181"
#define SERVER_HOST_UDP_IP       "106.14.171.181"
#define SERVER_HOST_UDP_PORT     8234

//????
#define SET_UE_DEFAULT_BAND      "AT+NBAND=5"         
#define UE_DEFAULT_BAND          "+NBAND:5"


#define SET_UE_REBOOT            "AT+NRB"

#define QUERY_UE_BAND            "AT+NBAND?"
#define SET_UE_BAND_5            "AT+NBAND=5"
#define SET_UE_BAND_8            "AT+NBAND=8"

#define QUERY_UE_CONNECT_MODE    "AT+NCONFIG?"
#define SET_UE_AUTOCONNECT       "AT+NCONFIG=AUTOCONNECT,TRUE"
#define SET_UE_MANUALCONNECT     "AT+NCONFIG=AUTOCONNECT,FALSE"

#define QUERY_UE_FUNC            "AT+CFUN?"
#define SET_UE_FUNC_0            "AT+CFUN=0"
#define SET_UE_FUNC_1            "AT+CFUN=1"


#define QUERY_UE_SIM_STATS		 "AT+CIMI"


#define QUERY_UE_SIGNAL_QTY      "AT+CSQ"
#define UE_NO_SIGNAL_QTY		 "+CSQ:99,99"  


#define QUERY_UE_ATTACH_STATS    "AT+CGATT?"
#define UE_ATTACHED_STATS        "+CGATT:1"
#define SET_UE_ATTACH            "AT+CGATT=1"

#define QUERY_UE_EREG_STATS      "AT+CEREG?"
#define UE_EREGISTERING_STATS    "+CEREG:0,2"
#define UE_EREGISTERED_STATS     "+CEREG:0,1"
#define SET_UE_EREG              "AT+CEREG=1"

#define QUERY_UE_SCCON_STATS     "AT+CSCON?"
#define SET_UE_SCCON             "AT+CSCON=1"
#define UE_CONNECTED             "+CSCON:0,1"

#define NEUL_MAX_BUF_SIZE 256
#define NEUL_MAX_SOCKETS 1 // the max udp socket links
#define NEUL_IP_LEN 16
#define NEUL_MANUFACT_LEN 20


static char send_server_buf[30];
static int send_server_len = 0;

static char receive_server_buf[NEUL_MAX_BUF_SIZE];
static int receive_server_len = 0;

static char neul_bc95_rbuf[NEUL_MAX_BUF_SIZE];
static char neul_bc95_wbuf[NEUL_MAX_BUF_SIZE];
static char neul_bc95_tmpbuf[NEUL_MAX_BUF_SIZE];

static neul_dev_operation_t neul_ops = {
    bc95_receive_cmd_handing,
    bc95_send_cmd_handing,
};

static remote_info udp_socket[NEUL_MAX_SOCKETS] = {
	{0, -1, {0}}
};

static neul_dev_info_t neul_dev = {
    neul_bc95_rbuf,
    neul_bc95_wbuf,
    0,
    udp_socket,
    &neul_ops,
    {0}
};



static BC_SEND_E sBcSendState = bc_send_start;
static BC_POWER_ON_STATE sPowerOnState = bc_power_on_start;
static BC_QUERY_NET_STATE sQueryNetState = bc_query_net_start;
static BC_CreateUDP_STATE sCreateUdpState = bc_create_udp_start;
static BC_UDPSendMsg_STATE sUdpSendState = bc_send_msg_start;
static BC_UDPReceiveMsg_STATE sUdpReceiveState = bc_receive_msg_start;
static BC_UDPProtocolMsg_STATE sUdpProtocolState = bc_protocol_start;
static BC_TASK_STATE task_state = bc_state_variable_init;

	
#define bc_send_enable() 		sBcSendState = bc_send_start
#define bc_udp_send_enable() 	sUdpSendState = bc_send_msg_start
#define bc_udp_receive_enable() sUdpReceiveState = bc_receive_msg_start


void bc95Task(const u32 tick)
{
	if (RATE_DO_EXECUTE(RATE_1000_HZ, tick)) /** 1000Hz 1ms update **/
	{
		bc95_task_process();
	}
}



// 1ms once
void bc95_task_process(void)
{
	
	switch(task_state)
	{
		case bc_state_variable_init:
			protocol_data_initial();
			task_state = bc_state_power_on;
			break;
		case bc_state_power_on:
			if(bc95_power_on_handing() == bc_power_on_normal)
			{
				task_state = bc_state_net_query;
			}
			break;	
		case bc_state_net_query:
			if(bc95_query_net_status() == bc_query_net_attched_register_success)
			{
				task_state = bc_state_create_upd;
			}
			break;		
		case bc_state_create_upd:
			if(neul_bc95_create_udpsocket(UE_LOCAL_UDP_PORT) == bc_create_udp_success)
			{
				task_state = bc_state_work;
			}
			break;
		case bc_state_work:
			protocl_bc95_udp_task();
					
			break;
		default:
			break;


	}
}


/**
  * @brief  check ack data dptr
  * @param  checked string
  * @retval ack string dptr position
  * @author stark
  * @date   2017-11-22
  */
uint8_t* bc95_check_ack(char *str)
{
	char *str_x=0;
	int32_t len = 0;
	len =  uart3_receiveValidBufferSize();

	
	// have data 
	if(len > 0 && len< (sizeof(neul_bc95_rbuf) - 1))
	{
		uart3_readReceiveBuffer(neul_bc95_rbuf,len);
		neul_bc95_rbuf[len] = '\0';
		str_x=mystrstr((char*)neul_bc95_rbuf,(char*)str);
	}
	
	return (uint8_t*)str_x;
}


BC_SEND_E bc95_receive_cmd_handing(char *cmd,char *ack,uint16_t WAITCOUNT)
{
	return bc_send_start;
}

/**
  * @brief  send cmd and wait ack cmd 
  * @param  cmd string
  * @param  ack string
  * @param  wait time out
  * @retval send status
  * @author stark
  * @date   2017-11-22
  */
BC_SEND_E bc95_send_cmd_handing(char *cmd,char *ack,uint16_t timeout)
{
	static uint32_t sTimeout = 0;
	uint16_t len = 0;
	if(sTimeout) sTimeout--;
	
	switch(sBcSendState)
	{
		case bc_send_start: // init prepare and send cmd data
			// clear receive data 
			memset(neul_bc95_rbuf,0,sizeof(neul_bc95_rbuf));
			memset(neul_bc95_wbuf,0,sizeof(neul_bc95_wbuf));
			
			len = strlen(cmd);
			if(len > sizeof(neul_bc95_wbuf) - 2) break;		
			memcpy(neul_bc95_wbuf,cmd ,len);
			neul_bc95_wbuf[len] = '\r';
			neul_bc95_wbuf[len+1]= '\n';
		
			uart3_transmit(neul_bc95_wbuf,len+2);
			sBcSendState = bc_send_check;
			sTimeout = timeout;					// wait 30ms
			break;
		case bc_send_check:   
			if(sTimeout) break;

			// next status 
			if(bc95_check_ack(ack))
			{
				sBcSendState = bc_send_ack_wait;		// ack is ok
			}
			else
			{
				sBcSendState = bc_send_noack_wait;		// ack is ok
			}
			sTimeout = 50;							// wait 50ms	
			break;
		case bc_send_ack_wait:			
			if(sTimeout) break;
			
			// next status 
			sBcSendState = bc_send_ack; 
			break;
		case bc_send_noack_wait:
			if(sTimeout) 	break;

			// next status 
			sBcSendState = bc_send_noack; 			
			break;
		case bc_send_ack:
			
			break;
		case bc_send_noack:
			
			break;
		default:
			break;
	}
	return sBcSendState;
}


/**
  * @brief  Power up program, check whether the module is connected, 
  	 		check whether the configuration is automatic mode, 
  	 		whether the required frequency band
  * @param  none
  * @retval status
  * @author stark
  * @date   2017-11-22
  */
BC_POWER_ON_STATE bc95_power_on_handing(void)
{
	static uint16_t sPowerOnTimeOut = 0;
	static unsigned char sPowerOnTimeCount = 0;
	BC_SEND_E bc_state;
	
	switch(sPowerOnState)
	{
		case bc_power_on_start:
			sPowerOnState = bc_power_on_send_at;
			bc_send_enable();
			sPowerOnTimeCount = 10;
			break;
		case bc_power_on_send_at:
			 bc_state = neul_dev.ops->dev_write("AT","OK",100);
			if(bc_state == bc_send_ack)
			{
				sPowerOnState = bc_power_on_ack_at;
			}
			else if(bc_state == bc_send_noack)
			{
				sPowerOnState = bc_power_on_noack_at;
				sPowerOnTimeOut = 1000; 
			}
			break;
		case bc_power_on_noack_at:	// noack , wait 1s
			if(sPowerOnTimeOut) 
			{
				sPowerOnTimeOut--;
				break;
			}
			if(sPowerOnTimeCount)
			{
				sPowerOnTimeCount--;
				sPowerOnTimeOut = 1000;
				sPowerOnState = bc_power_on_send_at;
				bc_send_enable();
			}
			else
			{		// error
				sPowerOnState = bc_power_on_error;
			}
			break;
		case bc_power_on_ack_at:  // next send auto connect cmd
			sPowerOnState = bc_power_on_send_query_autoconnect_false;
			bc_send_enable();
			break;
		case bc_power_on_send_query_autoconnect_false:
			 bc_state = neul_dev.ops->dev_write(QUERY_UE_CONNECT_MODE,"AUTOCONNECT,FALSE",100);
			if(bc_state == bc_send_ack) //  "AUTOCONNECT,FALSE"
			{
				sPowerOnState = bc_power_on_ack_autoconnect_false;
			}
			else if(bc_state == bc_send_noack)
			{							//  "AUTOCONNECT,TRUE"
				sPowerOnState = bc_power_on_noack_autoconnect_false;
				sPowerOnTimeOut = 1000; 
			}
			break;
		case bc_power_on_noack_autoconnect_false:   
			if(sPowerOnTimeOut) 		//  "AUTOCONNECT,TRUE"
			{
				sPowerOnTimeOut--;
				break;
			}
			sPowerOnState = bc_power_on_send_query_defaultband;
			bc_send_enable();
			break;
		case bc_power_on_ack_autoconnect_false: // next send set default frequence
			sPowerOnState = bc_power_on_send_set_autoconnect_true;
			bc_send_enable();
			sPowerOnTimeCount = 3;
			break;

		case bc_power_on_send_set_autoconnect_true:
			 bc_state = neul_dev.ops->dev_write(SET_UE_AUTOCONNECT,"OK",100);
			if(bc_state == bc_send_ack) // return "AUTOCONNECT,ok"
			{
				sPowerOnState = bc_power_on_ack_autoconnect_true;
			}
			else if(bc_state == bc_send_noack)
			{
				sPowerOnState = bc_power_on_noack_autoconnect_true;
				sPowerOnTimeOut = 1000; 
			}
			break;
		case bc_power_on_noack_autoconnect_true:
			if(sPowerOnTimeOut) 
			{
				sPowerOnTimeOut--;
				break;
			}
			if(sPowerOnTimeCount)
			{
				sPowerOnTimeCount--;
				sPowerOnTimeOut = 1000;
				sPowerOnState = bc_power_on_send_set_autoconnect_true;
				bc_send_enable();
			}
			else
			{		// error
				sPowerOnState = bc_power_on_error;
			}
			break;		
		case bc_power_on_ack_autoconnect_true:
			sPowerOnState = bc_power_on_send_query_defaultband;
			bc_send_enable();
			break;
		case bc_power_on_send_query_defaultband:
			 bc_state = neul_dev.ops->dev_write(QUERY_UE_BAND,UE_DEFAULT_BAND,100);
			if(bc_state == bc_send_ack)
			{
				sPowerOnState = bc_power_on_send_reboot;
				bc_send_enable();
			}
			else if(bc_state == bc_send_noack)
			{
				sPowerOnState = bc_power_on_send_set_defaultband;
				bc_send_enable();
			}
			break;

		case bc_power_on_send_set_defaultband:
			 bc_state = neul_dev.ops->dev_write(SET_UE_DEFAULT_BAND,UE_DEFAULT_BAND,100);
			if(bc_state == bc_send_ack || bc_state == bc_send_noack)
			{
				sPowerOnState = bc_power_on_send_query_defaultband;
				bc_send_enable();
			}
			break;
		case bc_power_on_send_reboot:
			 bc_state = neul_dev.ops->dev_write(SET_UE_REBOOT,"REBOOT",100);
			if(bc_state == bc_send_ack )
			{
				sPowerOnState = bc_power_on_ack_reboot;
				sPowerOnTimeOut = 1000; 
				sPowerOnTimeCount = 5;	// wait 5s
			}
			else if(bc_state == bc_send_noack)
			{
				sPowerOnState = bc_power_on_noack_reboot;
				sPowerOnTimeOut = 1000; 
				sPowerOnTimeCount = 2; // 2s
			}
			break;
		case bc_power_on_noack_reboot:
			if(sPowerOnTimeOut) 
			{
				sPowerOnTimeOut--;
				break;
			}
			if(sPowerOnTimeCount)
			{
				sPowerOnTimeOut = 1000;
				sPowerOnTimeCount--;
				break;
			}
			sPowerOnState = bc_power_on_send_reboot;  // continue send reboot
			bc_send_enable();
			break;
		case bc_power_on_ack_reboot:
			if(sPowerOnTimeOut) 
			{
				sPowerOnTimeOut--;
				break;
			}
			if(sPowerOnTimeCount)
			{
				sPowerOnTimeCount--;
				sPowerOnTimeOut = 1000;
				break;
			}
			sPowerOnState = bc_power_on_wait_neul;
			break;
		case bc_power_on_wait_neul:
			if(bc95_check_ack("Neul"))
			{ // recevie ok
				sPowerOnState = bc_power_on_normal;
			}
			else
			{// receive time out
				sPowerOnState = bc_power_on_normal;
			}
			break;
		
		case bc_power_on_normal:
			break;

		case bc_power_on_error: 
			break;
		default:
			break;
	}
	return sPowerOnState;
}



//?????????,????LED1???,LED1?????????
//??????????,?????????,????????BC95_send_cmd,????
//????????????0,??1??????,??2????
BC_QUERY_NET_STATE bc95_query_net_status(void)
{
	BC_SEND_E bc_send_state;

	switch(sQueryNetState)
	{
		case bc_query_net_start:
			sQueryNetState = bc_query_sim_card;
			bc_send_enable();
			break;
		case bc_query_sim_card:
			bc_send_state = neul_dev.ops->dev_write(QUERY_UE_SIM_STATS,"OK",100);
			if(bc_send_state == bc_send_ack)	 
			{
				sQueryNetState = bc_query_sim_signal;
				bc_send_enable();
			}
			else if(bc_send_state == bc_send_noack)// continue query sim state
			{

				bc_send_enable();
			}
			break;
		case bc_query_sim_signal:
			bc_send_state = neul_dev.ops->dev_write(QUERY_UE_SIGNAL_QTY,UE_NO_SIGNAL_QTY,100);//no signal
			if(bc_send_state == bc_send_noack)	 
			{				
				sQueryNetState = bc_query_net_attched_state;
				bc_send_enable();
			}
			else if(bc_send_state == bc_send_ack)
			{
				bc_send_enable();
			}
			break;
		case bc_query_net_attched_state:
			bc_send_state = neul_dev.ops->dev_write(QUERY_UE_ATTACH_STATS,UE_ATTACHED_STATS,100);
			if(bc_send_state == bc_send_ack)	// Attached net is ok
			{
				sQueryNetState = bc_query_net_register_state; 	// query register state
				bc_send_enable();
			}
			else if(bc_send_state == bc_send_noack)
			{
				bc_send_enable();
			}
			break;
		case bc_query_net_register_state:
			bc_send_state = neul_dev.ops->dev_write(QUERY_UE_EREG_STATS,UE_EREGISTERED_STATS,100);
			if(bc_send_state == bc_send_ack)	// registered net is ok
			{
				sQueryNetState = bc_query_net_sccon_state; // to success
				bc_send_enable();
			}
			else if(bc_send_state == bc_send_noack)
			{
				bc_send_enable();
			}
			break;
		case bc_query_net_sccon_state:
			bc_send_state = neul_dev.ops->dev_write(QUERY_UE_SCCON_STATS,UE_CONNECTED,100);
			if(bc_send_state == bc_send_ack)	// Attached net is ok
			{
				sQueryNetState = bc_query_net_attched_register_success;  // net success
			}
			else if(bc_send_state == bc_send_noack)
			{
				bc_send_enable();
			}
			break;
		case bc_query_net_attched_register_success:
			break;
		default:
			break;
	}

	return sQueryNetState;
}




/* ============================================================
func name   :  neul_bc95_create_udpsocket
discription :  create udp socket 
               
param       :  localport @ input param , the port local socket used
return      :  >=0 socket handle , < 0 not ok
Revision    : 
=============================================================== */
BC_CreateUDP_STATE neul_bc95_create_udpsocket(unsigned short localport)
{
	BC_SEND_E bc_send_state;	
	
	switch(sCreateUdpState)
	{
		case bc_create_udp_start:
			sCreateUdpState = bc_create_udp_proc;
			bc_send_enable();
			{
				char *cmd = "AT+NSOCR=DGRAM,17,";
				if (0 == localport)
				{
					break;
				}
				memset(neul_bc95_tmpbuf, 0, 64);
				sprintf(neul_bc95_tmpbuf, "%s%d,1\r", cmd, localport);
			}
			break;
		case bc_create_udp_proc:
			bc_send_state = neul_dev.ops->dev_write(neul_bc95_tmpbuf,"OK",100);
			if(bc_send_state == bc_send_ack)				
			{
				uint8_t tUpdSocketId = 0x07;	
				char data[10]="";
				get_str_data(data,(char *)neul_dev.rbuf,2,1);
				tUpdSocketId = (uint8_t) StringToInt(data);				
				neul_bc95_set_upd_param(SERVER_HOST_UDP_IP,SERVER_HOST_UDP_PORT,tUpdSocketId);
				sCreateUdpState = bc_create_udp_success;	// to success
			}
			else if(bc_send_state == bc_send_noack)
			{
				sCreateUdpState = bc_create_udp_fail;	
			}
			break;
		case bc_create_udp_success:
			break;
		case bc_create_udp_fail:
			
		default:
			break;
			
	}
	return sCreateUdpState;
}



/* ============================================================
func name   :   neul_bc95_set_upd_param
discription :   this func just called after the socket created
                set the remote ip address and port the socket sendto or receive fro
param       :   
             rip  @ input param, remote client/server ip address
             port @ input param, remote client/server port
             socket @ input param, local socket handle
return      :  0 mean ok, !0 means param err
Revision    : 
=============================================================== */
int neul_bc95_set_upd_param(const char *rip, const unsigned short port, const int socket)
{
    if (NULL == rip || 0 == port)
    {
        return -1;
    }
    if (strlen(rip) >= NEUL_IP_LEN)
    {
        return -2;
    }
    neul_dev.remotecount++;
    (neul_dev.addrinfo+socket)->port = port;
    (neul_dev.addrinfo+socket)->socket = socket;
    memcpy((neul_dev.addrinfo+socket)->ip, rip, strlen(rip));
    return 0;
}

uint16_t neul_bc95_get_data_len(void)
{
	const Uart_Receive_Def*	get_param = get_bc95_receive_param();
	Uart_Receive_Def*	set_param = set_bc95_receive_param();

	if(get_param->state == RECEIVE_COMPLETE )
	{
		set_param->state = RECEIVE_CHECK;
		if(get_param->server_send_len != 0)
		{
			return get_param->server_send_len;
		}
	}
#if 0
	static unsigned char get_state = 0;
	static unsigned char pos_count = 0 ;
	static unsigned int check_server_len_time = 0;
	uint8_t server_data_len = 0;
	char *str_x=0;
	char data[30]="";
	int32_t len = 0;
	if(check_server_len_time)	check_server_len_time--;
	
	// have data
	switch ( get_state )
	{
	    case 0 :	// \r\n+NSONMI:0,
			len =  uart3_receiveValidBufferSize();
			if(len >= 12 && len< (sizeof(data) - 1))
			{
				uart3_readReceiveBuffer(data,len);
				data[len] = '\0';
				str_x = mystrstr((char*)data,("+NSONMI:0,")); // \r\n++NSONMI:0, = 12byte
				if(str_x)
				{
					get_state = 1;
					pos_count = 0;
				}
			}
	        break;
	    case 1 :
			len =  uart3_receiveValidBufferSize();
			if(len > 0)
	        {
				char temp_data;
				uart3_readReceiveBuffer(&temp_data,1);
				if(temp_data != 0x0a && temp_data != 0x0d)
				{
					neul_bc95_rbuf[pos_count++] = temp_data;
				}
				else
				{
					char data[10]="";
					get_str_data(data,neul_bc95_rbuf,0,pos_count);
					server_data_len = (uint8_t) StringToInt(data); 			
					get_state = 0;
					return server_data_len;
				}
			}
	        break;
	    default:
	        break;
	}
#endif

	return 0;
}

/* ============================================================
func name   :  bc95_send_udp_msg
discription :  send data to socket 
               
param       :  socket @ input param , the data will send to this socket
               buf @ input param, the data buf
               sendlen @ input param, the send data length
return      :  0 send data ok , !0 not ok
Revision    : 
=============================================================== */
BC_UDPSendMsg_STATE protocl_bc95_udp_send_msg(int socket, const char *buf, int sendlen)
{
	BC_SEND_E bc_send_state;
	static unsigned int wait_server_send_time = 0;
	static unsigned char error_count = 0;
	if(error_count)error_count--;
	if(wait_server_send_time)	wait_server_send_time--;
	
	switch(sUdpSendState)
	{
		case bc_send_msg_start:
			{
				char *cmd = "AT+NSOST=";
				char temp_buf[NEUL_MAX_BUF_SIZE];
				if (socket < 0 || NULL == buf || 0 == sendlen)
				{
					break;
				}
				memset(neul_bc95_tmpbuf, 0, NEUL_MAX_BUF_SIZE);
				HexToString((unsigned char *)buf, sendlen, temp_buf);
				sprintf(neul_bc95_tmpbuf, "%s%d,%s,%d,%d,%s\r", cmd, socket,
						   (neul_dev.addrinfo + socket)->ip,
						   (neul_dev.addrinfo + socket)->port,
						   sendlen,
						   temp_buf);
				sUdpSendState = bc_send_msg_proc;
				bc_send_enable();
				error_count = 10;
			}
			break;
		case bc_send_msg_proc:
		{	
			bc_send_state = neul_dev.ops->dev_write(neul_bc95_tmpbuf,"OK",100);
			if(bc_send_state == bc_send_ack)
			{
				sUdpSendState = bc_send_msg_success;
				
			}
			else if(bc_send_state == bc_send_noack)
			{
				if(error_count)
				{			
					bc_send_enable(); // continue send data,error 10 timers.
				}
				else
				{
					sUdpSendState = bc_send_msg_fail;
				}
			}
			break;
		}
		case bc_send_msg_success:
			sUdpSendState = bc_query_net_sscon;
			break;
		case bc_send_msg_fail:
			break;
		case bc_query_net_sscon:
			bc_send_state = neul_dev.ops->dev_write(QUERY_UE_SCCON_STATS,UE_CONNECTED,100);
			if(bc_send_state == bc_send_ack)
			{
				sUdpSendState = bc_query_net_sscon_ok;
			}
			else if(bc_send_state == bc_send_noack)
			{
				sUdpSendState = bc_query_net_sscon_fail;
			}
			break;
		case bc_query_net_sscon_ok:
			sUdpSendState = bc_check_receive_data;
			wait_server_send_time = 30000;	// 30s
			break;
		case bc_query_net_sscon_fail:

			break;
		case bc_check_receive_data:
			receive_server_len = neul_bc95_get_data_len();
			if(receive_server_len)
			{
				sUdpSendState = bc_get_receive_data_len;
			}
			else
			{
				if(wait_server_send_time == 0)
				{
					sUdpSendState = bc_get_receive_data_fail;
				}
			}
			break;
		case bc_get_receive_data_len:
			break;
		case bc_get_receive_data_fail:
			break;
		default:
			break;
	}	
	return sUdpSendState;
}



void bc95_master_send(char *send_buf,int len)
{
	unsigned char i ;
	if(len > sizeof(send_server_buf)) return;
	send_server_len = len;
	
	if(sUdpSendState == bc_check_receive_data 
		&& receive_server_len == 0
		&& sUdpProtocolState == bc_protocol_send_heart
	)
	{
		sUdpProtocolState = bc_protocol_send_heart_wait;
		
		for (i = 0; i < len; i++)
		{
		    send_server_buf[i] = send_buf[i];
		}
	}
}

// A? .....
void bc95_master_send_protocol_command(unsigned char param ,unsigned char keyval)
{
	char buf[20];
	unsigned char i = 0;
	unsigned short crcval;
	if(keyval == 0) return ;
	
	buf[i++] = 0xFA;
	buf[i++] = NBProtocol.cloud_address;
	buf[i++] = NBProtocol.device_address[0];
	buf[i++] = NBProtocol.device_address[1];
	buf[i++] = NBProtocol.device_address[2];
	buf[i++] = NBProtocol.device_address[3];

	buf[i++] = 0xA4;
	buf[i++] = param;
	buf[i++] = keyval;
	buf[i++] = NBProtocol.cmd_save_protocol[keyval - 1][0];
	buf[i++] = NBProtocol.cmd_save_protocol[keyval - 1][1];
	buf[i++] = NBProtocol.cmd_save_protocol[keyval - 1][2];
	buf[i++] = NBProtocol.cmd_save_protocol[keyval - 1][3];
	
	crcval = math_crc16(0,buf,i);
	buf[i++] = HIBYTE(crcval);
	buf[i++] = LOBYTE(crcval);
	bc95_master_send(buf,i);
}


/* ============================================================
func name   :  neul_bc95_udp_read
discription :  read data from socket 
               
param       :  socket @ input param , the data will read from this socket
               buf @ out param, store the data read
               maxrlen @ input param, the max read data length
return      :  >0 read data length , <0 not ok
Revision    : 
=============================================================== */
BC_UDPProtocolMsg_STATE protocl_bc95_udp_task(void)
{
	static uint32_t sProtocolTimeout = 0;
	static unsigned char sProtocolwait = 0;
	BC_UDPSendMsg_STATE bc_sendmsg_state;
	BC_UDPReceiveMsg_STATE bc_receivemsg_state;

	// timer count
	if(sProtocolTimeout)sProtocolTimeout--;
	if(sProtocolwait)sProtocolwait--;
	
	switch(sUdpProtocolState)
	{
		case bc_protocol_start:
		{	
			unsigned short crcval;
			unsigned char i = 0;
		#if 0
			if(device_address_bk[0] != NBProtocol.device_address[0]
				|| device_address_bk[1] != NBProtocol.device_address[1])
		#endif
			{
			#if 0
				device_address_bk[0] = NBProtocol.device_address[0];
				device_address_bk[1] = NBProtocol.device_address[1];
			#endif
				// count crc 
				send_server_buf[i++] = 0xFA;
				send_server_buf[i++] = NBProtocol.cloud_address;
				send_server_buf[i++] = NBProtocol.device_address[0];
				send_server_buf[i++] = NBProtocol.device_address[1];
				send_server_buf[i++] = NBProtocol.device_address[2];
				send_server_buf[i++] = NBProtocol.device_address[3];
				send_server_buf[i++] = 0xFF;
				crcval = math_crc16(0,send_server_buf,i);
				send_server_buf[i++] = HIBYTE(crcval);
				send_server_buf[i++] = LOBYTE(crcval);
				send_server_len = i;
			}
			sUdpProtocolState = bc_protocol_send_heart_wait;
			sProtocolTimeout = 500;
			break;
		}
		case bc_protocol_send_heart_wait:
		{	
			if(sProtocolTimeout) break;
			sUdpProtocolState = bc_protocol_send_heart;
			bc_udp_send_enable();
			break;
		}
		case bc_protocol_send_heart:
		{
			bc_sendmsg_state = protocl_bc95_udp_send_msg((neul_dev.addrinfo+neul_dev.addrinfo->socket)->socket, 
								send_server_buf,send_server_len);
			if(bc_sendmsg_state == bc_get_receive_data_len)
			{	
				sUdpProtocolState = bc_protocol_send_wait;
//				sProtocolTimeout = 2000;
			}
			else if(bc_sendmsg_state == bc_send_msg_fail 
					|| bc_sendmsg_state == bc_query_net_sscon_fail
					|| bc_sendmsg_state == bc_get_receive_data_fail)
			{
				sUdpProtocolState = bc_protocol_start;
			}
			break;
		}
		case bc_protocol_send_wait:
//			if(sProtocolTimeout) break;
			sUdpProtocolState = bc_protocol_receive_start;
			bc_udp_receive_enable();
			break;
		case bc_protocol_receive_start:
			bc_receivemsg_state = protocl_bc95_udp_receive_msg((neul_dev.addrinfo+neul_dev.addrinfo->socket)->socket, 
									receive_server_buf,&receive_server_len,NEUL_MAX_BUF_SIZE);// receiver data
			if(bc_receivemsg_state == bc_receive_msg_success)
			{
				sUdpProtocolState = bc_protocol_receive_wait;
//				sProtocolTimeout = 1000;
			}
			else if(bc_receivemsg_state == bc_receive_msg_null)
			{
				sUdpProtocolState = bc_protocol_start;
			}
			break;
		case bc_protocol_receive_wait:
//			if(sProtocolTimeout) break;
			protocol_data_proc(receive_server_buf,&receive_server_len);
			sUdpProtocolState = bc_protocol_send_ack;
			bc_udp_send_enable();
			break;
		case bc_protocol_send_ack:
			bc_sendmsg_state = protocl_bc95_udp_send_msg((neul_dev.addrinfo+neul_dev.addrinfo->socket)->socket, 
								receive_server_buf,receive_server_len);
			if(bc_sendmsg_state == bc_query_net_sscon_ok)
			{
			#if 0
				bc_udp_send_enable();
			#endif
				sUdpProtocolState = bc_protocol_send_heart;
				sUdpSendState = bc_query_net_sscon;
				//sProtocolTimeout = 1000;		//restart protocol 
			}
			else if(bc_sendmsg_state == bc_send_msg_fail 
					|| bc_sendmsg_state == bc_query_net_sscon_fail
					|| bc_sendmsg_state == bc_get_receive_data_fail)
			{
				sUdpProtocolState = bc_protocol_send_heart;
				sUdpSendState = bc_send_msg_start;
				//sProtocolTimeout = 1000;		//restart protocol 
			}
			break;
		case bc_protocol_idle:
			break;
		default:
			break;
	}
	return sUdpProtocolState;
}


BC_UDPReceiveMsg_STATE protocl_bc95_udp_receive_msg(int socket,char* rbuf,int* rlen, int maxrlen)
{
	BC_SEND_E bc_send_state;
	int read_len = *rlen;
	switch(sUdpReceiveState)
	{
		case bc_receive_msg_start: // get receive content
		{
			char *cmd = "AT+NSORF="; // send command , for receive content 
			if (socket < 0 || NULL == rbuf  || rlen == NULL || maxrlen <= 0)
			{
				break;
			}
			memset(neul_bc95_tmpbuf, 0, NEUL_MAX_BUF_SIZE);
			sprintf(neul_bc95_tmpbuf, "%s%d,%d\r", cmd, socket, read_len);
			sUdpReceiveState = bc_receive_msg_proc;
			bc_send_enable();
			break;
		}
		case bc_receive_msg_proc:	
			bc_send_state = neul_dev.ops->dev_write(neul_bc95_tmpbuf,
													SERVER_HOST_UDP_IP,100);
			if(bc_send_state == bc_send_ack)
			{		
				char str_remote_ip[16] = {2};
				char str_remote_port[6] = {2};
				char str_buf_len[4] = {2};
				char str_remain_len[4] = {2};			
				 int int_remian_len = 0;
				char *string = 0;
				
//1ex: "\r\n+NSONMI:0,5\r\n\r\n0,106.14.171.181,8234,5,1213141516,0\r\n\r\nOK"
//2ex: "\r\n\r0,106.14.171.181.8234,12,FA000001A5AF861122334455,0\r\n\r\nOK\r\n"
//3ex:106.14.171.181.8234,12,FA000001A5AF861122334455,0\r\n\r\nOK\r\n				
// %d	  		整形输出 
// %s 	  		字符串输出	
// %[^ ]  		取到指定字符为止的字符串
// %[1-9a-z]	取仅包含指定字符集的字符串  取仅包含1到9和小写字母的字符串
// %[^A-Z]		取到指定字符集为止的字符串	取遇到大写字母为止的字符串
// %*[^/]/%[^@]	给定一个字符串iios/12DDWDFF@122，获取 / 和 @ 之间的字符串
// %*s%s	   	给定一个字符串"hello,world"，仅保留"world"。（注意：“，”之后有一空格）%*s表示第一个匹配到的%s被过滤掉，
//				即hello被过滤了，如果没有空格则结果为NULL

				memset(neul_bc95_tmpbuf, 0, NEUL_MAX_BUF_SIZE);
				// 保证不出错>>>>>
				string = mystrstr(neul_dev.rbuf,SERVER_HOST_UDP_IP);
				if(string)
				{
					
					sscanf(string,"%[^,],%[^,],%[^,],%[^,],%[^\r]\r\n\r\nOK\r\n",
					str_remote_ip,str_remote_port,str_buf_len,neul_bc95_tmpbuf,str_remain_len);			
				}
				else
				{
					sUdpReceiveState = bc_receive_msg_null;
					break;
				}
#if 0
	#if 1			// important , don't move
				sscanf(neul_dev.rbuf,"\r\n%[^:]:%[^,],%[^\r]\r\n\r\n%[^,],%[^,],%[^,],%[^,],%[^,],%[^\r]\r\n\r\nOK",
				str_cmd_char,str_cmd_soket,str_cmd_len,str_remote_soket,str_remote_ip,str_remote_port,str_buf_len,neul_bc95_tmpbuf,str_remain_len);
	#else
				sscanf(neul_dev.rbuf,"\r\n\r%[^,],%[^,],%[^,],%[^,],%[^,],%[^\r]\r\n\r\nOK",
				str_remote_soket,str_remote_ip,str_remote_port,str_buf_len,neul_bc95_tmpbuf,str_remain_len);			
	#endif
#endif
				*rlen = StringToInt(str_buf_len);
				if (*rlen > 0)
				{
					StringToHex(neul_bc95_tmpbuf, *rlen * 2, rbuf);					
					sUdpReceiveState = bc_receive_msg_success;
				}
				else 
				{
					sUdpReceiveState = bc_receive_msg_null;
				}
				int_remian_len = StringToInt(str_remain_len);
				if(int_remian_len > 0)
				{
					Uart_Receive_Def* set_param = set_bc95_receive_param();
					set_param->state = RECEIVE_COMPLETE;
					set_param->server_send_len = int_remian_len;
				}
			}
			else if(bc_send_state == bc_send_noack) 
			{			
				sUdpReceiveState = bc_receive_msg_null;
			}
    		break;
		case bc_receive_msg_success:
			break;
		case bc_receive_msg_null:
			break;
		default:
			break;
	}
	return sUdpReceiveState;
}









