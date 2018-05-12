#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"


#define KEY_UP  		GPIO_Pin_4
#define KEY_DN  		GPIO_Pin_6
#define KEY_OK   		GPIO_Pin_5


#define READ_KEY_UP()  		GPIO_ReadInputDataBit(GPIOB,KEY_UP)	 
#define READ_KEY_DN() 		GPIO_ReadInputDataBit(GPIOB,KEY_DN)	 
#define READ_KEY_OK() 		GPIO_ReadInputDataBit(GPIOB,KEY_OK)	 

enum
{	
	MSG_NONE				= 0x0000,	
 	MSG_UP	,			
 	MSG_DN	,			
 	MSG_OK	,			
};
//IO≥ı ºªØ
void KeyInit(void);

void KeyTask(const u32 tick);


		 				    
#endif
