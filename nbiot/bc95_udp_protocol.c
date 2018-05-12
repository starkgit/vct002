/**
  ******************************************************************************
  * @file    bc95_udp_protocol.c
  * @author  stark
  * @version V1.0
  * @date    2018-01-20
  * @brief   .C file
  ******************************************************************************
  * @attention
  *  
  * THE PURPOSE OF THE TEAM: 
  * TO WIN THE CREDIBILITY OF THE CUSTOMER, TO WIN THE QUALITY OF LIFE.
  * 
  * Copyright (C), 2018, Shenzhen Viketor Technology Co.,Ltd.
  ******************************************************************************
  * 
  */  

/* Includes ------------------------------------------------------------------*/

#include "bc95_uart.h"
#include "user_lib.h"
#include "bc95_udp_protocol.h"
#include "uart_display.h"
#include "bc95.h"

// param data 
#include "gps_main.h" //gps
#include "bsp_adc.h"
#include "algorithm.h"
#include "stabilizer_types.h"
#include "user_lib.h"
#include "math.h"
#include "g_public.h"
#include "algorithm.h"

#define CMD_LEN	10

const char command_type_table[] = {
	0x00,0x0d,0x09,0x02,0x01,0x08,0x0F,
	0x01,0x04,0x01,0x02,0x01,0x03
};
	
const char command_display_table[][4] =
{
"´ýÃü",	 
"¾ÈÈË",	
"Ãð»ð",	
"ÅÅÏÕ",	
"ÊèÉ¢",	
"ºóÔ®",	
"ÅÊÅÀ",	
"ÄÚ¹¥",	
"Ö§³Å",	
"½µÎÂ",	
"¸ôÀë",	
"ÅÅË®",	
"³·Àë",	// Í¨ÓÃ
"¹¥¼á",	
"Ç±Ë®",	
"·À×¹",	
"·À»¯",	
"·À·ä",	
"·À±¬",	
"·À¶¾",	
"·Àµç",	
"·À»¬",	// ÌØ·À
"Õì²ì",	
"¼ì²â",	// Õì¼ì
"¾¯½ä",	// ¾¯½ä
"Æøµæ",	
"Ë¯´ü",	
"¾ÈÉþ",	
"Æ¤Í§",	
"Á²Ê¬",	
"»º½µ",	
"Å×Í¶",	
"¾ÈÔ®",	// ¾ÈÉú
"ÇÐ¸î",	
"À©ÕÅ",	
"º¸½Ó",	
"³Å¶¥",	
"´ò¿×",	
"¼··ì",	
"ÔäÑÒ",	
"×²ÃÅ",	
"ÅÅÕÏ",	
"ÍÚ¾ò",	
"»ÙËø",	
"°Î¶¤",	
"Æ½Ôä",	
"ÆðÖØ",	
"Ö§³Å",	// ÆÆ²ð
"¶ÂÂ©",	// ¶ÂÂ©
"·â¶¾",	
"³é¶¾",	
"Î§ÓÍ",	
"ÎüÒº",	// Êä×ª
"Ï´Ïû",	// Ï´Ïû
"ÅÅÑÌ",	
"ÕÕÃ÷",	// ÅÅÑÌ
"ÉäË®",	// ÉäË®
"Ê§Õ½",	
"ÓöÏÕ",	
"ÓöÕÏ",	// ÍÑË®
};


// variable 
#if 0
NBProtocolTypedef NBProtocol = 
{
	{0x00,0x01},
	{0xff,0xff},
	0,  // address
	0,  
	0,
	0,
};
#endif
NBProtocolTypedef NBProtocol;

void protocol_data_initial(void)
{
//	printf(" 0x1ffff7e8 id:0x%X\n",(*(volatile uint32_t *)0x1ffff7e8));
//	printf(" 0x1ffff7ec id:0x%X\n",(*(volatile uint32_t *)0x1ffff7ec));
//	printf(" 0x1ffff7f0 id:0x%X\n",(*(volatile uint32_t *)0x1ffff7f0));
	uint32_t uid = (*(volatile uint32_t *)0x1ffff7e8);
	NBProtocol.cloud_address = 0x00;

	NBProtocol.device_address[3] = LOBYTE(uid);
	NBProtocol.device_address[2] = HIBYTE(uid);
	NBProtocol.device_address[1] = HI_LOBYTE(uid);
	NBProtocol.device_address[0] = HI_HIBYTE(uid);
}


