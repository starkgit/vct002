/*
*********************************************************************************************************
*
*	模块名称 : 串口中断+FIFO驱动模块
*	文件名称 : uart_fifo.c
*	版    本 : V1.0
*	说    明 : 采用串口中断+FIFO模式实现多个串口的同时访问
*********************************************************************************************************
*/
#include "uart_fifo.h"
#include "sys.h"
#include "string.h"
#include "bc95_uart.h"

#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOS使用	  
#endif

/* 定义每个串口结构体变量 */
#if UART1_FIFO_EN == 1
static UART1_DEF uart1;
#endif

#if UART2_FIFO_EN == 1
static UART2_DEF uart2;
#endif

#if UART3_FIFO_EN == 1
static BC95_UART3_DEF uart3;
#endif


// user add
char rev_buf[80]; //接收缓存
uint8_t num = 0;
bool rev_stop,isRxInt; //停止接收,接受中断


static void UartVarInit(void);
static void InitHardUart(void);
static void ConfigUartNVIC(void);


/*
*********************************************************************************************************
*	函 数 名: bsp_InitUart
*	功能说明: 初始化串口硬件，并对全局变量赋初值.
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void UartInit(void)
{
	UartVarInit();		/* 必须先初始化全局变量,再配置硬件 */

	InitHardUart();		/* 配置串口的硬件参数(波特率等) */

	ConfigUartNVIC();	/* 配置串口中断 */
}


