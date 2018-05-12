#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"


#define KEY_UP_PIN 		GPIO_Pin_4
#define KEY_DN_PIN 		GPIO_Pin_6
#define KEY_OK_PIN		GPIO_Pin_5
#define KEY_MENU_PORT	GPIOB


#define READ_KEY_UP()  		GPIO_ReadInputDataBit(KEY_MENU_PORT,KEY_UP_PIN)	 
#define READ_KEY_DN() 		GPIO_ReadInputDataBit(KEY_MENU_PORT,KEY_DN_PIN)	 
#define READ_KEY_OK() 		GPIO_ReadInputDataBit(KEY_MENU_PORT,KEY_OK_PIN)	


#define KEY_POWER_PIN		GPIO_Pin_4
#define KEY_POWER_PORT		GPIOD
#define READ_POWER_DET()	GPIO_ReadInputDataBit(GPIOD,KEY_POWER_PIN)


enum
{
	E_KEY_POWER,
	E_KEY_UP,
	E_KEY_DN,
	E_KEY_OK,

	
	E_KEY_NONE = 0XFF,
};



enum
{	
	MSG_NONE				= 0x0000,	
	MSG_POWER_SHORT,
	MSG_POWER_LONG,
 	MSG_UP	,			
 	MSG_DN	,			
 	MSG_OK	,			
};


//IO≥ı ºªØ
void KeyInit(void);

void KeyTask(const u32 tick);


		 				    
#endif
