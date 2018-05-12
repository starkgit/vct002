#ifndef _POWER_H_
#define _POWER_H_
#include "stm32f10x.h"

typedef enum
{
	POWER_OFF,
	POWER_LOCK_SCREEN,
	POWER_ON,
}power_status_E;





extern void PowerInit(void);
extern void set_power_event(unsigned char msg);
extern void powermanager(const u32 tick);















#endif
