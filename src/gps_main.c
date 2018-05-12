#include "gps_main.h"
#include "system.h"


GPS_INFO   GPS;
unsigned char error_num = 0;
unsigned int connect_num = 0;

void GPSTask(void* param)
{
	u32 lastWakeTime = getSysTickCnt();

	gps_port_init();
	gps_init();
	
	while(1) 
	{
		vTaskDelayUntil(&lastWakeTime, MAIN_LOOP_DT);		/*1ms周期延时*/
		gps_main();	
	}
}

void gps_port_init(void)
{
//	int32_t len = 0;
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);					
	GPIO_SetBits(GPIOA,GPIO_Pin_15);
}

void gps_init(void)
{
	vTaskDelay(200);
	if (GPS_GSV_Parse(rev_buf, &GPS))
	{
	//	Uart_Printf("可用卫星数量: %2d\r", GPS->satellite);
	}
}




//========================================================================
//GPS数据显示
//========================================================================
void GPS_Display(void)
{
#if 0
	Uart_Printf("%4d年%02d月%02d日  %02d:%02d:%02d\n", GPS.D.year, GPS.D.month, GPS.D.day, GPS.D.hour, GPS.D.minute, GPS.D.second);

	if ((GPS.NS == 'N') && (GPS.EW == 'E'))  //北纬，东经
		Uart_Printf("北纬：%2d° %2d＇%2d＂  东经：：%2d° %2d＇%2d＂\n", GPS.latitude_Degree, GPS.latitude_Cent, GPS.latitude_Second, GPS.longitude_Degree, GPS.longitude_Cent, GPS.longitude_Second);
	else if ((GPS.NS == 'N') && (GPS.EW == 'W')) //北纬，西经
		Uart_Printf("北纬：%2d° %2d＇%2d＂  西经：：%2d° %2d＇%2d＂\n", GPS.latitude_Degree, GPS.latitude_Cent, GPS.latitude_Second, GPS.longitude_Degree, GPS.longitude_Cent, GPS.longitude_Second);
	else if ((GPS.NS == 'S') && (GPS.EW == 'E')) //南纬，东经
		Uart_Printf("南纬：%2d° %2d＇%2d＂  东经：：%2d° %2d＇%2d＂\n", GPS.latitude_Degree, GPS.latitude_Cent, GPS.latitude_Second, GPS.longitude_Degree, GPS.longitude_Cent, GPS.longitude_Second);
	else if ((GPS.NS == 'S') && (GPS.EW == 'W')) //南纬，西经
		Uart_Printf("南纬：%2d° %2d＇%2d＂  东经：：%2d° %2d＇%2d＂\n", GPS.latitude_Degree, GPS.latitude_Cent, GPS.latitude_Second, GPS.longitude_Degree, GPS.longitude_Cent, GPS.longitude_Second);
#endif
}

void gps_main(void)
{
	if(isRxInt == 1)  //如果接收到数据
	{
		isRxInt = 0;
		if (rev_stop)  //如果接收完一行
		{
			rev_stop  = 0;
			if (GPS_RMC_Parse(rev_buf, &GPS)) //解析GPRMC
			{
				
			}
		}
	}
}







