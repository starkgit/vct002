#ifndef __FILE_BSP_ADC_H__
#define __FILE_BSP_ADC_H__



extern void bsp_InitAdc(void);
extern void batteryTask(void* param);



//extern unsigned short int After_filter[2]; //用来存放求平均值之后的结果
extern char battery_percent[2];



//unsigned int GetNTCVoltage(void);
#endif // __FILE_BSP_BATTERY_H__














