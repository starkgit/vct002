#include "power.h"
#include "sys.h"
#include "stdbool.h"
#include "key.h"



// mpu nrf power
bool mpu_nrf_power_on_flag;
#define mpu_nrf_power_on() 		mpu_nrf_power_on_flag = 1 ;PBout(14) = 0 
#define mpu_nrf_power_off()		mpu_nrf_power_on_flag = 0 ;PBout(14) = 1 

// bc95 simcard power
bool sim_power_on_flag;
#define sim_power_on() 		sim_power_on_flag = 1 ;PBout(13) = 1
#define sim_power_off() 	sim_power_on_flag = 0 ;PBout(13) = 0

// gps power
bool gps_power_on_flag;
#define gps_power_on() 		sim_power_on_flag = 1 ;PBout(12) = 1
#define gps_power_off() 	sim_power_on_flag = 0 ;PBout(12) = 0 

// For battery , charge A and B
bool vBatteryAChargeFlag;
bool vBatteryBChargeFlag;
#define batteryA_Charge_on()		vBatteryAChargeFlag = 1;PAout(5) = 1
#define BatteryB_Charge_on()		vBatteryBChargeFlag = 1;PAout(6) = 1
#define batteryA_Charge_off()		vBatteryAChargeFlag = 0;PAout(5) = 0
#define BatteryB_Charge_off()		vBatteryBChargeFlag = 0;PAout(6) = 0

//select A and B support voltage
bool vBatterySelectFlag;									// CLOSE    CHANGE
#define batteryA_support_on()		vBatterySelectFlag = 1;PAout(1) = 1;PAout(8) = 1
#define batteryB_support_on()		vBatterySelectFlag = 0;PAout(1) = 0;PAout(8) = 0

// power out
bool vPowerSwitchFlag;									// CLOSE    CHANGE
#define powerswitch_on()		vPowerSwitchFlag = 1;PDout(3) = 1
#define powerswitch_off()		vPowerSwitchFlag = 0;PDout(3) = 0

// display screen
bool vPowerScreenFlag;
#define powerscreen_on()			vPowerScreenFlag = 1;PBout(1) = 1
#define powerscreen_off()			vPowerScreenFlag = 0;PBout(1) = 0


static power_status_E power_status = POWER_OFF;
static unsigned char power_event = MSG_NONE;


/////////////////////////////////////////////
void PowerVarInit(void);
void PowerHardInit(void);


void PowerInit(void)
{
	PowerVarInit();
	
	PowerHardInit();
	
	mpu_nrf_power_on();
	
	sim_power_on();
	
	gps_power_on();

	batteryA_Charge_on();
	BatteryB_Charge_off();
	
	batteryB_support_on();
	
}


void PowerVarInit(void)
{
	mpu_nrf_power_on_flag = 0;
	sim_power_on_flag = 0;
	gps_power_on_flag = 0;
	vBatteryAChargeFlag = 0;
	vBatteryBChargeFlag = 0;	
	vBatterySelectFlag = 0;
	power_status = POWER_OFF;
	power_event = MSG_NONE;
}

void PowerHardInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);	 

	/* Close Jtag Function */
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_14 | GPIO_Pin_13 | GPIO_Pin_12;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOB, &GPIO_InitStructure);			
	powerscreen_off();

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_8;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);			

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOD, &GPIO_InitStructure);			
	powerswitch_off();
}
 

void set_power_event(unsigned char msg)
{
	power_event = msg;
}

unsigned char get_power_event(void)
{
	return power_event;
}

power_status_E get_power_status(void)
{
	return power_status;
}

void power_off_status(void)
{
	switch (power_event){
		case MSG_POWER_SHORT:
			power_status = POWER_ON;
			powerswitch_on();
 			powerscreen_on();
			break;
		default:
			break;
		}
		power_event = MSG_NONE;
}

void power_on_status(void)
{
	switch (power_event){
		case MSG_POWER_SHORT:
			power_status = POWER_OFF;
			powerscreen_off();
			powerswitch_off();
			break;

		default:
			break;
		}
	power_event = MSG_NONE;
}

void powermanager(const u32 tick)
{
	switch(power_status)
	{
		case POWER_OFF:
			power_off_status();
			break;
		case POWER_ON:
			power_on_status();
			break;
		default:
			
			break;
	}
}




















