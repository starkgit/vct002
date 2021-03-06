/*
	应用说明＿
	在访问I2C设备前，请先调用 i2c_CheckDevice() 检测I2C设备是否正常，该函数会配置GPIO
	注意读写应答  <<<<<<<<
*/

#include "i2c_gpio_1.h"


/* 定义I2C总线连接的GPIO端口, 用户只需要修改下靿行代码即可任意改变SCL和SDA的引脿*/

#define RCC_I2C_PORT 	RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */

#define PORT_I2C_SCL	GPIOB			/* GPIO端口 */
#define PIN_I2C_SCL		GPIO_Pin_8		/* GPIO引脚 */

#define PORT_I2C_SDA	GPIOB			/* GPIO端口 */
#define PIN_I2C_SDA		GPIO_Pin_9		/* GPIO引脚 */

#define I2C_SCL_PIN		GPIO_Pin_8			/* 连接到SCL时钟线的GPIO */
#define I2C_SDA_PIN		GPIO_Pin_9			/* 连接到SDA数据线的GPIO */

/* 定义读写SCL和SDA的宏 */
#define I2C_SCL_1()  PORT_I2C_SCL->BSRR = I2C_SCL_PIN				/* SCL = 1 */
#define I2C_SCL_0()  PORT_I2C_SCL->BRR = I2C_SCL_PIN				/* SCL = 0 */

#define I2C_SDA_1()  PORT_I2C_SDA->BSRR = I2C_SDA_PIN				/* SDA = 1 */
#define I2C_SDA_0()  PORT_I2C_SDA->BRR = I2C_SDA_PIN				/* SDA = 0 */

#define I2C_SDA_READ()  ((PORT_I2C_SDA->IDR & I2C_SDA_PIN) != 0)	/* 读SDA口线状怿*/
#define I2C_SCL_READ()  ((PORT_I2C_SCL->IDR & I2C_SCL_PIN) != 0)	/* 读SCL口线状怿*/


#define I2C_WR	0		/* 写控制bit */
#define I2C_RD	1		/* 读控制bit */
/*
*********************************************************************************************************
*	凿敿吿 bsp_InitI2C
*	功能说明: 配置I2C总线的GPIO，采用模拟IO的方式实玿
*	彿   叿  旿
*	迿囿倿 旿
*********************************************************************************************************
*/
void i2c_init_1(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_I2C_PORT, ENABLE);	/* 打开GPIO时钟 */

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	/* 开漏输出模弿*/
	
	GPIO_InitStructure.GPIO_Pin = PIN_I2C_SCL;
	GPIO_Init(PORT_I2C_SCL, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_I2C_SDA;
	GPIO_Init(PORT_I2C_SDA, &GPIO_InitStructure);

	/* 给一个停止信叿 复位I2C总线上的所有设备到待机模式 */
	i2c_Stop_1();
}

/*
*********************************************************************************************************
*	凿敿吿 i2c_Delay
*	功能说明: I2C总线位延迟，最忿00KHz
*	彿   叿  旿
*	迿囿倿 旿
*********************************************************************************************************
*/
static void i2c_Delay(void)
{
	uint8_t i;

	/*4: 380Khz
		5: 344KHZ
	  6: 288khz
	*/
	for (i = 0; i <  8; i++);
}

