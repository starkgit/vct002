#include "system.h"
#include "stdbool.h"

/********************************************************************/
/*	[PORT A]			[STANDBY IN] 	[SYSTEM INIT]				*/
/* -----------------------------------------------------------------*/
/*	15: PA15  											O/L					*/
/*	14: PA14  							 				O/L					*/
/*	13: PA13  											O/L					*/
/*	12: PA12  	USB_DP		Micro_USB					O/L		  			*/
/*	11: PA11  	USB_DM		Micro_USB					O/L 				*/
/*	10: PA10  	USART1_RX 	GPS_TX	(VK2527)			I/L					*/
/*	 9: PA9	  	USART1_TX   GPS_RX	(VK2527)			O/L					*/
/*	 8: PA8	  											O/L					*/		
/*	 7: PA7	  					 						O/L					*/
/*	 6: PA6	  											O/L					*/
/*	 5: PA5	  											O/L					*/
/*	 4: PA4	    		   	IMU_INT	(MPU9250)			O/L		  			*/
/*	 3: PA3	  	USART2_RX  	NRF_TX	(NRF51822)			I/L 				*/
/*	 2: PA2	  	USART2_TX  	NRF_RX	(NRF51822)			O/L					*/
/*	 1: PA1	  				 							O/L					*/
/*	 0: PA0	  	WAKE_UP	(Key)							O/L					*/
/********************************************************************/
/********************************************************************/
/*	[PORT B]			[STANDBY IN] 	[SYSTEM INIT]				*/
/* -----------------------------------------------------------------*/
/*	15: PB15  					 						O/L					*/
/*	14: PB14  							 				O/L					*/
/*	13: PB13  											O/L					*/
/*	12: PB12  											O/L		  			*/
/*	11: PB11  	USART3_RX  	NBI_TX	(NBIOT_SIM)			I/L 				*/
/*	10: PB10  	USART3_TX 	NBI_RX	(NBIOT_SIM)			O/L					*/
/*	 9: PB9	  	IIC_SDA		IMU_SDA	(MPU9250)			O/L					*/
/*	 8: PB8	  	IIC_SCL		IMU_SCL	(MPU9250)			O/L					*/		
/*	 7: PB7	  					 						O/L					*/
/*	 6: PB6	  											O/L					*/
/*	 5: PB5	  											O/L					*/
/*	 4: PB4	  						 					O/L		  			*/
/*	 3: PB3	  											O/L 				*/
/*	 2: PB2	  											O/L					*/
/*	 1: PB1	  											O/L					*/
/*	 0: PB0	   				 							O/L					*/


bool systemTest(void);


/*底层硬件初始化*/
void systemInit(void)
{
	bool Initstate;
	
	nvicInit();			/*中断配置初始化*/
	PowerInit();		// port init
	
	delay_init(72);	    				 
//	uart_init(115200);	
	UartInit();
	
	 
	LedInit();
	KeyInit();
	bsp_InitAdc();
	
	commInit();			/*通信初始化  STM32 & NRF51822 */
	atkpInit();			/*传输协议初始化*/


	stabilizerInit();

	Initstate = systemTest();
	
	if( Initstate == true)
	{
		// Init OK
		
	}
	else
	{

	}
	
//	watchdogInit(WATCHDOG_RESET_MS);	/*看门狗初始化*/	
}





bool systemTest(void)
{
	bool pass = true;
	
//	pass &= ledseqTest();
//	pass &= pmTest();
//	pass &= configParamTest();
//	pass &= commTest();
	pass &= stabilizerTest();	
//	pass &= watchdogTest();
	
	return pass;
}