/*
*********************************************************************************************************
*	函 数 名: ComToUart
*	功能说明: 将COM端口号转换为UART指针
*	形    参: _ucPort: 端口号(COM1 - COM6)
*	返 回 值: uart指针
*********************************************************************************************************
*/
static void UartVarInit(void)
{
#if UART1_FIFO_EN == 1	
	uart1.head_transmit = 0;
    uart1.tail_transmit = 0;
    uart1.head_receive = 0;
    uart1.tail_receive = 0;
#endif

#if UART2_FIFO_EN == 1		
	uart2.head_transmit = 0;
    uart2.tail_transmit = 0;
    uart2.head_receive = 0;
    uart2.tail_receive = 0;
#endif

#if UART3_FIFO_EN == 1	
	uart3.head_transmit = 0;
    uart3.tail_transmit = 0;
    uart3.head_receive = 0;
    uart3.tail_receive = 0;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SetUart1Baud
*	功能说明: 修改UART1波特率
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#if UART1_FIFO_EN == 1	
void Uart1SetBaud(uint32_t _baud)
{
	USART_InitTypeDef USART_InitStructure;

	/* 第2步： 配置串口硬件参数 */
	USART_InitStructure.USART_BaudRate = _baud;	/* 波特率 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
}
#endif

/*
*********************************************************************************************************
*	函 数 名: bsp_SetUart2Baud
*	功能说明: 修改UART2波特率
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#if UART2_FIFO_EN == 1	
void Uart2SetBaud(uint32_t _baud)
{
	USART_InitTypeDef USART_InitStructure;

	/* 第2步： 配置串口硬件参数 */
	USART_InitStructure.USART_BaudRate = _baud;	/* 波特率 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
}
#endif

/*
*********************************************************************************************************
*	函 数 名: bsp_SetUart2Baud
*	功能说明: 修改UART2波特率
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#if UART3_FIFO_EN == 1	
void Uart3SetBaud(uint32_t _baud)
{
	USART_InitTypeDef USART_InitStructure;

	/* 第2步： 配置串口硬件参数 */
	USART_InitStructure.USART_BaudRate = _baud;	/* 波特率 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
}
#endif


/*
*********************************************************************************************************
*	函 数 名: InitHardUart
*	功能说明: 配置串口的硬件参数（波特率，数据位，停止位，起始位，校验位，中断使能）适合于STM32-F4开发板
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitHardUart(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

#if UART1_FIFO_EN == 1		/* 串口1 TX = PA9   RX = PA10 或 TX = PB6   RX = PB7*/

	/* 第1步：打开GPIO和USART部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 第3步：将USART Rx的GPIO配置为浮空输入模式
		由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
		但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* 第4步： 配置串口硬件参数 */
	USART_InitStructure.USART_BaudRate = UART1_BAUD;	/* 波特率 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	/* 使能接收中断 */
    USART_ITConfig(USART1, USART_IT_TC,ENABLE);
	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		注意: 不要在此处打开发送中断
		发送中断使能在SendUart()函数打开
	*/
	USART_Cmd(USART1, ENABLE);		/* 使能串口 */

	/* CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
		如下语句解决第1个字节无法正确发送出去的问题 */
	USART_ClearFlag(USART1, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */

#endif

#if UART2_FIFO_EN == 1		/* 串口2 TX = PA2， RX = PA3  */
	/* 第1步：打开GPIO和USART部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 第3步：将USART Rx的GPIO配置为浮空输入模式
		由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
		但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*  第3步已经做了，因此这步可以不做
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 第4步： 配置串口硬件参数 */
	USART_InitStructure.USART_BaudRate = UART2_BAUD;	/* 波特率 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;		/* 仅选择接收模式 */
	USART_Init(USART2, &USART_InitStructure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	/* 使能接收中断 */
    USART_ITConfig(USART2, USART_IT_TC, ENABLE);

	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		注意: 不要在此处打开发送中断
		发送中断使能在SendUart()函数打开
	*/
	USART_Cmd(USART2, ENABLE);		/* 使能串口 */

	/* CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
		如下语句解决第1个字节无法正确发送出去的问题 */
	USART_ClearFlag(USART2, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */
#endif

#if UART3_FIFO_EN == 1			/* 串口3 TX = PB10   RX = PB11 */
	/* 第1步： 开启GPIO和UART时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* 第3步：将USART Rx的GPIO配置为浮空输入模式
		由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
		但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*  第3步已经做了，因此这步可以不做
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* 第4步： 配置串口硬件参数 */
	USART_InitStructure.USART_BaudRate = UART3_BAUD;	/* 波特率 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);	/* 使能接收中断 */
	USART_ITConfig(USART3, USART_IT_TC,ENABLE);

	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		注意: 不要在此处打开发送中断
		发送中断使能在SendUart()函数打开
	*/
	USART_Cmd(USART3, ENABLE);		/* 使能串口 */

	/* CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
		如下语句解决第1个字节无法正确发送出去的问题 */
	USART_ClearFlag(USART3, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: ConfigUartNVIC
*	功能说明: 配置串口硬件中断.
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ConfigUartNVIC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure the NVIC Preemption Priority Bits */
	/*	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  --- 在 bsp.c 中 bsp_Init() 中配置中断优先级组 */

#if UART1_FIFO_EN == 1
	/* 使能串口1中断 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if UART2_FIFO_EN == 1
	/* 使能串口2中断 */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if UART3_FIFO_EN == 1
	/* 使能串口3中断t */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
}

#if UART1_FIFO_EN == 1
// 获取发送缓冲区空闲空间大小。
int32_t uart1_transmitIdleBufferSize(void)
{
	if(uart1.tail_transmit <= uart1.head_transmit)
		return UART1_BUFFER_SIZE_TRANSMIT - uart1.head_transmit + uart1.tail_transmit;
	return uart1.tail_transmit - uart1.head_transmit - 1; // -1:不可以填满。
}

// 获取发送缓冲区中的数据长度。
int32_t uart1_transmitValidBufferSize(void)
{
	if(uart1.tail_transmit <= uart1.head_transmit)
		return uart1.head_transmit - uart1.tail_transmit;
	return uart1.head_transmit + (UART1_BUFFER_SIZE_TRANSMIT - uart1.tail_transmit);
}

// 从发送缓冲区取出一个字节，发送出去。
// 不检查缓冲区是否空，不检查发送状态。
void uart1_transmitAByte(void)
{
    USART_SendData(USART1,uart1.buffer_transmit[uart1.tail_transmit]);
	uart1.tail_transmit ++;
	if(uart1.tail_transmit >= UART1_BUFFER_SIZE_TRANSMIT)
		uart1.tail_transmit = 0;
}

// 把一段数据放入发送缓冲区。
// 返回值：{UART1_SUCCEED,UART1_FAILED}
uint8_t uart1_transmit(const void * data,uint32_t len)
{
	if(uart1_transmitIdleBufferSize() < len)
	{
		return UART_FAILED; // 空间不够。
	}
	//
	for(int i=0;i<len;i++)
	{
		uart1.buffer_transmit[uart1.head_transmit] = ((uint8_t *)data)[i];
		//
		uart1.head_transmit ++;
		if(uart1.head_transmit >= UART1_BUFFER_SIZE_TRANSMIT)
			uart1.head_transmit = 0;
		//
	}
	//
	// 如果发送没有正在进行，就启动发送。
	if(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == SET)
        uart1_transmitAByte();
	//
	return UART_SUCCEED;
}

// 获取接收缓冲区中的数据长度。
int32_t uart1_receiveValidBufferSize(void)
{
	if(uart1.tail_receive <= uart1.head_receive)
		return uart1.head_receive - uart1.tail_receive;
	return uart1.head_receive + (UART1_BUFFER_SIZE_RECEIVE - uart1.tail_receive);
}

// 把一个字节放入接收缓冲区。
// 缓冲区满则失败。
// 返回值：{UART1_SUCCEED,UART1_FAILED}。
uint8_t uart1_receivePushToBuffer(uint8_t data)
{
    uint32_t newHead = uart1.head_receive + 1;
	if(newHead == UART1_BUFFER_SIZE_RECEIVE)
		newHead = 0;
    //
    if(newHead == uart1.tail_receive)
        return UART_FAILED;
    //
    uart1.buffer_receive[uart1.head_receive] = data;
    uart1.head_receive = newHead;
    //
    return UART_SUCCEED;
}

// 从接收缓冲区取出数据，返回取出的长度。
// 取出的长度为outputBufferLength和uart1_receiveValidBufferSize()的最小者。
int32_t uart1_readReceiveBuffer(void * outputBuffer,int32_t outputBufferLength)
{
    // 计算outputBufferLength和uart1_receiveValidBufferSize()的最小值。
    uint32_t returnLength = uart1_receiveValidBufferSize();
    if(outputBufferLength < returnLength)
        returnLength = outputBufferLength;
    //
    // 复制数据，推进指针。
    for(uint32_t i=0;i<returnLength;i++)
    {
        ((uint8_t *)outputBuffer)[i] = uart1.buffer_receive[uart1.tail_receive];
        //
        uart1.tail_receive ++;
        if(uart1.tail_receive == UART1_BUFFER_SIZE_RECEIVE)
            uart1.tail_receive = 0;
    }
    //
    return returnLength;
}
#endif

#if UART2_FIFO_EN == 1
// 获取发送缓冲区空闲空间大小。
int32_t uart2_transmitIdleBufferSize(void)
{
	if(uart2.tail_transmit <= uart2.head_transmit)
		return UART2_BUFFER_SIZE_TRANSMIT - uart2.head_transmit + uart2.tail_transmit;
	return uart2.tail_transmit - uart2.head_transmit - 1; // -1:不可以填满。
}

// 获取发送缓冲区中的数据长度。
int32_t uart2_transmitValidBufferSize(void)
{
	if(uart2.tail_transmit <= uart2.head_transmit)
		return uart2.head_transmit - uart2.tail_transmit;
	return uart2.head_transmit + (UART2_BUFFER_SIZE_TRANSMIT - uart2.tail_transmit);
}

// 从发送缓冲区取出一个字节，发送出去。
// 不检查缓冲区是否空，不检查发送状态。
void uart2_transmitAByte(void)
{
    USART_SendData(USART2,uart2.buffer_transmit[uart2.tail_transmit]);
	uart2.tail_transmit ++;
	if(uart2.tail_transmit >= UART2_BUFFER_SIZE_TRANSMIT)
		uart2.tail_transmit = 0;
}

// 把一段数据放入发送缓冲区。
// 返回值：{UART1_SUCCEED,UART1_FAILED}
uint8_t uart2_transmit(const void * data,uint32_t len)
{
	if(uart2_transmitIdleBufferSize() < len)
	{
		return UART_FAILED; // 空间不够。
	}
	//
	for(int i=0;i<len;i++)
	{
		uart2.buffer_transmit[uart2.head_transmit] = ((uint8_t *)data)[i];
		//
		uart2.head_transmit ++;
		if(uart2.head_transmit >= UART2_BUFFER_SIZE_TRANSMIT)
			uart2.head_transmit = 0;
		//
	}
	//
	// 如果发送没有正在进行，就启动发送。
	if(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == SET)
        uart2_transmitAByte();
	//
	return UART_SUCCEED;
}

// 获取接收缓冲区中的数据长度。
int32_t uart2_receiveValidBufferSize(void)
{
	if(uart2.tail_receive <= uart2.head_receive)
		return uart2.head_receive - uart2.tail_receive;
	return uart2.head_receive + (UART2_BUFFER_SIZE_RECEIVE - uart2.tail_receive);
}

// 把一个字节放入接收缓冲区。
// 缓冲区满则失败。
// 返回值：{UART1_SUCCEED,UART1_FAILED}。
uint8_t uart2_receivePushToBuffer(uint8_t data)
{
    uint32_t newHead = uart2.head_receive + 1;
	if(newHead == UART2_BUFFER_SIZE_RECEIVE)
		newHead = 0;
    //
    if(newHead == uart2.tail_receive)
        return UART_FAILED;
    //
    uart2.buffer_receive[uart2.head_receive] = data;
    uart2.head_receive = newHead;
    //
    return UART_SUCCEED;
}

// 从接收缓冲区取出数据，返回取出的长度。
// 取出的长度为outputBufferLength和uart1_receiveValidBufferSize()的最小者。
int32_t uart2_readReceiveBuffer(void * outputBuffer,int32_t outputBufferLength)
{
    // 计算outputBufferLength和uart1_receiveValidBufferSize()的最小值。
    uint32_t returnLength = uart2_receiveValidBufferSize();
    if(outputBufferLength < returnLength)
        returnLength = outputBufferLength;
    //
    // 复制数据，推进指针。
    for(uint32_t i=0;i<returnLength;i++)
    {
        ((uint8_t *)outputBuffer)[i] = uart2.buffer_receive[uart2.tail_receive];
        //
        uart2.tail_receive ++;
        if(uart2.tail_receive == UART2_BUFFER_SIZE_RECEIVE)
            uart2.tail_receive = 0;
    }
    //
    return returnLength;
}
#endif


#if UART3_FIFO_EN == 1
// 获取发送缓冲区空闲空间大小。
int32_t uart3_transmitIdleBufferSize(void)
{
	if(uart3.tail_transmit <= uart3.head_transmit)
		return UART3_BUFFER_SIZE_TRANSMIT - uart3.head_transmit + uart3.tail_transmit;
	return uart3.tail_transmit - uart3.head_transmit - 1; // -1:不可以填满。
}

// 获取发送缓冲区中的数据长度。
int32_t uart3_transmitValidBufferSize(void)
{
	if(uart3.tail_transmit <= uart3.head_transmit)
		return uart3.head_transmit - uart3.tail_transmit;
	return uart3.head_transmit + (UART3_BUFFER_SIZE_TRANSMIT - uart3.tail_transmit);
}

// 从发送缓冲区取出一个字节，发送出去。
// 不检查缓冲区是否空，不检查发送状态。
void uart3_transmitAByte(void)
{
    USART_SendData(USART3,uart3.buffer_transmit[uart3.tail_transmit]);
	uart3.tail_transmit ++;
	if(uart3.tail_transmit >= UART3_BUFFER_SIZE_TRANSMIT)
		uart3.tail_transmit = 0;
}

// 把一段数据放入发送缓冲区。
// 返回值：{UART1_SUCCEED,UART1_FAILED}
uint8_t uart3_transmit(const void * data,uint32_t len)
{
	if(uart3_transmitIdleBufferSize() < len)
	{
		return UART_FAILED; // 空间不够。
	}
	//
	for(int i=0;i<len;i++)
	{
		uart3.buffer_transmit[uart3.head_transmit] = ((uint8_t *)data)[i];
		//
		uart3.head_transmit ++;
		if(uart3.head_transmit >= UART3_BUFFER_SIZE_TRANSMIT)
			uart3.head_transmit = 0;
		//
	}
	//
	// 如果发送没有正在进行，就启动发送。
	if(USART_GetFlagStatus(USART3,USART_FLAG_TXE) == SET)
        uart3_transmitAByte();
	//
	return UART_SUCCEED;
}

// 获取接收缓冲区中的数据长度。
int32_t uart3_receiveValidBufferSize(void)
{
	if(uart3.tail_receive <= uart3.head_receive)
		return uart3.head_receive - uart3.tail_receive;
	return uart3.head_receive + (UART3_BUFFER_SIZE_RECEIVE - uart3.tail_receive);
}

// 把一个字节放入接收缓冲区。
// 缓冲区满则失败。
// 返回值：{UART1_SUCCEED,UART1_FAILED}。
uint8_t uart3_receivePushToBuffer(uint8_t data)
{
    uint32_t newHead = uart3.head_receive + 1;
	if(newHead == UART3_BUFFER_SIZE_RECEIVE)
		newHead = 0;
    //
    if(newHead == uart3.tail_receive)
        return UART_FAILED;
    //
    uart3.buffer_receive[uart3.head_receive] = data;
    uart3.head_receive = newHead;
    //
    return UART_SUCCEED;
}

// 从接收缓冲区取出数据，返回取出的长度。
// 取出的长度为outputBufferLength和uart1_receiveValidBufferSize()的最小者。
int32_t uart3_readReceiveBuffer(void * outputBuffer,int32_t outputBufferLength)
{
    // 计算outputBufferLength和uart1_receiveValidBufferSize()的最小值。
    uint32_t returnLength = uart3_receiveValidBufferSize();
    if(outputBufferLength < returnLength)
        returnLength = outputBufferLength;
    //
    // 复制数据，推进指针。
    for(uint32_t i=0;i<returnLength;i++)
    {
        ((uint8_t *)outputBuffer)[i] = uart3.buffer_receive[uart3.tail_receive];
        //
        uart3.tail_receive ++;
        if(uart3.tail_receive == UART3_BUFFER_SIZE_RECEIVE)
            uart3.tail_receive = 0;
    }
    //
    return returnLength;
}
#endif




/*
*********************************************************************************************************
*	函 数 名: USART1_IRQHandler  USART2_IRQHandler USART3_IRQHandler UART4_IRQHandler UART5_IRQHandler
*	功能说明: USART中断服务程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#if UART1_FIFO_EN == 1
void USART1_IRQHandler(void)
{
	static unsigned char gps_receive_status = 0;
	static unsigned char gps_find_head[6] ;
    // 发送完成。
    if(USART_GetFlagStatus(USART1,USART_FLAG_TC) == SET)
    {
        USART_ClearFlag(USART1,USART_FLAG_TC);
        //
        if(uart1_transmitValidBufferSize() > 0)
            uart1_transmitAByte();
    }
    //
    // 接收到数据。
    if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == SET)
    {
		USART_ClearFlag(USART1,USART_FLAG_RXNE);
		uint8_t ch = (uint8_t)USART_ReceiveData(USART1);	//

		switch(gps_receive_status)
		{
			case 0:
				if (ch == '$')  //如果收到字符'$'，便开始接收
				{
					gps_receive_status++;
					num = 0;
					gps_find_head[num++] = ch ;
				}
				break;
			case 1:
				gps_find_head[num++] = ch ;
				if(num >= sizeof(gps_find_head)) 
				{
					if(gps_find_head[1] == 'G' 
					&& gps_find_head[2] == 'N' 
					&& gps_find_head[3] == 'R' 
					&& gps_find_head[4] == 'M' 
					&& gps_find_head[5] == 'C' 
					)
					{
						memcpy(rev_buf,gps_find_head,sizeof(gps_find_head));
						gps_receive_status++;
					}
					else
					{
						gps_receive_status = 0;
					}
				}
				break;
			case 2:
				rev_buf[num++] = ch ;
				if(num >= sizeof(rev_buf))
				{
					num = 0;
					gps_receive_status = 0;
					break;
				}
				if (ch == '\n') 		//如果接收到换行
				{
					rev_buf[num] = '\0';
					rev_stop  = 1;
					num = 0;
					gps_receive_status = 0;
				}
				break;
			default:
				break;

		}
		isRxInt = 1;
    }

}
#endif

#if UART2_FIFO_EN == 1
void USART2_IRQHandler(void)
{
 // 发送完成。
    if(USART_GetFlagStatus(USART2,USART_FLAG_TC) == SET)
    {
        USART_ClearFlag(USART2,USART_FLAG_TC);
        //
        if(uart2_transmitValidBufferSize() > 0)
            uart2_transmitAByte();
    }
    //
    // 接收到数据。
    if(USART_GetFlagStatus(USART2,USART_FLAG_RXNE) == SET)
    {
        USART_ClearFlag(USART2,USART_FLAG_RXNE);
        //
        uint8_t data = (uint8_t)USART_ReceiveData(USART2);
        //
        uart2_receivePushToBuffer(data);
    }
}
#endif

#if UART3_FIFO_EN == 1
void USART3_IRQHandler(void)
{
 /* usRxWrite ±?á??ú?D??oˉêy?D±???D′￡??÷3ìDò?áè???±?á?ê±￡?±?D???DDáù????±￡?¤ */
 // 发送完成。
    if(USART_GetFlagStatus(USART3,USART_FLAG_TC) == SET)
    {
        USART_ClearFlag(USART3,USART_FLAG_TC);
        //
        if(uart3_transmitValidBufferSize() > 0)
            uart3_transmitAByte();
    }
    //
    // 接收到数据。
    if(USART_GetFlagStatus(USART3,USART_FLAG_RXNE) == SET)
    {
        USART_ClearFlag(USART3,USART_FLAG_RXNE);
        //
        uint8_t data = (uint8_t)USART_ReceiveData(USART3);
        //
        uart3_receivePushToBuffer(data);

		bc95_get_server_data_len(data);
    }
}
#endif


//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
#if 1 // for uart  printf
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
#else //for usb printf
	extern int consolePutchar(int ch);
	consolePutchar(ch); /*打印到上位机*/
	return ch;
#endif
}
#endif 



