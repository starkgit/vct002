#include "system.h"
#include "vct_main.h"
#include "bc95.h"
#include "uart_display.h"
#include "key.h"
//#include "neul_bc95_main.h"


void vctTask(void* param)
{
	u32 tick = 0;
	u32 lastWakeTime = getSysTickCnt();

//	bc95Init();
	while(1) 
	{
		vTaskDelayUntil(&lastWakeTime, MAIN_LOOP_DT);		/*1ms÷‹∆⁄—” ±*/


		powermanager(tick);
		
		bc95Task(tick);

		uart_displayTask(tick);

		KeyTask(tick);
		
		tick++;
	}

}


















