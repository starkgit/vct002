/**
  ******************************************************************************
  * @file    vct_main.h
  * @author  stark
  * @version V1.0
  * @date    2017-11-21
  * @brief   vct_main.c header file
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

#ifndef __GPS_DATA_PROC_H__
#define __GPS_DATA_PROC_H__
#include "stm32f10x.h"
#include "sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#define GPRS_CH 		0x00
#define GPS_CH  		0x01
#define Bluetooth_CH 	0x02
#define IrDA_CH 		0x03

#define UARTBUFFER 		 _NONCACHE_STARTADDRESS
#define TX_INTTYPE 1	//0:Tx interrupt type is pulse, 1:Tx interrupt type is level
#define RX_INTTYPE 1	//0:Rx interrupt type is pulse, 1:Rx interrupt type is level

typedef struct{
	int year;  
	int month; 
	int  day;
	int hour;
	int minute;
	int second;
}DATE_TIME;

typedef struct{
	double  latitude;  //����
	double  longitude; //γ��
	int     latitude_Degree;	//��
	int		latitude_Cent;		//��
	int   	latitude_Second;    //��
	int     longitude_Degree;	//��
	int		longitude_Cent;		//��
	int   	longitude_Second;   //��
	int     satellite;
	uint8_t 	NS;
	uint8_t 	EW;
	DATE_TIME D;
}GPS_INFO;

int GPS_RMC_Parse(char *line, GPS_INFO *GPS);
int GPS_GSV_Parse(char *line, GPS_INFO *GPS);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __VCT_MAIN_H__ */

