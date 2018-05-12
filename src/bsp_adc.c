#include "bsp_adc.h"
#include "system.h"
#include "math.h"

// 18650 电压范围 2.7V ~ 4.2V  正常工作将电池电压最低设置在 3.2V
// MCU基准电压为	   : Vref = 3.0V
// 电池电压的变化范围为: 3.2V ~ 4.2V
// 到MCU AD口电压为:     1.6V ~ 2.1V
// 到MCU AD口数值为:	 2185 ~ 2867(Y)
// AD值得变化范围为: 	 0 ~ 100(X)  X = (Y-2048)/8.19
#define AD_VAL_MAX 2867 
#define AD_VAL_MIN 2185
#define AD_CH 		2
#define AD_CNT	50
vu16 AD_Value[AD_CNT][AD_CH]; //用来存放ADC转换结果，也是DMA的目标地址  为 2个通道  每通道采50次
char battery_percent[2];
float f_ad[2];	// for test

void bsp_InitAdc(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure; 
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1	, ENABLE );	  //使能ADC1通道时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);

	// port init
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				// ADC12_IN7	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_InitStructure);				// ADC12_IN8
	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_DeInit(ADC1); //将外设 ADC1 的全部寄存器重设为缺省值
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode =ENABLE; //模数转换工作在扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; //模数转换工作在连续转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //外部触发转换关闭
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = AD_CH; //顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure); //根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器

	//设置指定ADC的规则组通道，设置它们的转化顺序和采样时间
	//ADC1,ADC通道x,规则采样顺序值为y,采样时间为239.5周期
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5 );
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5 );
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_239Cycles5 );
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_239Cycles5 );
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 6, ADC_SampleTime_239Cycles5 );
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 7, ADC_SampleTime_239Cycles5 );
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 8, ADC_SampleTime_239Cycles5 );
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 9, ADC_SampleTime_239Cycles5 );
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 10, ADC_SampleTime_239Cycles5 );
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 11, ADC_SampleTime_239Cycles5 );
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 12, ADC_SampleTime_239Cycles5 );
	
	// 开启ADC的DMA支持（要实现DMA功能，还需独立配置DMA通道等参数）
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE); //使能指定的ADC1
	ADC_ResetCalibration(ADC1); //复位指定的ADC1的校准寄存器
	while(ADC_GetResetCalibrationStatus(ADC1)); //获取ADC1复位校准寄存器的状态,设置状态则等待

	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel1); //将DMA的通道1寄存器重设为缺省值 
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR; //DMA外设ADC基地址  
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value; //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //内存作为数据传输的目的地
	DMA_InitStructure.DMA_BufferSize = AD_CNT*AD_CH; //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //工作在循环缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA通道 x拥有高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure); //根据DMA_InitStruct中指定的参数初始化DMA的通道
}

float Fileter_LPF_1st(float oldData, float newData, float lpf_factor)
{
	return oldData * (1 - lpf_factor) + newData * lpf_factor;
}

void GetADValue(void)
{
	static unsigned short int old_value[AD_CH] = {0};
	unsigned short int current_val[AD_CH]; //用来存放求平均值之后的结果
	float temp_precent[2];
	int sum = 0;
	u8 count , i;
	for(i=0; i < AD_CH; i++)
	{
		for (count = 0; count < AD_CNT; count++)
		{
			sum += AD_Value[count][i];
		}
		current_val[i] = sum/AD_CNT;
		current_val[i] = Fileter_LPF_1st(old_value[i] ,current_val[i] , 0.8 );
		old_value[i] = current_val[i];
		f_ad[i] = (float)current_val[i]/4096 * 3.0;  // test
		sum=0;
	}

	// Get percent 
	if(current_val[0] > AD_VAL_MAX)
	{
		temp_precent[0] = 1.00;
	}
	else if(current_val[0] < AD_VAL_MIN)
	{
		temp_precent[0] = 0.00;
	}
	else
	{
		temp_precent[0] = (float)(current_val[0] - AD_VAL_MIN) / 819;
	}

	
	if(current_val[1] > AD_VAL_MAX)
	{
		temp_precent[1] = 1.00;
	}
	else if(current_val[1] < AD_VAL_MIN)
	{
		temp_precent[1] = 0.00;
	}
	else
	{
		temp_precent[1] = (float)(current_val[1] - AD_VAL_MIN) / 819;
	}
	battery_percent[0] = temp_precent[0] * 100;
	battery_percent[1] = temp_precent[1] * 100;
} 



void batteryTask(void* param)
{
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	DMA_Cmd(DMA1_Channel1, ENABLE);
	while(1)
	{
		GetADValue();
		vTaskDelay(100);
	}
}









