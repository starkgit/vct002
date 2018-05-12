/**
  ******************************************************************************
  * @file    bc95.h
  * @author  stark
  * @version V1.0
  * @date    2017-11-20
  * @brief   bc95.c header file
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

#ifndef __BC95_H
#define __BC95_H
#include "stm32f10x.h"
#include "stabilizer_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef struct
{
	char manufacture_id[12];
	char device_module[18];
	char firmware_version[30];
	char frequency_band[10];
} BC95_UE_INFO_typedef;


typedef enum{
  bc_state_variable_init,
  bc_state_power_on,
  bc_state_net_query,
  bc_state_create_upd,
  bc_state_work,
  
}BC_TASK_STATE;



typedef enum{
  bc_send_start,
  bc_send_check,
  bc_send_ack_wait,
  bc_send_noack_wait,
  bc_send_ack,
  bc_send_noack,
}BC_SEND_E;


typedef enum{
	bc_power_on_start,

	bc_power_on_send_at,
	bc_power_on_noack_at,
	bc_power_on_ack_at,

	bc_power_on_send_query_autoconnect_false,
	bc_power_on_noack_autoconnect_false,
	bc_power_on_ack_autoconnect_false,

	bc_power_on_send_set_autoconnect_true,
	bc_power_on_noack_autoconnect_true,
	bc_power_on_ack_autoconnect_true,

	bc_power_on_send_query_defaultband,
	bc_power_on_send_set_defaultband,

	bc_power_on_send_reboot ,
	bc_power_on_noack_reboot,
	bc_power_on_ack_reboot,

	bc_power_on_wait_neul,
	
	bc_power_on_normal,
	bc_power_on_error,
  
}BC_POWER_ON_STATE;


typedef enum{
  	bc_query_net_start,  
	bc_query_sim_card,
	bc_query_sim_signal,

	bc_query_net_attched_state,	
  	bc_query_net_register_state,


	bc_query_net_sccon_state,
	
	bc_query_net_attched_register_success,
}BC_QUERY_NET_STATE;

typedef enum{
	bc_create_udp_start,
	bc_create_udp_proc,
	bc_create_udp_success,
	bc_create_udp_fail,
}BC_CreateUDP_STATE;



typedef enum{
	bc_send_msg_start,
	bc_send_msg_proc,	
	bc_send_msg_success,
	bc_send_msg_fail,

	bc_query_net_sscon,

	bc_query_net_sscon_ok,
	bc_query_net_sscon_fail,

	bc_check_receive_data,
	bc_get_receive_data_len, // is ok
	bc_get_receive_data_fail,// is fail
}BC_UDPSendMsg_STATE;

typedef enum{
	bc_receive_msg_start,
	bc_receive_msg_proc,	
	bc_receive_msg_null,
	bc_receive_msg_success,
}BC_UDPReceiveMsg_STATE;



typedef enum{
	bc_protocol_start,
	bc_protocol_send_heart_wait,
	bc_protocol_send_heart,
	bc_protocol_send_wait,
	bc_protocol_receive_start,
	bc_protocol_receive_wait,
	bc_protocol_send_ack,

	
	bc_protocol_idle,
}BC_UDPProtocolMsg_STATE;


typedef BC_SEND_E (*neul_read)(char *cmd,char *ack,uint16_t timeout);
typedef BC_SEND_E (*neul_write)(char *cmd, char *ack,uint16_t timeout);

typedef struct _neul_dev_operation_t 
{
    neul_read dev_read;
    neul_write dev_write;
} neul_dev_operation_t;



typedef struct _remote_info_t
{
    unsigned short port;
    int socket;
    char ip[16];
}remote_info;


typedef struct _neul_dev_info_t 
{
    char *rbuf;
    char *wbuf;
    int remotecount;
    remote_info *addrinfo;
    neul_dev_operation_t *ops;
    char cdpip[16];
} neul_dev_info_t;



extern void bc95Task( const u32 tick);
extern uint8_t* bc95_check_ack(char *str);
extern BC_POWER_ON_STATE bc95_power_on_handing(void);
extern BC_QUERY_NET_STATE bc95_query_net_status(void);
extern BC_SEND_E bc95_receive_cmd_handing(char *cmd,char *ack,uint16_t WAITCOUNT);
extern BC_SEND_E bc95_send_cmd_handing(char *cmd,char *ack,uint16_t timeout);
extern void bc95_task_process(void);
extern void bc95_master_send(char *send_buf,int len);
extern void bc95_master_send_protocol_command(unsigned char param, unsigned char keyval);

extern BC_CreateUDP_STATE neul_bc95_create_udpsocket(unsigned short localport);
extern int neul_bc95_set_upd_param(const char *rip, const unsigned short port, const int socket);
extern BC_UDPReceiveMsg_STATE protocl_bc95_udp_receive_msg(int socket,char* rbuf,int* rlen, int maxrlen);
extern BC_UDPSendMsg_STATE protocl_bc95_udp_send_msg(int socket, const char *buf, int sendlen);
extern BC_UDPProtocolMsg_STATE protocl_bc95_udp_task(void);






#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */



#endif

