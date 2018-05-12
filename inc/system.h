#ifndef __SYSTEM_H
#define __SYSTEM_H

/* freertos 配置文件 */
#include "FreeRTOSConfig.h"

/*FreeRTOS相关头文件*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/* Project includes */
#include "config.h"
#include "nvic.h"


#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* public */
#include "user_lib.h"
#include "g_typedef.h"
#include "g_public.h"


/*底层硬件驱动*/
#include "sys.h"
#include "delay.h"
//#include "usart.h"	  
#include "uart_fifo.h"
#include "led.h"
#include "power.h"
#include "bsp_adc.h"

// usb 
#include "hw_config.h"
#include "usblink.h"


#include "sensors.h"
#include "comm.h"
#include "i2c_gpio_1.h"
#include "stabilizer.h"
#include "atkp.h"
#include "comm.h"
#include "bsp_adc.h"
#include "key.h"




















void systemInit(void);














#endif /* __SYSTEM_H */

