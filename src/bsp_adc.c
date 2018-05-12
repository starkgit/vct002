#include "bsp_adc.h"
#include "system.h"
#include "math.h"

// 18650 ��ѹ��Χ 2.7V ~ 4.2V  ������������ص�ѹ��������� 3.2V
// MCU��׼��ѹΪ	   : Vref = 3.0V
// ��ص�ѹ�ı仯��ΧΪ: 3.2V ~ 4.2V
// ��MCU AD�ڵ�ѹΪ:     1.6V ~ 2.1V
// ��MCU AD����ֵΪ:	 2185 ~ 2867(Y)
// ADֵ�ñ仯��ΧΪ: 	 0 ~ 100(X)  X = (Y-2048)/8.19
#define AD_VAL_MAX 2867 
#define AD_VAL_MIN 2185
#define AD_CH 		2
#define AD_CNT	50
vu16 AD_Value[AD_CNT][AD_CH]; //�������ADCת�������Ҳ��DMA��Ŀ���ַ  Ϊ 2��ͨ��  ÿͨ����50��
char battery_percent[2];
float f_ad[2];	// for test

void bsp_InitAdc(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure; 
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1	, ENABLE );	  //ʹ��ADC1ͨ��ʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);

	// port init
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				// ADC12_IN7	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_InitStructure);				// ADC12_IN8
	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_DeInit(ADC1); //������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode =ENABLE; //ģ��ת��������ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; //ģ��ת������������ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //�ⲿ����ת���ر�
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = AD_CH; //˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure); //����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���

	//����ָ��ADC�Ĺ�����ͨ�����������ǵ�ת��˳��Ͳ���ʱ��
	//ADC1,ADCͨ��x,�������˳��ֵΪy,����ʱ��Ϊ239.5����
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
	
	// ����ADC��DMA֧�֣�Ҫʵ��DMA���ܣ������������DMAͨ���Ȳ�����
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE); //ʹ��ָ����ADC1
	ADC_ResetCalibration(ADC1); //��λָ����ADC1��У׼�Ĵ���
	while(ADC_GetResetCalibrationStatus(ADC1)); //��ȡADC1��λУ׼�Ĵ�����״̬,����״̬��ȴ�

	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel1); //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ 
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR; //DMA����ADC����ַ  
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value; //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //�ڴ���Ϊ���ݴ����Ŀ�ĵ�
	DMA_InitStructure.DMA_BufferSize = AD_CNT*AD_CH; //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //������ѭ������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ�� xӵ�и����ȼ�
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel1, &DMA_InitStructure); //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��
}

float Fileter_LPF_1st(float oldData, float newData, float lpf_factor)
{
	return oldData * (1 - lpf_factor) + newData * lpf_factor;
}

void GetADValue(void)
{
	static unsigned short int old_value[AD_CH] = {0};
	unsigned short int current_val[AD_CH]; //���������ƽ��ֵ֮��Ľ��
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