void protocol_master_send_sos(NBProtocolSOS_E sel)
{
	unsigned char i = 0;
	unsigned short crcval;
	char buf[20];
	i = 0;
	buf[i++] = 0xFA;
	buf[i++] = NBProtocol.cloud_address;
	buf[i++] = NBProtocol.device_address[0];
	buf[i++] = NBProtocol.device_address[1];
	buf[i++] = NBProtocol.device_address[2];
	buf[i++] = NBProtocol.device_address[3];
	buf[i++] = 0xA4;
	buf[i++] = 0x00;
	if(sel == E_SOS_OK)
	{
		buf[i++] = 0x01;
	}
	else if(sel == E_SOS_CANCEL)
	{
		buf[i++] = 0x02;
	}
	else if(sel == E_SOS_DEL)
	{
		buf[i++] = 0x03;
	}
	crcval = math_crc16(0,buf,i);
	buf[i++] = HIBYTE(crcval);
	buf[i++] = LOBYTE(crcval);
	bc95_master_send(buf,i);
}

void protocol_master_send_demand(unsigned char type,unsigned char cmd)
{
	unsigned char i = 0;
	unsigned short crcval;
	char buf[20];
	i = 0;
	buf[i++] = 0xFA;
	buf[i++] = NBProtocol.cloud_address;
	buf[i++] = NBProtocol.device_address[0];
	buf[i++] = NBProtocol.device_address[1];
	buf[i++] = NBProtocol.device_address[2];
	buf[i++] = NBProtocol.device_address[3];
	buf[i++] = 0xA6;
	buf[i++] = type;
	buf[i++] = cmd;
	crcval = math_crc16(0,buf,i);
	buf[i++] = HIBYTE(crcval);
	buf[i++] = LOBYTE(crcval);
	bc95_master_send(buf,i);
}

void protocol_master_send_setup(unsigned char cmd)
{
	unsigned char i = 0;
	unsigned short crcval;
	char buf[20];
	i = 0;
	buf[i++] = 0xFA;
	buf[i++] = NBProtocol.cloud_address;
	buf[i++] = NBProtocol.device_address[0];
	buf[i++] = NBProtocol.device_address[1];
	buf[i++] = NBProtocol.device_address[2];
	buf[i++] = NBProtocol.device_address[3];
	buf[i++] = 0xFD;
	buf[i++] = cmd;
	crcval = math_crc16(0,buf,i);
	buf[i++] = HIBYTE(crcval);
	buf[i++] = LOBYTE(crcval);
	bc95_master_send(buf,i);
}

