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

#ifndef __GPS_MAIN_H__
#define __GPS_MAIN_H__
#include "stm32f10x.h"
#include "sys.h"
#include "gps_data_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#define GPS_EN 		PAout(15)	


extern GPS_INFO   GPS;



extern void GPSTask(void* param);
extern void gps_port_init(void);
extern void gps_init(void);
extern void gps_main(void);












#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __VCT_MAIN_H__ */

