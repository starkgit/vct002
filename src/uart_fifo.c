/*
*********************************************************************************************************
*
*	ģ������ : �����ж�+FIFO����ģ��
*	�ļ����� : uart_fifo.c
*	��    �� : V1.0
*	˵    �� : ���ô����ж�+FIFOģʽʵ�ֶ�����ڵ�ͬʱ����
*********************************************************************************************************
*/
#include "uart_fifo.h"
#include "sys.h"
#include "string.h"
#include "bc95_uart.h"

#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOSʹ��	  
#endif

/* ����ÿ�����ڽṹ����� */
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
char rev_buf[80]; //���ջ���
uint8_t num = 0;
bool rev_stop,isRxInt; //ֹͣ����,�����ж�


static void UartVarInit(void);
static void InitHardUart(void);
static void ConfigUartNVIC(void);


/*
*********************************************************************************************************
*	�� �� ��: bsp_InitUart
*	����˵��: ��ʼ������Ӳ��������ȫ�ֱ�������ֵ.
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void UartInit(void)
{
	UartVarInit();		/* �����ȳ�ʼ��ȫ�ֱ���,������Ӳ�� */

	InitHardUart();		/* ���ô��ڵ�Ӳ������(�����ʵ�) */

	ConfigUartNVIC();	/* ���ô����ж� */
}