/* ============================================================
func name   :  protocol part
=============================================================== */
void protocol_data_proc(char *rbuf,int* rlen)
{
	unsigned char cmd_type;
//	unsigned int device_add;
	unsigned char len = *rlen;
	 char *receive_buf = rbuf;
	if(len < 6)	return ;  // data + crc len
	if(math_crc16(0,receive_buf,len) != 0) return;
	
	
	if(receive_buf[0] == 0xFA	// head 
		&& receive_buf[1] == NBProtocol.device_address[0]  
		&& receive_buf[2] == NBProtocol.device_address[1] 
		&& receive_buf[3] == NBProtocol.device_address[2] 
		&& receive_buf[4] == NBProtocol.device_address[3] 
		&& receive_buf[5] == NBProtocol.cloud_address 
		)
	{
		cmd_type = receive_buf[6];
		switch(cmd_type)
		{
			case 0xA1:// ????????
			
				break;
			case 0xA2:// ????????(GPS)
				protocol_gps_instructions(receive_buf,rlen);
				break;
			case 0xA3:// ??????????(?????)
				protocol_action_instructions(receive_buf,rlen);
				break;
			case 0xA4:// ?? ????
				protocol_combat_instructions(receive_buf,rlen);
				break;
			case 0xA5:// ????????
				protocol_battery_instructions(receive_buf,rlen);
				break;
			case 0xC1:
				protocol_device_name(receive_buf,rlen);
				break;
			default:
				break;
		}
	}
}
void protocol_gps_instructions(char *rbuf,int *rlen)
{
	unsigned char i = 0;
#if 0
	unsigned char j=0 ,error = 0;
#endif
	unsigned short crcval;
	char *buf = rbuf;
//	char info[10];	
	
#if 0
	// for test gps data
	GPS.NS = 'N';
	GPS.EW = 'E';
	GPS.latitude_Degree = 31;
	GPS.latitude_Cent = 10;
	GPS.latitude_Second = 51;
	
	GPS.longitude_Degree = 121;
	GPS.longitude_Cent = 32;
	GPS.longitude_Second = 53;
#endif
	
	i = 0;
	buf[i++] = 0xFA;
	buf[i++] = NBProtocol.cloud_address;
	buf[i++] = NBProtocol.device_address[0];
	buf[i++] = NBProtocol.device_address[1];
	buf[i++] = NBProtocol.device_address[2];
	buf[i++] = NBProtocol.device_address[3];
	buf[i++] = 0xA2;
	
#if 1
	buf[i++] = 0X01;

	buf[i++] = 31;
	buf[i++] = 0XDD;
	buf[i++] = 10;
	buf[i++] = 0XFF;
	buf[i++] = 51;
	buf[i++] = 0XAA;

	buf[i++] = 0X02;
	
	buf[i++] = 121;
	buf[i++] = 0XDD;
	buf[i++] = 32;
	buf[i++] = 0XFF;
	buf[i++] = 53;
	buf[i++] = 0XAA;
#else
	// Î³¶È
	if (GPS.NS == 'N')  {
		error = 0;
		buf[i++] = 0X01;
	}
	else if(GPS.NS == 'S'){
		error = 0;
		buf[i++] = 0X02;
	}else{
		buf[i++] = 0X00;
		error = 1;
	}
	//Êý¾Ý
	if(error == 0){ 
		Int_To_Str(GPS.latitude_Degree,info);  //
		j = 0;
		while(info[j] != '\0'){
			buf[i++] = info[j++];
		}
		buf[i++] = 0XDD;
		
		Int_To_Str(GPS.latitude_Cent,info);  
		j = 0;
		while(info[j] != '\0'){
			buf[i++] = info[j++];
		}
		buf[i++] = 0XFF;
		
		Int_To_Str(GPS.latitude_Second,info);  
		j = 0;
		while(info[j] != '\0'){
			buf[i++] = info[j++];
		}
		buf[i++] = 0XAA;
	}


	// ¾­¶È
	if(GPS.EW == 'E'){
		error = 0;
		buf[i++] = 0X03;
	}else if(GPS.EW == 'W'){
		error = 0;
		buf[i++] = 0X04;
	}else{
		buf[i++] = 0X00;
		error = 1;
	}
	// Êý¾Ý
	if(error == 0){
		Int_To_Str(GPS.longitude_Degree,info);  
		j = 0;
		while(info[j] != '\0'){
			buf[i++] = info[j++];
		}
		buf[i++] = 0XDD;


		Int_To_Str(GPS.longitude_Cent,info);  
		j = 0;
		while(info[j] != '\0'){
			buf[i++] = info[j++];
		}
		buf[i++] = 0XFF;

		
		Int_To_Str(GPS.longitude_Second,info);   
		j = 0;
		while(info[j] != '\0'){
			buf[i++] = info[j++];
		}
		buf[i++] = 0XAA;
		
	}
#endif
	crcval = math_crc16(0,buf,i);
	buf[i++] = HIBYTE(crcval);
	buf[i++] = LOBYTE(crcval);
	*rlen = i;

}


void protocol_action_instructions(char *rbuf,int *rlen)
{
	unsigned char i = 0 ;
	unsigned short crcval;
	char *buf = rbuf;
	unsigned char cmd_buf = rbuf[5];

	buf[i++] = 0xFA;
	buf[i++] = NBProtocol.cloud_address;
	buf[i++] = NBProtocol.device_address[0];
	buf[i++] = NBProtocol.device_address[1];
	buf[i++] = NBProtocol.device_address[2];
	buf[i++] = NBProtocol.device_address[3];
	buf[i++] = 0xA3;
	buf[i++] = cmd_buf;
	
	buf[i++] = 0;	// ??
	buf[i++] = 0;
	buf[i++] = 1;	// ??
	
	crcval = math_crc16(0,buf,i);
	buf[i++] = HIBYTE(crcval);
	buf[i++] = LOBYTE(crcval);

	*rlen = i;
}

