#include "key.h"
#include "stabilizer_types.h"
#include "uart_display.h"


//LED IO初始化
void KeyInit(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin  = KEY_UP | KEY_DN | KEY_OK;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,KEY_UP | KEY_DN | KEY_OK);
}

#define 	ADC_KEY_SCAN_TIME			20
#define 	ADC_KEY_JITTER_TIME			30
#define 	ADC_KEY_CP_TIME				1000
#define 	ADC_KEY_CPH_TIME			200
#define		ADC_KEY_COUNT				10 			//key count per adc channel
#define		ADC_KEY_FULL_VAL			4096		//

static const uint16_t AdcKeyEvent[][5] = 
{
//	PDS(按键开始)		SPR(短按松开)		CPS(长按开始)		CPH(长按保持)		CPR(长按松开)
	{MSG_UP,			MSG_NONE, 	MSG_NONE, 		MSG_NONE, 		MSG_NONE	},	//SW1
	{MSG_DN,			MSG_NONE, 	MSG_NONE, 		MSG_NONE, 		MSG_NONE	},	//SW2
	{MSG_OK,			MSG_NONE, 	MSG_NONE, 		MSG_NONE, 		MSG_NONE	},	//SW3	
																						
};	

typedef enum _ADC_KEY_STATE
{
    ADC_KEY_STATE_IDLE,
    ADC_KEY_STATE_JITTER,
    ADC_KEY_STATE_PRESS_DOWN,
    ADC_KEY_STATE_CP

} ADC_KEY_STATE;


unsigned int			AdcKeyWaitTimer;
unsigned char			AdcKeyScanTimer;
ADC_KEY_STATE	AdcKeyState;

static uint8_t GetAdcKeyIndex(void)
{
	unsigned char key_up,key_dn,key_ok;
	key_up = READ_KEY_UP();
	key_dn = READ_KEY_DN();
	key_ok = READ_KEY_OK();

//	if(key_ok ==1)
//	{
//		return 0xff;
//}
	if( key_up == 1 && key_dn == 0 && key_ok == 0)
	{
		return 0;
	}
	else if(key_up == 0 && key_dn == 1 && key_ok == 0)
	{
		return 1;
	}
	else if(key_up == 0 && key_dn == 0 && key_ok == 1)
	{
		return 2;
	}
	else 
	{
		return 0xff;
	}
}


// adc key scan
// return: 0---no key, else---key msg
uint16_t KeyScan(void)
{
	static 	uint8_t 	PreKeyIndex = 0xFF;
	uint8_t				KeyIndex;

#if 0
	if(!IsTimeOut(&AdcKeyScanTimer))
	{
		return MSG_NONE;
	}
	TimeOutSet(&AdcKeyScanTimer, ADC_KEY_SCAN_TIME);
#endif
	if(AdcKeyScanTimer)  return MSG_NONE;
	
	AdcKeyScanTimer = ADC_KEY_SCAN_TIME;

	KeyIndex = GetAdcKeyIndex();

	switch(AdcKeyState)
	{
		case ADC_KEY_STATE_IDLE:
			if(KeyIndex == 0xFF)
			{
				return MSG_NONE;
			}

			PreKeyIndex = KeyIndex;
		//	TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_JITTER_TIME);
			AdcKeyWaitTimer = ADC_KEY_JITTER_TIME;
			//DBG("GOTO JITTER!\n");
			AdcKeyState = ADC_KEY_STATE_JITTER;

		case ADC_KEY_STATE_JITTER:
			if(PreKeyIndex != KeyIndex)
			{
				//DBG("GOTO IDLE Because jitter!\n");
				AdcKeyState = ADC_KEY_STATE_IDLE;
			}
		//	else if(IsTimeOut(&AdcKeyWaitTimer))
			else if(AdcKeyWaitTimer == 0)
			{
				//DBG("GOTO PRESS_DOWN!\n");
			//	TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_CP_TIME);
				AdcKeyWaitTimer = ADC_KEY_CP_TIME;
				AdcKeyState = ADC_KEY_STATE_PRESS_DOWN;
				return AdcKeyEvent[PreKeyIndex][0];
			}
			break;

		case ADC_KEY_STATE_PRESS_DOWN:
			if(PreKeyIndex != KeyIndex)
			{
				//return key sp value
				//DBG("ADC KEY SP%bu\n", PreKeyIndex);
				AdcKeyState = ADC_KEY_STATE_IDLE;
				return AdcKeyEvent[PreKeyIndex][1];
			}
			//else if(IsTimeOut(&AdcKeyWaitTimer))
			else if(AdcKeyWaitTimer == 0)
			{
				//return key cp value
				//DBG("ADC KEY CPS!\n");
			//	TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_CPH_TIME);
				AdcKeyWaitTimer = ADC_KEY_CPH_TIME;
				AdcKeyState = ADC_KEY_STATE_CP;
				return AdcKeyEvent[PreKeyIndex][2];
			}
			break;

		case ADC_KEY_STATE_CP:
			if(PreKeyIndex != KeyIndex)
			{
				//return key cp value
				//DBG("ADC KEY CPR!\n");
				AdcKeyState = ADC_KEY_STATE_IDLE;
				return AdcKeyEvent[PreKeyIndex][4];
			}
			//else if(IsTimeOut(&AdcKeyWaitTimer))
			else if(AdcKeyWaitTimer == 0)
			{
				//return key cph value
				//DBG("ADC KEY CPH!\n");
			//	TimeOutSet(&AdcKeyWaitTimer, ADC_KEY_CPH_TIME);
				AdcKeyWaitTimer = ADC_KEY_CPH_TIME;
				return AdcKeyEvent[PreKeyIndex][3];
			}
			break;

		default:
			AdcKeyState = ADC_KEY_STATE_IDLE;
			break;
	}

	return MSG_NONE;
}


