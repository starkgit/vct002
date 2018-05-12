#ifndef __BC95_UDP_PROTOCOL_H__
#define __BC95_UDP_PROTOCOL_H__

#include "g_typedef.h"


#define PROTOCOL_CODE_TYPE_CMD  		0X01  // server -> device
#define PROTOCOL_CODE_TYPE_DECMD		0X02  // device -> server
#define PROTOCOL_CODE_TYPE_TOTAL		0X03



#define CMD_TYPE_CURRENCY				0X01
#define CMD_TYPE_SPECIAL_PROTECTION		0X02
#define CMD_TYPE_DETECTION				0X03
#define CMD_TYPE_ALERT					0X04
#define CMD_TYPE_LIFESAVING				0X05
#define CMD_TYPE_RESCUE					0X06
#define CMD_TYPE_PLUGGING				0X07
#define CMD_TYPE_TRANSFER				0X08
#define CMD_TYPE_DECONTAMINATION		0X09
#define CMD_TYPE_LIGHTING_EXHAUSTING	0X0A
#define CMD_TYPE_WATER_JET				0X0B
#define CMD_TYPE_ASK_FIGHT				0X0C
#define CMD_TYPE_LEAVE 					0X0D
#define CMD_TYPE_TOTAL					0X0E

#define CMD_NUM_STANDBY					0x01
#define CMD_NUM_RESCUE_PERSION			0x02
#define CMD_NUM_OUTFIRE                 0x03
#define CMD_NUM_DETECT_RISK             0x04
#define CMD_NUM_DISPERSED               0x05
#define CMD_NUM_SUPPORT                 0x06
#define CMD_NUM_CLIMB                   0x07
#define CMD_NUM_ATTACK                  0x08
#define CMD_NUM_BRACE_COM               0x09
#define CMD_NUM_COOLING                 0x0A
#define CMD_NUM_QUARANTINE              0x0B
#define CMD_NUM_DRAINAGE                0x0C
#define CMD_NUM_EVACUATE                0x0D

#define CMD_NUM_ASSAULT                 0x01
#define CMD_NUM_DIVING                  0x02
#define CMD_NUM_ANTI_FALLING            0x03
#define CMD_NUM_ANTICHEMICAL            0x04
#define CMD_NUM_ANTI_BEE                0x05
#define CMD_NUM_EXPLOSION_PROOF         0x06
#define CMD_NUM_ANTIVIRUS               0x07
#define CMD_NUM_ANTI_ELECTRIC           0x08
#define CMD_NUM_NON_SLIP                0x09

#define CMD_NUM_SCOUT                   0x01
#define CMD_NUM_TESTING                 0x02

#define CMD_NUM_ALERT                   0x01

#define CMD_NUM_AIR_CUSHION             0x01
#define CMD_NUM_SLEEPING_BAG            0x02
#define CMD_NUM_LIFESAVING_ROPE         0x03
#define CMD_NUM_RUBBER_YACHT            0x04
#define CMD_NUM_CONVERGENCE             0x05
#define CMD_NUM_SLOW_DOWN               0x06
#define CMD_NUM_CAST                    0x07
#define CMD_NUM_RESCUE                  0x08

#define CMD_NUM_CUTTING                 0x01
#define CMD_NUM_EXPAND                  0x02
#define CMD_NUM_WELDING                 0x03
#define CMD_NUM_HOLD_THE_TOP            0x04
#define CMD_NUM_PUNCH                   0x05
#define CMD_NUM_CALKING                 0x06
#define CMD_NUM_DRILLING                0x07
#define CMD_NUM_HIT_THE_DOOR            0x08
#define CMD_NUM_TROUBLESHOOTING         0x09
#define CMD_NUM_EXCAVATE                0x0A
#define CMD_NUM_DESTROY_THE_LOCK        0x0B
#define CMD_NUM_PULL_OUT_A_NAIL         0x0C
#define CMD_NUM_FLAT_CHISEL             0x0D
#define CMD_NUM_CRANE                   0x0E
#define CMD_NUM_BRACE                   0x0F

#define CMD_NUM_PLUGGING                0x01

#define CMD_NUM_A_POISON                0x01
#define CMD_NUM_SMOKE_POISON            0x02
#define CMD_NUM_THE_OIL                 0x03
#define CMD_NUM_IMBIBITION              0x04

#define CMD_NUM_DECONTAMINATION         0x01

#define CMD_NUM_SMOKE                   0x01
#define CMD_NUM_LIGHTING                0x02

#define CMD_NUM_WATER_JET               0x01

#define CMD_NUM_ASK_FOR_A_BATTLE        0x01

#define CMD_NUM_LOSE_THE_POWER_OF_WAR   0x01
#define CMD_NUM_DISTRESS                0x02
#define CMD_NUM_OBSTACLE                0x03

//type
#define TONG_YONG 		0X01
//cmd
#define TY_STANDBY		0X01
#define TY_RESCUE		0X02
#define TY_OUTFIRE		0X03
#define TY_DETECTRISK	0X04



typedef enum
{
	E_SOS_OK,
	E_SOS_CANCEL,
	E_SOS_DEL,
}NBProtocolSOS_E;

typedef struct
{
	unsigned char device_address[4];
	unsigned char device_address_bk[2];
	unsigned char cloud_address;
	//receive 
	APP_BTF0_7 combat_variable;
	unsigned char cmd_protocol[4]; // 01 01 A0 03 (Í¨ÓÃ-´ýÃü-ÊÒÍâ-03Â¥)
	unsigned char cmd_save_protocol[4][4];  
	unsigned char count;
	unsigned char cmd_count[4];
	char cmd_content[4][4];
	unsigned char cmd_floor[4];


	//send
	unsigned char cmd_recount;
	unsigned char cmd_report[4];
}NBProtocolTypedef;


#define NB_Combat_Receive_Flag NBProtocol.combat_variable.btf0

// 
#define Combat_Receive_Type		NBProtocol.cmd_protocol[0]
#define Combat_Receive_Content	NBProtocol.cmd_protocol[1]
#define Combat_Receive_InOut	NBProtocol.cmd_protocol[2]
#define Combat_Receive_Floor	NBProtocol.cmd_protocol[3]


extern NBProtocolTypedef NBProtocol;

extern void protocol_data_initial(void);
extern void protocol_gps_instructions(char *rbuf,int *rlen);
extern void protocol_action_instructions(char *rbuf,int *rlen);
extern void protocol_battery_instructions(char *rbuf,int *rlen);
extern void protocol_device_name(char *rbuf,int *rlen);

extern void protocol_combat_instructions(char *rbuf,int *rlen);
extern void protocol_data_proc(char *rbuf,int* rlen);


extern void protocol_master_send_sos(NBProtocolSOS_E sel);
extern void protocol_master_send_demand(unsigned char type,unsigned char cmd);
extern void protocol_master_send_setup(unsigned char cmd);


#endif

