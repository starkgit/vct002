#include "i2c1_dev.h"
#include "i2c_gpio_1.h"
#include "stdbool.h"



static bool isInit;


void iicDevInit(void)
{
	if(isInit)	return;	
	
	i2c_init_1();
	
	isInit = true;
}


unsigned char iicDevReadByte(unsigned char DevAdd,unsigned char RegAdd)
{
	unsigned char data = 0; 

	i2c_Start_1();
	i2c_SendByte_1(DevAdd);	
	i2c_WaitAck_1();
	i2c_SendByte_1(RegAdd);
	i2c_WaitAck_1();

	i2c_Start_1();
	i2c_SendByte_1(DevAdd | 0X01); // read
	i2c_WaitAck_1();
	data = i2c_ReadByte_1();
	i2c_NAck_1();		// nack <<<<<
	i2c_Stop_1();		// stop <<<<<  protocol notice

	return data ;
}




void iicDevRead(unsigned char DevAdd,unsigned char RegAdd,
						  unsigned char len ,unsigned char *rbuf)
{
	int i = 0;	
	
	i2c_Start_1();
	i2c_SendByte_1(DevAdd);	
	i2c_WaitAck_1();
	i2c_SendByte_1(RegAdd);
	i2c_WaitAck_1();

	i2c_Start_1();	
	i2c_SendByte_1(DevAdd | 0X01); // read
	i2c_WaitAck_1();
	for(i=0; i<len; i++)
	{	
		rbuf[i] = i2c_ReadByte_1();		
		if(i == len-1)
		{			
			break;
		}		
		i2c_Ack_1();		
	}
	i2c_NAck_1();//最后一个字节不应答	
	i2c_Stop_1( );	
}


void iicDevWriteByte(unsigned char DevAdd,unsigned char RegAdd,unsigned char data)
{
	i2c_Start_1();
	i2c_SendByte_1(DevAdd);
	i2c_WaitAck_1();
	i2c_SendByte_1(RegAdd);
	i2c_WaitAck_1();
	
	i2c_SendByte_1(data);
	i2c_WaitAck_1();
	i2c_Stop_1( );	
}


void iicDevWrite(unsigned char DevAdd,unsigned char RegAdd,
					unsigned char len,unsigned char *wbuf)
{
	int i = 0;
	
	i2c_Start_1();
	i2c_SendByte_1(DevAdd);
	i2c_WaitAck_1();
	i2c_SendByte_1(RegAdd);
	i2c_WaitAck_1();

	// wirte in data
	for (i = 0; i < len; ++i)
	{
		i2c_SendByte_1(wbuf[i]);
		i2c_WaitAck_1();
	}
	i2c_Stop_1();
}