void KeyTimer(void)
{
	if(AdcKeyWaitTimer)AdcKeyWaitTimer--;
	if(AdcKeyScanTimer)AdcKeyScanTimer--;
}


void KeyFunction(unsigned char msg)
{
	if(msg == MSG_UP)
	{
		uart_display_up();
	}
	else if(msg == MSG_DN)
	{
		uart_display_dn();
	}
	else if(msg == MSG_OK)
	{
		uart_display_ok();
	}
}




void KeyTask(const u32 tick)
{

	if (RATE_DO_EXECUTE(RATE_1000_HZ, tick)) /** 100Hz 1ms update **/
	{
		KeyFunction(KeyScan());
		KeyTimer();
	}
}



















#if 0

typedef enum _POWER_KEY_STATE
{
	POWER_KEY_STATE_IDLE,
	POWER_KEY_STATE_JITTER,
	POWER_KEY_STATE_PRESS_DOWN,
	POWER_KEY_STATE_CP,

} POWER_KEY_STATE;


#define POWER_KEY_JITTER_TIME		30			//????ê±??￡???ê±??oíèí?a1??a1??úó2?tê±??óD1?
#define POWER_KEY_CP_TIME			1000			
#define POWER_KEY_CPH_TIME			200			
static POWER_KEY_STATE PowerKeyState;
static unsigned int	PowerKeyWaitTimer;
/*****************************************************************************
 * oˉ êy ??  : PowerKeyScan
 * ?o ?e è?  : stark
 * ′′?¨è??ú  : 2018?ê1??2è?
 * oˉêy1|?ü  : detect power key 
 * ê?è?2?êy  : void  None
 * ê?3?2?êy  : power key status
 * ·μ ?? ?μ  : 
 * μ÷ó?1??μ  : 
 * ??    ?ü  : 

*****************************************************************************/
unsigned char PowerKeyScan(void)							
{
	unsigned char temp_key = get_power_key();
	
	switch(PowerKeyState)
	{
		case POWER_KEY_STATE_IDLE:
			if(temp_key == KEY_NULL)
			{
				return 0;
			}
			else
			{	
				PowerKeyWaitTimer = POWER_KEY_JITTER_TIME;
				PowerKeyState = POWER_KEY_STATE_JITTER;
			}
			break;
		case POWER_KEY_STATE_JITTER:
			if(temp_key == KEY_NULL)
			{
				PowerKeyState = POWER_KEY_STATE_IDLE;
			}
			else if(PowerKeyWaitTimer == 0)
			{
				PowerKeyState = POWER_KEY_STATE_PRESS_DOWN;
				PowerKeyWaitTimer = POWER_KEY_CP_TIME;
				return 0;	
			}
			break;
			
		case POWER_KEY_STATE_PRESS_DOWN:
			if(temp_key == KEY_NULL)
			{
				PowerKeyState = POWER_KEY_STATE_IDLE;
		//		set_event(EV_POWER_COURT);
				return 1;		// short key release
			}
			else if(PowerKeyWaitTimer == 0)
			{
				PowerKeyState = POWER_KEY_STATE_CP;
				PowerKeyWaitTimer = POWER_KEY_CPH_TIME;
				return 0;		// 
			}
			break;
			
		case POWER_KEY_STATE_CP:
			if(temp_key == KEY_NULL)
			{
				PowerKeyState = POWER_KEY_STATE_IDLE;
				return 0;		// long key release
			}
			else if(PowerKeyWaitTimer == 0)
			{
		//		set_event(EV_POWER_LONG);
				return 1;	  // long key hold	
			}
			else
			{
				//do no thing
			}
			break;
			
		default:
			PowerKeyState = POWER_KEY_STATE_IDLE;
			break;
	}
	return 0;
}
#endif