// FA 06 73 FF 53 00 | A4 01 01 A0 03 CRC16
// 00 01 02 03 04 05 | 06 07 08 09 10 11
void protocol_combat_instructions(char *rbuf,int *rlen){

	unsigned char i = 0 ;
	unsigned char j;
	unsigned short crcval;
	unsigned char len = *rlen;
	char *buf = rbuf;
	unsigned char cmd_pos = 0;
	unsigned char cmd_display = rbuf[6];	// A4 Ö¸Áî½çÃæ
	Combat_Receive_Type = rbuf[7]; 			// 01 Ö¸ÁîÀàÐÍ
	Combat_Receive_Content = rbuf[8]; 		// 01 Ö¸ÁîÄÚÈÝ
	Combat_Receive_InOut = rbuf[9];			// A0 ÊÒÄÚÊÒÍâ
	Combat_Receive_Floor = rbuf[10];		// 03 3Â¥

	
	if(	   (cmd_display != 0xA4)
		|| (Combat_Receive_Type == 0x00 || Combat_Receive_Type > 0x0c) 
		|| (Combat_Receive_Content == 0x00) 
		|| (len < 10)){	
		return;
	}
	NB_Combat_Receive_Flag = 1;
	
	NBProtocol.count %= 4;	
	NBProtocol.count++; // 1 ~ 4

	// remember 
	NBProtocol.cmd_save_protocol[NBProtocol.count - 1][0] = Combat_Receive_Type;
	NBProtocol.cmd_save_protocol[NBProtocol.count - 1][1] = Combat_Receive_Content;
	NBProtocol.cmd_save_protocol[NBProtocol.count - 1][2] = Combat_Receive_InOut;
	NBProtocol.cmd_save_protocol[NBProtocol.count - 1][3] = Combat_Receive_Floor;

	NBProtocol.cmd_count[NBProtocol.count - 1] = 1;

	for(j = 0; j < Combat_Receive_Type; j++){
		cmd_pos += command_type_table[Combat_Receive_Type - 1];	
	}
	cmd_pos += Combat_Receive_Content;
	
	for(j = 0; j < 4; j++){
		NBProtocol.cmd_content[NBProtocol.count - 1][j] = command_display_table[cmd_pos - 1][j];
	}
	
	NBProtocol.cmd_floor[NBProtocol.count - 1] = Combat_Receive_Floor;
	NBProtocol.cmd_report[NBProtocol.count - 1] = 0; 		// clear flag
	display_manage_request(DISPLAY_CMD);
	 

	buf[i++] = 0xFA;
	buf[i++] = NBProtocol.cloud_address;
	buf[i++] = NBProtocol.device_address[0];
	buf[i++] = NBProtocol.device_address[1];
	buf[i++] = NBProtocol.device_address[2];
	buf[i++] = NBProtocol.device_address[3];
	buf[i++] = 0xA4;
	buf[i++] = Combat_Receive_Type;
	buf[i++] = Combat_Receive_Content;   // <<<<<
	buf[i++] = Combat_Receive_InOut; 	 // 
	buf[i++] = Combat_Receive_Floor;	 // <<<<<
	buf[i++] = 0xFF;
	crcval = math_crc16(0,buf,i);
	buf[i++] = HIBYTE(crcval);
	buf[i++] = LOBYTE(crcval);
	
	*rlen = i;
}

void protocol_battery_instructions(char *rbuf,int *rlen)
{
	unsigned char i = 0 ;
	unsigned short crcval;
	char *buf = rbuf;
	
	buf[i++] = 0xFA;
	buf[i++] = NBProtocol.cloud_address;
	buf[i++] = NBProtocol.device_address[0];
	buf[i++] = NBProtocol.device_address[1];
	buf[i++] = NBProtocol.device_address[2];
	buf[i++] = NBProtocol.device_address[3];
	buf[i++] = 0xA5;
	buf[i++] = battery_percent[0];
	buf[i++] = battery_percent[1];
	crcval = math_crc16(0,buf,i);
	buf[i++] = HIBYTE(crcval);
	buf[i++] = LOBYTE(crcval);
	
	*rlen = i;
}

void protocol_device_name(char *rbuf,int *rlen)
{
	unsigned char i = 0 ;
	unsigned short crcval;
	char *buf = rbuf;
	
	buf[i++] = 0xFA;
	buf[i++] = NBProtocol.cloud_address;
	buf[i++] = NBProtocol.device_address[0];
	buf[i++] = NBProtocol.device_address[1];
	buf[i++] = NBProtocol.device_address[2];
	buf[i++] = NBProtocol.device_address[3];
	buf[i++] = 0xC1;
	
	// ³¤¶È
	buf[i++] = 0x03;

	// "ÕÅÃÈÃÈ"×Ö·û
	buf[i++] = 0xD5;
	buf[i++] = 0xC5;
	buf[i++] = 0xC3;
	buf[i++] = 0xC8;
	buf[i++] = 0xC3;
	buf[i++] = 0xC8;	
	crcval = math_crc16(0,buf,i);
	buf[i++] = HIBYTE(crcval);
	buf[i++] = LOBYTE(crcval);
	
	*rlen = i;
}



























