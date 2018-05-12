#ifndef _BSP_I2C_GPIO_1_H
#define _BSP_I2C_GPIO_1_H
#include "stm32f10x.h"


void i2c_init_1(void);
void i2c_Start_1(void);
void i2c_Stop_1(void);
void i2c_SendByte_1(uint8_t _ucByte);
uint8_t i2c_ReadByte_1(void);
uint8_t i2c_WaitAck_1(void);
void i2c_Ack_1(void);
void i2c_NAck_1(void);
uint8_t i2c_CheckDevice_1(uint8_t _Address);

#endif
