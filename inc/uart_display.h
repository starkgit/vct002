/**
  ******************************************************************************
  * @file    uart_display.h
  * @author  stark
  * @version V1.0
  * @date    2018-01-06
  * @brief   uart_display.c header file
  ******************************************************************************
  * @attention
  *  
  * THE PURPOSE OF THE TEAM: 
  * TO WIN THE CREDIBILITY OF THE CUSTOMER, TO WIN THE QUALITY OF LIFE.
  * 
  * Copyright (C), 2018, Shenzhen Viketor Technology Co.,Ltd.
  ******************************************************************************
  * 
  */  

/* Includes ------------------------------------------------------------------*/

#ifndef __UART_DISPLAY_H__
#define __UART_DISPLAY_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum
{
		
	DISPLAY_INIT,
	DISPLAY_WAIT_INIT,


	DISPLAY_INIT_WELCOME,
	DISPLAY_WAIT_WECLOME,

	DISPLAY_IDLE,
	DISPLAY_SOS,
	DISPLAY_CMD,
	DISPLAY_CMD_1,
	
	DISPLAY_DEMD_1_1,
	DISPLAY_DEMD_1_2,
	DISPLAY_DEMD_1_3,
	
	DISPLAY_SETUP_1_1,
	DISPLAY_SETUP_1_2,
}display_task_stateE;

typedef enum
{
	KEY_NONE = 0,
//	MODE_KEY_EXCISE = 1,
//	MODE_KEY_ACTUAL = 2,
	MENU_KEY_SOS = 1,
	MENU_KEY_CMD = 2,
	MENU_KEY_DEMAND = 3,
	MENU_KEY_SETUP = 4,

	// SOS Screen
	SOS_KEY_OK = 5,
	SOS_KEY_CANCEL = 6,
	SOS_KEY_DEL = 7,

	CMD_KEY_1 = 5,
	CMD_KEY_2 = 6,
	CMD_KEY_3 = 7,
	CMD_KEY_4 = 8,

	CMD_REPORT_1 = 10,	// report
	CMD_REPORT_2 = 11,
	CMD_REPORT_3 = 12,
	CMD_REPORT_4 = 13,
	CMD_REPORT_5 = 14,
	CMD_REPORT_6 = 15,
	CMD_REPORT_7 = 16,
	CMD_REPORT_8 = 17,

	DEMAND_KEY_1 = 5, // 请求界面菜单
	DEMAND_KEY_2 = 6,
	DEMAND_KEY_3 = 7,
	DEMAND_KEY_4 = 8,
	DEMAND_KEY_5 = 9,// 返回

	SET_KEY_M1 = 5,

	SET_KEY_1 = 5, // 设置界面菜单
	SET_KEY_2 = 6,
	SET_KEY_3 = 7,
	SET_KEY_4 = 8, // 返回




}keyindexE;


typedef enum
{
//	MODE_SELECT,
	DISPLAY_DELAY_IDLE,
	DISPLAY_DELAY_TEMP,
}display_delay_statusE;


typedef enum
{
	COUNT_DOWN_IDLE,
	COUNT_DOWN_START,
	COUNT_DOWN_COMPLETE,

}count_down_stateE;


extern void display_manage_request(display_task_stateE request);
extern void display_manage_task(void);
extern void display_timer5ms(void);
extern void GpuSend(char * buf1);
extern void uart_displayTask(const u32 tick);
extern void uart_display_check_key(void);
extern void uart_display_read_input(void);
extern void uart_display_battery_refresh(void);
extern void uart_display_menu_common_battery(void);
extern void uart_display_menu_common_icon(void);
extern void uart_display_screen_command(void);
extern void uart_display_screen_command_1(void);
extern void uart_display_screen_demand_1_1(void);
extern void uart_display_screen_demand_1_2(void);
extern void uart_display_screen_demand_1_3(void);
extern void uart_display_screen_demand_2_2(void);
extern void uart_display_screen_setup_1_1(void);
extern void uart_display_screen_setup_1_2(void);
extern void uart_display_screen_sos(void);
extern void uart_display_screen_welcome(void);
extern void uart_display_up(void);
extern void uart_display_dn(void);
extern void uart_display_ok(void);




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __UART_DISPLAY_H__ */

