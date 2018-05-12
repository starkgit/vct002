#include <string.h>
#include "sys.h"
#include "config.h"
#include "uart_syslink.h"
#include "debug_assert.h"
#include "config.h"

/*FreeRtos includes*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"


#define UARTSLK_DATA_TIMEOUT_MS 	1000
#define UARTSLK_DATA_TIMEOUT_TICKS (UARTSLK_DATA_TIMEOUT_MS / portTICK_RATE_MS)
#define CCR_ENABLE_SET  ((u32)0x00000001)

static bool isInit = false;

static xSemaphoreHandle waitUntilSendDone;
static xSemaphoreHandle uartBusy;
static xQueueHandle uartslkDataDelivery;

static u8 dmaBuffer[64];
static u8 *outDataIsr;
static u8 dataIndexIsr;
static u8 dataSizeIsr;
static bool    isUartDmaInitialized;
static DMA_InitTypeDef DMA_InitStructure;
static u32 initialDMACount;
static u32 remainingDMACount;
static bool     dmaIsPaused;

static void uartslkPauseDma(void);
static void uartslkResumeDma(void);
#if 0
/*配置串口DMA*/
void uartslkDmaInit(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	

	/* USART TX DMA 通道配置*/
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&UARTSLK_TYPE->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)dmaBuffer;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
	DMA_InitStructure.DMA_Channel = UARTSLK_DMA_CH;

	NVIC_InitStructure.NVIC_IRQChannel = UARTSLK_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	isUartDmaInitialized = true;
}
#endif
void uartslkInit(void)	/*串口初始化*/
{
	waitUntilSendDone = xSemaphoreCreateBinary(); 	/*等待发送完成 二值信号量*/
	uartBusy = xSemaphoreCreateBinary();			/*串口忙 二值信号量*/
	xSemaphoreGive(uartBusy); 
	
	uartslkDataDelivery = xQueueCreate(1024, sizeof(u8));	/*队列 1024个消息*/
	ASSERT(uartslkDataDelivery);

	// 配置USART 
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	/* 使能GPIO 和 UART 时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_USART1, ENABLE);

	/* μú12?￡o′ò?aGPIOoíUSART2??tμ?ê±?ó */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* μú22?￡o??USART Txμ?GPIO?????aí?íì?′ó??￡ê? */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* μú32?￡o??USART Rxμ?GPIO?????a????ê?è??￡ê?
		óéóúCPU?′??oó￡?GPIOè±ê???ê?????ê?è??￡ê?￡?òò′??????a??2??è2?ê?±?D?μ?
		μ?ê?￡??ò?1ê??¨òé?óé?±?óú???á￡?2￠?ò·à?1???üμ?·?DT??á??a???ú??μ?éè??2?êy
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*  μú32?ò??-×?á?￡?òò′??a2??éò?2?×?
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* μú42?￡o ????′??úó2?t2?êy */
	USART_InitStructure.USART_BaudRate = UART2_BAUD;	/* 2¨ì??ê */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;		/* ???????óê??￡ê? */
	USART_Init(USART2, &USART_InitStructure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	/* ê1?ü?óê??D?? */
	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		×￠òa: 2?òa?ú′?′|′ò?a·￠?í?D??
		·￠?í?D??ê1?ü?úSendUart()oˉêy′ò?a
	*/
	USART_Cmd(USART2, ENABLE);		/* ê1?ü′??ú */

	/* CPUμ?D?è±?Y￡o′??ú????o?￡?è?1??±?óSend￡??òμú1??×??ú·￠?í2?3?è￥
		è???ó????a??μú1??×??ú?T·¨?yè··￠?í3?è￥μ??êìa */
	USART_ClearFlag(USART2, USART_FLAG_TC);     /* ??·￠?ííê3é±ê??￡?Transmission Complete flag */

	/*配置串口非空中断*/
	NVIC_InitStructure.NVIC_IRQChannel = UARTSLK_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	
	USART_Cmd(UARTSLK_TYPE, ENABLE);	/*使能定时器*/




	
	isInit = true;
}

bool uartslkTest(void)
{
	return isInit;
}
/*从接收队列读取数据(带超时处理)*/
bool uartslkGetDataWithTimout(u8 *c)
{
	/*接收uartslkDataDelivery(1024个容量)消息*/
	if (xQueueReceive(uartslkDataDelivery, c, UARTSLK_DATA_TIMEOUT_TICKS) == pdTRUE)	
	{
		return true;
	}
	*c = 0;
	return false;
}
/*发送原始数据*/
void uartslkSendData(u32 size, u8* data)
{
	u32 i;

	if (!isInit) return;

	for(i = 0; i < size; i++)
	{
	#ifdef UARTSLK_SPINLOOP_FLOWCTRL
		while(GPIO_ReadInputDataBit(UARTSLK_TXEN_PORT, UARTSLK_TXEN_PIN) == Bit_SET);
	#endif
		while (!(UARTSLK_TYPE->SR & USART_FLAG_TXE))
		{};
		UARTSLK_TYPE->DR = (data[i] & 0x00FF);
	}
}
/*中断方式发送原始数据*/
void uartslkSendDataIsrBlocking(u32 size, u8* data)
{
	xSemaphoreTake(uartBusy, portMAX_DELAY);
	outDataIsr = data;
	dataSizeIsr = size;
	dataIndexIsr = 1;
	uartslkSendData(1, &data[0]);
	USART_ITConfig(UARTSLK_TYPE, USART_IT_TXE, ENABLE);	/*串口发送数据寄存器为空中断*/
	xSemaphoreTake(waitUntilSendDone, portMAX_DELAY);
	outDataIsr = 0;
	xSemaphoreGive(uartBusy);
}
/*发送一个字符到串口*/
int uartslkPutchar(int ch)
{
    uartslkSendData(1, (u8 *)&ch);
    
    return (u8)ch;
}
/*通过DMA发送原始数据*/
void uartslkSendDataDmaBlocking(u32 size, u8* data)
{
	if (isUartDmaInitialized)
	{
		xSemaphoreTake(uartBusy, portMAX_DELAY);
		while(DMA_GetCmdStatus(UARTSLK_DMA_STREAM) != DISABLE);	/*等待DMA空闲*/
		memcpy(dmaBuffer, data, size);		/*复制数据到DMA缓冲区*/
		DMA_InitStructure.DMA_BufferSize = size;
		initialDMACount = size;
		DMA_Init(UARTSLK_DMA_STREAM, &DMA_InitStructure);	/*重新初始化DMA数据流*/
		DMA_ITConfig(UARTSLK_DMA_STREAM, DMA_IT_TC, ENABLE);/*开启DMA传输完成中断*/		
		USART_DMACmd(UARTSLK_TYPE, USART_DMAReq_Tx, ENABLE);/* 使能USART DMA TX请求 */
		USART_ClearFlag(UARTSLK_TYPE, USART_FLAG_TC);		/* 清除传输完成中断标志位 */
		DMA_Cmd(UARTSLK_DMA_STREAM, ENABLE);	/* 使能DMA USART TX数据流 */
		xSemaphoreTake(waitUntilSendDone, portMAX_DELAY);
		xSemaphoreGive(uartBusy);
	}
}
/*暂停DMA传输*/
static void uartslkPauseDma()
{
	if (DMA_GetCmdStatus(UARTSLK_DMA_STREAM) == ENABLE)
	{
		DMA_ITConfig(UARTSLK_DMA_STREAM, DMA_IT_TC, DISABLE);	/*关闭DMA传输完成中断*/	
		DMA_Cmd(UARTSLK_DMA_STREAM, DISABLE);
		while(DMA_GetCmdStatus(UARTSLK_DMA_STREAM) != DISABLE);
		DMA_ClearITPendingBit(UARTSLK_DMA_STREAM, UARTSLK_DMA_IT_TCIF);
		remainingDMACount = DMA_GetCurrDataCounter(UARTSLK_DMA_STREAM);
		dmaIsPaused = true;
	}
}
/*恢复DMA传输*/
static void uartslkResumeDma()
{
	if (dmaIsPaused)
	{
		DMA_SetCurrDataCounter(UARTSLK_DMA_STREAM, remainingDMACount);	/*更新DMA计数器*/
		UARTSLK_DMA_STREAM->M0AR = (u32)&dmaBuffer[initialDMACount - remainingDMACount];/*更新内存读取地址*/
		DMA_ITConfig(UARTSLK_DMA_STREAM, DMA_IT_TC, ENABLE);	/*开启DMA传输完成中断*/	
		USART_ClearFlag(UARTSLK_TYPE, USART_FLAG_TC);	/* 清除传输完成中断标志位 */
		DMA_Cmd(UARTSLK_DMA_STREAM, ENABLE);	/* 使能DMA USART TX数据流 */
		dmaIsPaused = false;
	}
}

void uartslkDmaIsr(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	DMA_ITConfig(UARTSLK_DMA_STREAM, DMA_IT_TC, DISABLE);
	DMA_ClearITPendingBit(UARTSLK_DMA_STREAM, UARTSLK_DMA_IT_TCIF);
	USART_DMACmd(UARTSLK_TYPE, USART_DMAReq_Tx, DISABLE);
	DMA_Cmd(UARTSLK_DMA_STREAM, DISABLE);

	remainingDMACount = 0;
	xSemaphoreGiveFromISR(waitUntilSendDone, &xHigherPriorityTaskWoken);
}

void uartslkIsr(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if ((UARTSLK_TYPE->SR & (1<<5)) != 0) /*接收非空中断*/
	{
		u8 rxDataInterrupt = (u8)(UARTSLK_TYPE->DR & 0xFF);
		xQueueSendFromISR(uartslkDataDelivery, &rxDataInterrupt, &xHigherPriorityTaskWoken);
	}
	else if (USART_GetITStatus(UARTSLK_TYPE, USART_IT_TXE) == SET)
	{
		if (outDataIsr && (dataIndexIsr < dataSizeIsr))
		{
			USART_SendData(UARTSLK_TYPE, outDataIsr[dataIndexIsr] & 0x00FF);
			dataIndexIsr++;
		} else
		{
			USART_ITConfig(UARTSLK_TYPE, USART_IT_TXE, DISABLE);
			xSemaphoreGiveFromISR(waitUntilSendDone, &xHigherPriorityTaskWoken);
		}
	}
}

void uartslkTxenFlowctrlIsr()
{
	EXTI_ClearFlag(UARTSLK_TXEN_EXTI);
	if (GPIO_ReadInputDataBit(UARTSLK_TXEN_PORT, UARTSLK_TXEN_PIN) == Bit_SET)
	{
		uartslkPauseDma();
		//ledSet(LED_GREEN_R, 1);
	}
	else
	{
		uartslkResumeDma();
		//ledSet(LED_GREEN_R, 0);
	}
}

void __attribute__((used)) EXTI0_Callback(void)
{
	uartslkTxenFlowctrlIsr();
}

void __attribute__((used)) USART2_IRQHandler(void)	
{
	uartslkIsr();
}

void __attribute__((used)) DMA1_Stream6_IRQHandler(void)	
{
	uartslkDmaIsr();
}