/*
*********************************************************************************************************
*	�� �� ��: ComToUart
*	����˵��: ��COM�˿ں�ת��ΪUARTָ��
*	��    ��: _ucPort: �˿ں�(COM1 - COM6)
*	�� �� ֵ: uartָ��
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
*	�� �� ��: bsp_SetUart1Baud
*	����˵��: �޸�UART1������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
#if UART1_FIFO_EN == 1	
void Uart1SetBaud(uint32_t _baud)
{
	USART_InitTypeDef USART_InitStructure;

	/* ��2���� ���ô���Ӳ������ */
	USART_InitStructure.USART_BaudRate = _baud;	/* ������ */
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
*	�� �� ��: bsp_SetUart2Baud
*	����˵��: �޸�UART2������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
#if UART2_FIFO_EN == 1	
void Uart2SetBaud(uint32_t _baud)
{
	USART_InitTypeDef USART_InitStructure;

	/* ��2���� ���ô���Ӳ������ */
	USART_InitStructure.USART_BaudRate = _baud;	/* ������ */
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
*	�� �� ��: bsp_SetUart2Baud
*	����˵��: �޸�UART2������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
#if UART3_FIFO_EN == 1	
void Uart3SetBaud(uint32_t _baud)
{
	USART_InitTypeDef USART_InitStructure;

	/* ��2���� ���ô���Ӳ������ */
	USART_InitStructure.USART_BaudRate = _baud;	/* ������ */
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
*	�� �� ��: InitHardUart
*	����˵��: ���ô��ڵ�Ӳ�������������ʣ�����λ��ֹͣλ����ʼλ��У��λ���ж�ʹ�ܣ��ʺ���STM32-F4������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void InitHardUart(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

#if UART1_FIFO_EN == 1		/* ����1 TX = PA9   RX = PA10 �� TX = PB6   RX = PB7*/

	/* ��1������GPIO��USART������ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* ��2������USART Tx��GPIO����Ϊ���츴��ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* ��3������USART Rx��GPIO����Ϊ��������ģʽ
		����CPU��λ��GPIOȱʡ���Ǹ�������ģʽ���������������費�Ǳ����
		���ǣ��һ��ǽ�����ϱ����Ķ������ҷ�ֹ�����ط��޸���������ߵ����ò���
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* ��4���� ���ô���Ӳ������ */
	USART_InitStructure.USART_BaudRate = UART1_BAUD;	/* ������ */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	/* ʹ�ܽ����ж� */
    USART_ITConfig(USART1, USART_IT_TC,ENABLE);
	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		ע��: ��Ҫ�ڴ˴��򿪷����ж�
		�����ж�ʹ����SendUart()������
	*/
	USART_Cmd(USART1, ENABLE);		/* ʹ�ܴ��� */

	/* CPU��Сȱ�ݣ��������úã����ֱ��Send�����1���ֽڷ��Ͳ���ȥ
		�����������1���ֽ��޷���ȷ���ͳ�ȥ������ */
	USART_ClearFlag(USART1, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */

#endif

#if UART2_FIFO_EN == 1		/* ����2 TX = PA2�� RX = PA3  */
	/* ��1������GPIO��USART������ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* ��2������USART Tx��GPIO����Ϊ���츴��ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* ��3������USART Rx��GPIO����Ϊ��������ģʽ
		����CPU��λ��GPIOȱʡ���Ǹ�������ģʽ���������������費�Ǳ����
		���ǣ��һ��ǽ�����ϱ����Ķ������ҷ�ֹ�����ط��޸���������ߵ����ò���
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*  ��3���Ѿ����ˣ�����ⲽ���Բ���
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* ��4���� ���ô���Ӳ������ */
	USART_InitStructure.USART_BaudRate = UART2_BAUD;	/* ������ */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;		/* ��ѡ�����ģʽ */
	USART_Init(USART2, &USART_InitStructure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	/* ʹ�ܽ����ж� */
    USART_ITConfig(USART2, USART_IT_TC, ENABLE);

	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		ע��: ��Ҫ�ڴ˴��򿪷����ж�
		�����ж�ʹ����SendUart()������
	*/
	USART_Cmd(USART2, ENABLE);		/* ʹ�ܴ��� */

	/* CPU��Сȱ�ݣ��������úã����ֱ��Send�����1���ֽڷ��Ͳ���ȥ
		�����������1���ֽ��޷���ȷ���ͳ�ȥ������ */
	USART_ClearFlag(USART2, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */
#endif

#if UART3_FIFO_EN == 1			/* ����3 TX = PB10   RX = PB11 */
	/* ��1���� ����GPIO��UARTʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* ��2������USART Tx��GPIO����Ϊ���츴��ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* ��3������USART Rx��GPIO����Ϊ��������ģʽ
		����CPU��λ��GPIOȱʡ���Ǹ�������ģʽ���������������費�Ǳ����
		���ǣ��һ��ǽ�����ϱ����Ķ������ҷ�ֹ�����ط��޸���������ߵ����ò���
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*  ��3���Ѿ����ˣ�����ⲽ���Բ���
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* ��4���� ���ô���Ӳ������ */
	USART_InitStructure.USART_BaudRate = UART3_BAUD;	/* ������ */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);	/* ʹ�ܽ����ж� */
	USART_ITConfig(USART3, USART_IT_TC,ENABLE);

	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		ע��: ��Ҫ�ڴ˴��򿪷����ж�
		�����ж�ʹ����SendUart()������
	*/
	USART_Cmd(USART3, ENABLE);		/* ʹ�ܴ��� */

	/* CPU��Сȱ�ݣ��������úã����ֱ��Send�����1���ֽڷ��Ͳ���ȥ
		�����������1���ֽ��޷���ȷ���ͳ�ȥ������ */
	USART_ClearFlag(USART3, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: ConfigUartNVIC
*	����˵��: ���ô���Ӳ���ж�.
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ConfigUartNVIC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure the NVIC Preemption Priority Bits */
	/*	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  --- �� bsp.c �� bsp_Init() �������ж����ȼ��� */

#if UART1_FIFO_EN == 1
	/* ʹ�ܴ���1�ж� */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if UART2_FIFO_EN == 1
	/* ʹ�ܴ���2�ж� */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if UART3_FIFO_EN == 1
	/* ʹ�ܴ���3�ж�t */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
}

#if UART1_FIFO_EN == 1
// ��ȡ���ͻ��������пռ��С��
int32_t uart1_transmitIdleBufferSize(void)
{
	if(uart1.tail_transmit <= uart1.head_transmit)
		return UART1_BUFFER_SIZE_TRANSMIT - uart1.head_transmit + uart1.tail_transmit;
	return uart1.tail_transmit - uart1.head_transmit - 1; // -1:������������
}

// ��ȡ���ͻ������е����ݳ��ȡ�
int32_t uart1_transmitValidBufferSize(void)
{
	if(uart1.tail_transmit <= uart1.head_transmit)
		return uart1.head_transmit - uart1.tail_transmit;
	return uart1.head_transmit + (UART1_BUFFER_SIZE_TRANSMIT - uart1.tail_transmit);
}

// �ӷ��ͻ�����ȡ��һ���ֽڣ����ͳ�ȥ��
// ����黺�����Ƿ�գ�����鷢��״̬��
void uart1_transmitAByte(void)
{
    USART_SendData(USART1,uart1.buffer_transmit[uart1.tail_transmit]);
	uart1.tail_transmit ++;
	if(uart1.tail_transmit >= UART1_BUFFER_SIZE_TRANSMIT)
		uart1.tail_transmit = 0;
}

// ��һ�����ݷ��뷢�ͻ�������
// ����ֵ��{UART1_SUCCEED,UART1_FAILED}
uint8_t uart1_transmit(const void * data,uint32_t len)
{
	if(uart1_transmitIdleBufferSize() < len)
	{
		return UART_FAILED; // �ռ䲻����
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
	// �������û�����ڽ��У����������͡�
	if(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == SET)
        uart1_transmitAByte();
	//
	return UART_SUCCEED;
}

// ��ȡ���ջ������е����ݳ��ȡ�
int32_t uart1_receiveValidBufferSize(void)
{
	if(uart1.tail_receive <= uart1.head_receive)
		return uart1.head_receive - uart1.tail_receive;
	return uart1.head_receive + (UART1_BUFFER_SIZE_RECEIVE - uart1.tail_receive);
}

// ��һ���ֽڷ�����ջ�������
// ����������ʧ�ܡ�
// ����ֵ��{UART1_SUCCEED,UART1_FAILED}��
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

// �ӽ��ջ�����ȡ�����ݣ�����ȡ���ĳ��ȡ�
// ȡ���ĳ���ΪoutputBufferLength��uart1_receiveValidBufferSize()����С�ߡ�
int32_t uart1_readReceiveBuffer(void * outputBuffer,int32_t outputBufferLength)
{
    // ����outputBufferLength��uart1_receiveValidBufferSize()����Сֵ��
    uint32_t returnLength = uart1_receiveValidBufferSize();
    if(outputBufferLength < returnLength)
        returnLength = outputBufferLength;
    //
    // �������ݣ��ƽ�ָ�롣
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
// ��ȡ���ͻ��������пռ��С��
int32_t uart2_transmitIdleBufferSize(void)
{
	if(uart2.tail_transmit <= uart2.head_transmit)
		return UART2_BUFFER_SIZE_TRANSMIT - uart2.head_transmit + uart2.tail_transmit;
	return uart2.tail_transmit - uart2.head_transmit - 1; // -1:������������
}

// ��ȡ���ͻ������е����ݳ��ȡ�
int32_t uart2_transmitValidBufferSize(void)
{
	if(uart2.tail_transmit <= uart2.head_transmit)
		return uart2.head_transmit - uart2.tail_transmit;
	return uart2.head_transmit + (UART2_BUFFER_SIZE_TRANSMIT - uart2.tail_transmit);
}

// �ӷ��ͻ�����ȡ��һ���ֽڣ����ͳ�ȥ��
// ����黺�����Ƿ�գ�����鷢��״̬��
void uart2_transmitAByte(void)
{
    USART_SendData(USART2,uart2.buffer_transmit[uart2.tail_transmit]);
	uart2.tail_transmit ++;
	if(uart2.tail_transmit >= UART2_BUFFER_SIZE_TRANSMIT)
		uart2.tail_transmit = 0;
}

// ��һ�����ݷ��뷢�ͻ�������
// ����ֵ��{UART1_SUCCEED,UART1_FAILED}
uint8_t uart2_transmit(const void * data,uint32_t len)
{
	if(uart2_transmitIdleBufferSize() < len)
	{
		return UART_FAILED; // �ռ䲻����
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
	// �������û�����ڽ��У����������͡�
	if(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == SET)
        uart2_transmitAByte();
	//
	return UART_SUCCEED;
}

// ��ȡ���ջ������е����ݳ��ȡ�
int32_t uart2_receiveValidBufferSize(void)
{
	if(uart2.tail_receive <= uart2.head_receive)
		return uart2.head_receive - uart2.tail_receive;
	return uart2.head_receive + (UART2_BUFFER_SIZE_RECEIVE - uart2.tail_receive);
}

// ��һ���ֽڷ�����ջ�������
// ����������ʧ�ܡ�
// ����ֵ��{UART1_SUCCEED,UART1_FAILED}��
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

// �ӽ��ջ�����ȡ�����ݣ�����ȡ���ĳ��ȡ�
// ȡ���ĳ���ΪoutputBufferLength��uart1_receiveValidBufferSize()����С�ߡ�
int32_t uart2_readReceiveBuffer(void * outputBuffer,int32_t outputBufferLength)
{
    // ����outputBufferLength��uart1_receiveValidBufferSize()����Сֵ��
    uint32_t returnLength = uart2_receiveValidBufferSize();
    if(outputBufferLength < returnLength)
        returnLength = outputBufferLength;
    //
    // �������ݣ��ƽ�ָ�롣
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
// ��ȡ���ͻ��������пռ��С��
int32_t uart3_transmitIdleBufferSize(void)
{
	if(uart3.tail_transmit <= uart3.head_transmit)
		return UART3_BUFFER_SIZE_TRANSMIT - uart3.head_transmit + uart3.tail_transmit;
	return uart3.tail_transmit - uart3.head_transmit - 1; // -1:������������
}

// ��ȡ���ͻ������е����ݳ��ȡ�
int32_t uart3_transmitValidBufferSize(void)
{
	if(uart3.tail_transmit <= uart3.head_transmit)
		return uart3.head_transmit - uart3.tail_transmit;
	return uart3.head_transmit + (UART3_BUFFER_SIZE_TRANSMIT - uart3.tail_transmit);
}

// �ӷ��ͻ�����ȡ��һ���ֽڣ����ͳ�ȥ��
// ����黺�����Ƿ�գ�����鷢��״̬��
void uart3_transmitAByte(void)
{
    USART_SendData(USART3,uart3.buffer_transmit[uart3.tail_transmit]);
	uart3.tail_transmit ++;
	if(uart3.tail_transmit >= UART3_BUFFER_SIZE_TRANSMIT)
		uart3.tail_transmit = 0;
}

// ��һ�����ݷ��뷢�ͻ�������
// ����ֵ��{UART1_SUCCEED,UART1_FAILED}
uint8_t uart3_transmit(const void * data,uint32_t len)
{
	if(uart3_transmitIdleBufferSize() < len)
	{
		return UART_FAILED; // �ռ䲻����
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
	// �������û�����ڽ��У����������͡�
	if(USART_GetFlagStatus(USART3,USART_FLAG_TXE) == SET)
        uart3_transmitAByte();
	//
	return UART_SUCCEED;
}

// ��ȡ���ջ������е����ݳ��ȡ�
int32_t uart3_receiveValidBufferSize(void)
{
	if(uart3.tail_receive <= uart3.head_receive)
		return uart3.head_receive - uart3.tail_receive;
	return uart3.head_receive + (UART3_BUFFER_SIZE_RECEIVE - uart3.tail_receive);
}

// ��һ���ֽڷ�����ջ�������
// ����������ʧ�ܡ�
// ����ֵ��{UART1_SUCCEED,UART1_FAILED}��
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

// �ӽ��ջ�����ȡ�����ݣ�����ȡ���ĳ��ȡ�
// ȡ���ĳ���ΪoutputBufferLength��uart1_receiveValidBufferSize()����С�ߡ�
int32_t uart3_readReceiveBuffer(void * outputBuffer,int32_t outputBufferLength)
{
    // ����outputBufferLength��uart1_receiveValidBufferSize()����Сֵ��
    uint32_t returnLength = uart3_receiveValidBufferSize();
    if(outputBufferLength < returnLength)
        returnLength = outputBufferLength;
    //
    // �������ݣ��ƽ�ָ�롣
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
*	�� �� ��: USART1_IRQHandler  USART2_IRQHandler USART3_IRQHandler UART4_IRQHandler UART5_IRQHandler
*	����˵��: USART�жϷ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
#if UART1_FIFO_EN == 1
void USART1_IRQHandler(void)
{
	static unsigned char gps_receive_status = 0;
	static unsigned char gps_find_head[6] ;
    // ������ɡ�
    if(USART_GetFlagStatus(USART1,USART_FLAG_TC) == SET)
    {
        USART_ClearFlag(USART1,USART_FLAG_TC);
        //
        if(uart1_transmitValidBufferSize() > 0)
            uart1_transmitAByte();
    }
    //
    // ���յ����ݡ�
    if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == SET)
    {
		USART_ClearFlag(USART1,USART_FLAG_RXNE);
		uint8_t ch = (uint8_t)USART_ReceiveData(USART1);	//

		switch(gps_receive_status)
		{
			case 0:
				if (ch == '$')  //����յ��ַ�'$'���㿪ʼ����
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
				if (ch == '\n') 		//������յ�����
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
 // ������ɡ�
    if(USART_GetFlagStatus(USART2,USART_FLAG_TC) == SET)
    {
        USART_ClearFlag(USART2,USART_FLAG_TC);
        //
        if(uart2_transmitValidBufferSize() > 0)
            uart2_transmitAByte();
    }
    //
    // ���յ����ݡ�
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
 /* usRxWrite ��?��??��?D??o����y?D��???D���??��3��D��?����???��?��?������?��?D???DD����????����?�� */
 // ������ɡ�
    if(USART_GetFlagStatus(USART3,USART_FLAG_TC) == SET)
    {
        USART_ClearFlag(USART3,USART_FLAG_TC);
        //
        if(uart3_transmitValidBufferSize() > 0)
            uart3_transmitAByte();
    }
    //
    // ���յ����ݡ�
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
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
#if 1 // for uart  printf
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
#else //for usb printf
	extern int consolePutchar(int ch);
	consolePutchar(ch); /*��ӡ����λ��*/
	return ch;
#endif
}
#endif 