/*
*********************************************************************************************************
*	凿敿吿 i2c_Start
*	功能说明: CPU发起I2C总线启动信号
*	彿   叿  旿
*	迿囿倿 旿
*********************************************************************************************************
*/
void i2c_Start_1(void)
{
	/* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
	I2C_SDA_1();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_0();
	i2c_Delay();
	
	I2C_SCL_0();
	i2c_Delay();
}

/*
*********************************************************************************************************
*	凿敿吿 i2c_Start
*	功能说明: CPU发起I2C总线停止信号
*	彿   叿  旿
*	迿囿倿 旿
*********************************************************************************************************
*/
void i2c_Stop_1(void)
{
	/* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */
	I2C_SDA_0();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_1();
	i2c_Delay();
}

/*
*********************************************************************************************************
*	凿敿吿 i2c_SendByte
*	功能说明: CPU向I2C总线设备发逿bit数据
*	彿   叿  _ucByte ＿等待发送的字节
*	迿囿倿 旿
*********************************************************************************************************
*/
void i2c_SendByte_1(uint8_t _ucByte)
{
	uint8_t i;

	/* 先发送字节的高位bit7 */
	for (i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			I2C_SDA_1();
		}
		else
		{
			I2C_SDA_0();
		}
		i2c_Delay();
		I2C_SCL_1();
		i2c_Delay();
		I2C_SCL_0();
		if (i == 7)
		{
			 I2C_SDA_1(); // 释放总线
		}
		_ucByte <<= 1;	/* 左移一个bit */
		i2c_Delay();
	}
}

/*
*********************************************************************************************************
*	凿敿吿 i2c_ReadByte
*	功能说明: CPU从I2C总线设备读取8bit数据
*	彿   叿  旿
*	迿囿倿 读到的数捿
*********************************************************************************************************
*/
uint8_t i2c_ReadByte_1(void)
{
	uint8_t i;
	uint8_t value;

	/* 读到笿个bit为数据的bit7 */
	value = 0;
	for (i = 0; i < 8; i++)
	{
		value <<= 1;
		I2C_SCL_1();
		i2c_Delay();
		if (I2C_SDA_READ())
		{
			value++;
		}
		I2C_SCL_0();
		i2c_Delay();
	}
	return value;
}

/*
*********************************************************************************************************
*	凿敿吿 i2c_WaitAck
*	功能说明: CPU产生一个时钟，并读取器件的ACK应答信号
*	彿   叿  旿
*	迿囿倿 返回0表示正确应答＿表示无器件响庿
*********************************************************************************************************
*/
uint8_t i2c_WaitAck_1(void)
{
	uint8_t re;

	I2C_SDA_1();	/* CPU释放SDA总线 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
	i2c_Delay();
	if (I2C_SDA_READ())	/* CPU读取SDA口线状怿*/
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	I2C_SCL_0();
	i2c_Delay();
	return re;
}

/*
*********************************************************************************************************
*	凿敿吿 i2c_Ack
*	功能说明: CPU产生一个ACK信号
*	彿   叿  旿
*	迿囿倿 旿
*********************************************************************************************************
*/
void i2c_Ack_1(void)
{
	I2C_SDA_0();	/* CPU驱动SDA = 0 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU产生1个时钿*/
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
	I2C_SDA_1();	/* CPU释放SDA总线 */
}

/*
*********************************************************************************************************
*	凿敿吿 i2c_NAck
*	功能说明: CPU产生1个NACK信号
*	彿   叿  旿
*	迿囿倿 旿
*********************************************************************************************************
*/
void i2c_NAck_1(void)
{
	I2C_SDA_1();	/* CPU驱动SDA = 1 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU产生1个时钿*/
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
}

/*
*********************************************************************************************************
*	凿敿吿 i2c_CheckDevice
*	功能说明: 检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存圿
*	彿   叿  _Address：设备的I2C总线地址
*	迿囿倿 返回倿0 表示正确＿返回1表示未探测到
*********************************************************************************************************
*/
uint8_t i2c_CheckDevice_1(uint8_t _Address)
{
	uint8_t ucAck;

	if (I2C_SDA_READ() && I2C_SCL_READ())
	{
		i2c_Start_1();		/* 发送启动信叿*/

		/* 发送设备地址+读写控制bit＿ = w＿1 = r) bit7 先传 */
		i2c_SendByte_1(_Address | I2C_WR);
		ucAck = i2c_WaitAck_1();	/* 检测设备的ACK应答 */

		i2c_Stop_1();			/* 发送停止信叿*/

		return ucAck;
	}
	return 1;	/* I2C总线异常 */
}



