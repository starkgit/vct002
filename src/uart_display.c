/**
  ******************************************************************************
  * @file    uart_display.c
  * @author  stark
  * @version V1.0
  * @date    2018-01-06
  * @brief   .C file
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

#include "system.h"
#include "uart_display.h"
#include "bc95.h"
#include "bc95_udp_protocol.h"
#include "math.h"



// demo
/*
DS24(0,0,'NO ??á?  ????  ×′ì? ',16,0);
BTN(5,0,40,254,85,4,135);
DS32(2,45,'1 ??á? 15?￥',16,0);
*/
const unsigned char report_cmd[][4]=
{
	"收到",
	"准备",
	"就位",
	"遇险",
	"遇障",
	"终止",
	"完成",
	"取消",
};


//static display_task_stateE display_init_status = DISPLAY_INIT_IDLE;
static display_task_stateE display_process_status = DISPLAY_INIT;
static display_task_stateE display_process_current_status;
//static display_statusE display_process_status = DISPLAY_IDLE;
//static display_statusE display_process_current_status;
//static display_delay_statusE display_delay_status;
static unsigned char display_delay_timer;

static unsigned int display_status_timer = 0;
static unsigned char uart_display_buf[10] ;
static unsigned char uart_display_len = 0;
static unsigned char read_touch_key = 0;
static unsigned int display_battery_level_delay = 0;
static unsigned char display_command_delay = 0;
static unsigned char display_menu_demand_sel_1 = 0;
static unsigned char display_menu_demand_sel_2 = 0;
static unsigned char display_menu_setup_sel_2 = 0;
static unsigned char setup_weight_param = 0;


void GpuSend(char * buf1)
{
	uint32_t buf_len = strlen(buf1);
	uart2_transmit(buf1,buf_len);
}

void uart_displayTask(const u32 tick)
{
	if (RATE_DO_EXECUTE(RATE_200_HZ, tick)) /** 200Hz 5ms update **/
	{
		display_manage_task();

	
		display_timer5ms();
	}
}


void display_timer5ms(void)
{
	if(display_status_timer)display_status_timer--;
	if(display_battery_level_delay)display_battery_level_delay--;
	if(display_command_delay)display_command_delay--;
	if(display_delay_timer)display_delay_timer--;
}


void display_manage_task(void)
{
	uart_display_read_input();

	switch ( display_process_status )
	{
		case DISPLAY_INIT:
			display_process_status = DISPLAY_WAIT_INIT;
			display_status_timer = 1000/5;
			break;
			
		case DISPLAY_WAIT_INIT:
	        if(display_status_timer) break; // power up wait power  
			display_process_status = DISPLAY_INIT_WELCOME;
			break;
			
	    case DISPLAY_INIT_WELCOME :
			uart_display_screen_welcome();
			display_status_timer = 3000/5;
			display_process_status = DISPLAY_WAIT_WECLOME;
	        break;
		case DISPLAY_WAIT_WECLOME:
	        if(display_status_timer) break; // display weclome delay 3s
			display_status_timer = 1000/5;
			display_process_status = DISPLAY_SOS;
			break;
		
	    case DISPLAY_IDLE :
	        break;
			
		case DISPLAY_SOS :// sos
			uart_display_screen_sos();
			display_process_current_status = DISPLAY_SOS;
			display_process_status = DISPLAY_IDLE;
			display_battery_level_delay = 1000/5;
			break;
		case DISPLAY_CMD :// command
			uart_display_screen_command();
			display_process_current_status = DISPLAY_CMD;
			display_process_status = DISPLAY_IDLE;
			display_battery_level_delay = 1000/5;
			break;
		case DISPLAY_CMD_1:
			uart_display_screen_command_1();
			display_process_current_status = DISPLAY_CMD_1;
			display_process_status = DISPLAY_IDLE;
			display_battery_level_delay = 1000/5;
			break;
		case DISPLAY_DEMD_1_1 :// demamd
			uart_display_screen_demand_1_1();
			display_process_current_status = DISPLAY_DEMD_1_1;
			display_process_status = DISPLAY_IDLE;
			display_battery_level_delay = 1000/5;
			break;
		case DISPLAY_DEMD_1_2 :
			uart_display_screen_demand_1_2();
			display_process_current_status = DISPLAY_DEMD_1_2;
			display_process_status = DISPLAY_IDLE;
			display_battery_level_delay = 1000/5;
			break;
		case DISPLAY_DEMD_1_3:
			uart_display_screen_demand_1_3();
			display_process_current_status = DISPLAY_DEMD_1_3;
			display_process_status = DISPLAY_IDLE;
			display_battery_level_delay = 1000/5;
			break;
		case DISPLAY_SETUP_1_1 :// setup
			uart_display_screen_setup_1_1();
			display_process_current_status = DISPLAY_SETUP_1_1;
			display_process_status = DISPLAY_IDLE;
			display_battery_level_delay = 1000/5;
			break;
		case DISPLAY_SETUP_1_2:
			uart_display_screen_setup_1_2();
			display_process_current_status = DISPLAY_SETUP_1_2;
			display_process_status = DISPLAY_IDLE;
			display_battery_level_delay = 1000/5;
			break;
	    default:
	        break;
	}

}

void display_manage_request(display_task_stateE request)
{
	display_process_status = request;
}


void uart_display_check_key(void)
{
	static unsigned char receive_key_status = 0;
	unsigned char temp_len = 0 , rbuf = 0;
	temp_len =  uart2_receiveValidBufferSize();
	
	if(temp_len <= 0)	 return ;
	uart2_readReceiveBuffer(&rbuf,1);
			
			// protocol bt 
	switch ( receive_key_status )
	{
	    case 0 :
			if(rbuf == '[')
	        receive_key_status = 1;
	        break;
	    case 1 :
			if(rbuf == 'B')
	        receive_key_status = 2;
			else
	        receive_key_status = 0;
	        break;
	    case 2 :
			if(rbuf == 'N')
	        receive_key_status = 3; // OK[BN:5]
			else
	        receive_key_status = 0;
	        break;
	    case 3 :
			if(rbuf == ':')
	        receive_key_status = 4;
			else
	        receive_key_status = 0;
	        break;
		case 4:
			if((rbuf == '\r'  || rbuf == '\n' || rbuf == ']')&& uart_display_len > 0 )
			{
				
				char data[10]="";
				get_str_data(data,(char *)uart_display_buf,0,uart_display_len);
				read_touch_key = (uint8_t) StringToInt(data);
				uart_display_len = 0;
				receive_key_status = 0;
			}
			else
			{
				uart_display_buf[uart_display_len] = rbuf;
				uart_display_len++;
				if(uart_display_len > sizeof(uart_display_buf))
				{
					uart_display_len = 0;
					receive_key_status = 0;
				}
			}
			break;
	    default:
	        break;
	}
}



void uart_display_screen_welcome(void)
{
	GpuSend("SPG(1);");
	GpuSend("\r\n");
}
 

void uart_display_battery_refresh(void)
{
	static char s_ad = 0xff;
	char buf[30];
	char temp_ad = 0;
	if(display_battery_level_delay) return;
	display_battery_level_delay = 1000/5;
	
	temp_ad = battery_percent[0] > 0 ? (battery_percent[0] - 1)/17 : 0;   // 6 grade
	if(temp_ad != s_ad)
	{
		s_ad = temp_ad;
		sprintf(buf,"ICON(295,226,8,1,6,%d);\r\n",temp_ad);
		GpuSend(buf);	
	}
}

void uart_display_menu_common_battery(void)
{
	char buf[30];
	char temp_ad = 0;
	temp_ad = battery_percent[0] > 0 ? (battery_percent[0] - 1)/17 : 0;   // 6 grade
	sprintf(buf,"ICON(295,226,8,1,6,%d);",temp_ad);
	GpuSend(buf);	
}

// common dislay
void uart_display_menu_common_icon(void)
{
	GpuSend("ICON(295,0,2,1,1,0);");
	GpuSend("ICON(295,27,3,1,3,1);");
	GpuSend("ICON(295,52,4,1,4,3);");
	GpuSend("ICON(295,69,5,1,4,3);");
	GpuSend("ICON(295,83,6,1,2,0);");
	GpuSend("ICON(295,104,7,1,2,0);");
//	uart_display_menu_common_battery();

	//battery 
//	GpuSend("ICON(295,226,8,1,6,3);");
}

void uart_display_menu_common_font(void)
{
	GpuSend("SBC(17);");
	GpuSend("BOXF(254,0,294,60,30);");
	GpuSend("BTN(1,254,0,294,60,4,0);");
	GpuSend("DS24(262,4,'紧',0,0);");
	GpuSend("DS24(262,31,'急',0,0);");
	GpuSend("SBC(56);");
	GpuSend("BOXF(254,60,294,120,56);");
	GpuSend("BTN(2,254,60,294,120,4,0);");
	GpuSend("DS24(262,64,'指',0);");
	GpuSend("DS24(262,91,'令',0);");
	GpuSend("SBC(10);");
	GpuSend("BOXF(254,120,294,180,10);");
	GpuSend("BTN(3,254,120,294,180,4,0);");
	GpuSend("DS24(262,124,'请',0);");
	GpuSend("DS24(262,151,'求',0);");
	GpuSend("SBC(45);");
	GpuSend("BOXF(254,180,294,240,45);");
	GpuSend("BTN(4,254,180,294,240,4,0);");
	GpuSend("DS24(262,184,'设',0);");
	GpuSend("DS24(262,211,'置',0);");
}



void uart_display_screen_sos(void)
{
#if 1
	GpuSend("CLS(0);");
	uart_display_menu_common_icon();
	GpuSend("BOXF(0,0,254,240,27);");
	uart_display_menu_common_font();
	
	GpuSend("SBC(27);");
	GpuSend("DS32(20,18,'确认发出呼救？',15,0);");
	GpuSend("DS32(20,136,'确认销毁设备？',15,0);");
	GpuSend("SBC(10);");
	GpuSend("BOXF(25,70,100,120,10);");
	GpuSend("BTN(5,25,70,100,120,4,0);");
	GpuSend("DS24(37,85,'确认',0,0);");
	GpuSend("SBC(1);");
	GpuSend("BOXF(125,70,200,120,1);");
	GpuSend("BTN(6,125,70,200,120,4,0);");
	GpuSend("DS24(138,83,'取消',0,0);");
	GpuSend("SBC(59);");
	GpuSend("BOXF(85,180,160,230,59);");
	GpuSend("BTN(7,85,180,160,230,4,0);");
	GpuSend("DS24(98,195,'销毁',15,0);");
	GpuSend("TPN(2);");
	GpuSend("\r\n");
#endif
//	GpuSend("SPG(3);");
//	GpuSend("\r\n");
}



void uart_display_screen_command(void)
{
	GpuSend("CLS(0);");
	GpuSend("BOXF(0,0,254,240,56);");
	uart_display_menu_common_icon();
	uart_display_menu_common_font();
	
	GpuSend("SBC(56);");
	GpuSend("DS24(0,0,'NO 指令  位置  状态 ',16,0);");
	GpuSend("TPN(2);");
	// command number   "DS32(2,45,'1 待命 2楼',16,0);"  // 21
	if(NBProtocol.count >= 1 && NBProtocol.count <= 4)
	{
			char command_send[100],i,cout;
		//	unsigned char address;
			for(cout = 1; cout <= 4; cout++)
			{
				if(NBProtocol.cmd_count[cout -1] == 0) break;
				switch (cout)
				{
					case 4 :
						GpuSend("BTN(8,0,175,254,220,4,0);");
						strcpy(command_send,"DS32(2,180,'4               ',16,0);");
						for(i = 0; i < 4; i++)
						{
							command_send[14 + i] = NBProtocol.cmd_content[3][i]; 
						}				
						command_send[19] = NBProtocol.cmd_floor[3]/10 + '0'; 
						command_send[20] = NBProtocol.cmd_floor[3]%10 + '0'; 
						command_send[21] = LOBYTE("楼");  
						command_send[22] = HIBYTE("楼");
						if(NBProtocol.cmd_report[3])
						{
							command_send[24] = LOBYTE("已"); 
							command_send[25] = HIBYTE("已"); 
							command_send[26] = LOBYTE("复"); 
							command_send[27] = HIBYTE("复"); 							
						}
						GpuSend(command_send);
						break;
					case 3 :
						GpuSend("BTN(7,0,130,254,175,4,0);");
						strcpy(command_send,"DS32(2,135,'3               ',16,0);");
						for(i = 0; i < 4; i++)
						{
							command_send[14 + i] = NBProtocol.cmd_content[2][i]; 
						}				
						
						command_send[19] = NBProtocol.cmd_floor[2]/10 + '0'; 
						command_send[20] = NBProtocol.cmd_floor[2]%10 + '0'; 
						command_send[21] = LOBYTE("楼");  
						command_send[22] = HIBYTE("楼");
						
						if(NBProtocol.cmd_report[2])
						{
							command_send[24] = LOBYTE("已"); 
							command_send[25] = HIBYTE("已"); 
							command_send[26] = LOBYTE("复"); 
							command_send[27] = HIBYTE("复"); 							
						}
						GpuSend(command_send);
						break;
					case 2:
						GpuSend("BTN(6,0,85,254,130,4,0);");
						strcpy(command_send,"DS32(2,90,'2               ',16,0);");
						for(i = 0; i < 4; i++)
						{
							command_send[13 + i] = NBProtocol.cmd_content[1][i]; 
						}				
						command_send[18] = NBProtocol.cmd_floor[1]/10 + '0'; 
						command_send[19] = NBProtocol.cmd_floor[1]%10 + '0'; 
						command_send[20] = LOBYTE("楼");  
						command_send[21] = HIBYTE("楼");
						if(NBProtocol.cmd_report[1])
						{
							command_send[23] = LOBYTE("已"); 
							command_send[24] = HIBYTE("已"); 
							command_send[25] = LOBYTE("复"); 
							command_send[26] = HIBYTE("复"); 							
						}
						GpuSend(command_send);
							break;
					case 1 :
						GpuSend("BTN(5,0,40,254,85,4,0);");
						strcpy(command_send,"DS32(2,45,'1               ',16,0);");
						for(i = 0; i < 4; i++)
						{
							command_send[13 + i] = NBProtocol.cmd_content[0][i]; 
						}				
						command_send[18] = NBProtocol.cmd_floor[0]/10 + '0';
						command_send[19] = NBProtocol.cmd_floor[0]%10 + '0'; 
						command_send[20] = LOBYTE("楼");  
						command_send[21] = HIBYTE("楼");
						if(NBProtocol.cmd_report[0])
						{
							command_send[23] = LOBYTE("已"); 
							command_send[24] = HIBYTE("已"); 
							command_send[25] = LOBYTE("复"); 
							command_send[26] = HIBYTE("复"); 							
						}
						GpuSend(command_send);
						break;
					default:
						break;
				}
			}
		}
		GpuSend("\r\n");
}

void uart_display_screen_command_1(void)
{
#if 0
	GpuSend("SPG(7);");
	GpuSend("\r\n");
#endif
	GpuSend("CLS(0);");
	uart_display_menu_common_icon();
	GpuSend("BOXF(0,0,254,240,10);");
	uart_display_menu_common_font();
	GpuSend("SBC(56);");
	GpuSend("BTN(10,20,20,80,60,4,0);");
	GpuSend("BTN(11,90,20,150,60,4,0);");
	GpuSend("BTN(12,160,20,220,60,4,0);");
	GpuSend("BTN(13,20,80,80,120,4,0);");
	GpuSend("BTN(14,90,80,150,120,4,0);");
	GpuSend("BTN(15,160,80,220,120,4,0);");
	GpuSend("BTN(16,20,140,80,180,4,0);");
	GpuSend("BTN(17,90,140,150,180,4,0);");
	GpuSend("DS24(25,29,'确认',0,0);");
	GpuSend("DS24(98,29,'准备',0,0);");
	GpuSend("DS24(166,29,'奔赴',0,0);");
	GpuSend("DS24(25,88,'就位',0,0);");
	GpuSend("DS24(98,88,'完成',0,0);");
	GpuSend("DS24(166,88,'遇险',0,0);");
	GpuSend("DS24(25,148,'透支',0,0);");
	GpuSend("DS24(98,148,'耗完',0,0);");
	GpuSend("TPN(2);");
	GpuSend("\r\n");
}

//请求界面1 无选择
void uart_display_screen_demand_1_1(void)
{
	GpuSend("CLS(0);");
	uart_display_menu_common_icon();
	GpuSend("BOXF(100,0,254,240,10);");
	uart_display_menu_common_font();
	
	GpuSend("SBC(21);");
	GpuSend("DS24(0,0,'中心回复',18,0);");
	GpuSend("SBC(10);");
	GpuSend("DS24(130,0,'类别/指令 ',0,0);");
	GpuSend("TPN(2);");
	GpuSend("SCC(60,17896);");
	GpuSend("SCC(61,17896);");
	GpuSend("MENU(100,30,154,30,24,0,0,24,8,5,110,1);");
	GpuSend("MSHW;");
	GpuSend("\r\n");
}

//请求界面1 选择
void uart_display_screen_demand_1_2(void)
{
	char str[10] = " ";
	
	GpuSend("SCC(60,17896);");
	GpuSend("SCC(61,64512);");
	GpuSend("MENU(100,30,154,30,24,0,0,24,8,5,110,1);");
	
	sprintf(str,"MSET(%d);",display_menu_demand_sel_1);
	GpuSend(str);
	GpuSend("MSHW;");
	GpuSend("\r\n");
}

void uart_display_screen_menu_select(unsigned char num)
{
	char str[10] = " ";
	sprintf(str,"MSET(%d);",num);
	GpuSend(str);
	GpuSend("\r\n");
}


//请求界面2 选择
void uart_display_screen_demand_1_3(void)
{
//	char str[10] = " ";
//	GpuSend("CLS(0);");
//	uart_display_menu_common_icon();
//	GpuSend("BOXF(100,0,254,240,10);");
//	uart_display_menu_common_font();
//	
//	GpuSend("SBC(21);");
//	GpuSend("DS24(0,0,'中心回复',18,0);");
//	GpuSend("SBC(10);");
//	GpuSend("DS24(130,0,'类别/指令 ',0,0);");
//	GpuSend("TPN(2);");

//	GpuSend("SCC(60,17896);");
//	GpuSend("SCC(61,64512);");
	GpuSend("MENU(100,30,154,30,24,0,0,24,8,5,111,1);");
//	sprintf(str,"MSET(%d);",display_menu_demand_sel_2);  // MSET 和 MSHW 的指令不能同时？
//	GpuSend(str);
	GpuSend("MSHW;");
	GpuSend("\r\n");
}


void uart_display_screen_setup_1_1(void)
{
	 char temp_string[50];
	GpuSend("CLS(0);");
	uart_display_menu_common_icon();
	GpuSend("BOXF(0,0,254,240,45);");
	uart_display_menu_common_font();
	GpuSend("TPN(2);");
	GpuSend("PL(0,27,254,27,18);");
	GpuSend("PL(0,57,254,57,18);");
	GpuSend("PL(0,87,254,87,18);");

	sprintf(temp_string,"DS24(0,0,'设备地址：%08X',18,0);",(*(volatile uint32_t *)0x1ffff7e8));
	GpuSend(temp_string);
	
	GpuSend("DS24(0,30,'姓名：张萌萌',18,0);");
	GpuSend("SBC(56);");
//	GpuSend("SCC(60,40443);");
//	GpuSend("SCC(61,64512);");
	GpuSend("BOXF(0,60,254,86,56);");
	if(setup_weight_param == 0)
	{
		GpuSend("DS24(0,60,'空呼设置(6.8公斤)',18,0);");
	}
	else if(setup_weight_param == 1)
	{
		GpuSend("DS24(0,60,'空呼设置(9公斤)',18,0);");
	}
	else if(setup_weight_param == 2)
	{
		GpuSend("DS24(0,60,'空呼设置(12公斤)',18,0);");
	}
	GpuSend("BTN(5,0,60,254,90,0,0);");
	GpuSend("\r\n");


}


void uart_display_screen_setup_1_2(void)
{
	GpuSend("SCC(60,40443);");
	GpuSend("SCC(61,64512);");
	GpuSend("SBC(45);");
	GpuSend("MENU(0,0,254,30,24,0,0,24,8,4,121,1);");
	GpuSend("MSHW;");
	GpuSend("\r\n");

}


void uart_display_up(void)
{
	switch ( display_process_current_status )
	{
		case DISPLAY_SOS:
			display_manage_request(DISPLAY_CMD);
			break;
		case DISPLAY_CMD:
			display_manage_request(DISPLAY_DEMD_1_1);
			break;
		case DISPLAY_CMD_1:
			break;
		case DISPLAY_DEMD_1_1://无选择
			display_manage_request(DISPLAY_SETUP_1_1);
			break;
		case DISPLAY_DEMD_1_2://有选择
			if(display_menu_demand_sel_1) display_menu_demand_sel_1--;
			else display_menu_demand_sel_1 = (DEMAND_KEY_5 - DEMAND_KEY_1);
			uart_display_screen_menu_select(display_menu_demand_sel_1);
			break;

		case DISPLAY_DEMD_1_3://有选择
			if(display_menu_demand_sel_2) display_menu_demand_sel_2--;
			else display_menu_demand_sel_2 = (DEMAND_KEY_5 - DEMAND_KEY_1);
			uart_display_screen_menu_select(display_menu_demand_sel_2);
			break;
		case DISPLAY_SETUP_1_1:
			display_manage_request(DISPLAY_SOS);
			break;
		case DISPLAY_SETUP_1_2:
			if(display_menu_setup_sel_2) display_menu_setup_sel_2--;
			else display_menu_setup_sel_2 = (SET_KEY_4 - SET_KEY_1);
			uart_display_screen_menu_select(display_menu_setup_sel_2);
			break;
		default:
			break;
	}
}

void uart_display_dn(void)
{
	switch ( display_process_current_status )
	{
		case DISPLAY_SOS:
			display_manage_request(DISPLAY_SETUP_1_1);
			break;
		case DISPLAY_CMD:
			display_manage_request(DISPLAY_SOS);
			break;
		case DISPLAY_CMD_1:
			break;
		case DISPLAY_DEMD_1_1:
			display_manage_request(DISPLAY_CMD);
			break;
		case DISPLAY_DEMD_1_2://有选择
			if(display_menu_demand_sel_1 < (DEMAND_KEY_5 - DEMAND_KEY_1)) display_menu_demand_sel_1++;
			else display_menu_demand_sel_1 = 0;
			uart_display_screen_menu_select(display_menu_demand_sel_1);
			break;

		case DISPLAY_DEMD_1_3://有选择
			if(display_menu_demand_sel_2 < (DEMAND_KEY_5 - DEMAND_KEY_1)) display_menu_demand_sel_2++;
			else display_menu_demand_sel_2 = 0;
			uart_display_screen_menu_select(display_menu_demand_sel_2);
			break;
		case DISPLAY_SETUP_1_1:
			display_manage_request(DISPLAY_DEMD_1_1);
			break;
		case DISPLAY_SETUP_1_2:
			if(display_menu_setup_sel_2 < (SET_KEY_4 - SET_KEY_1)) display_menu_setup_sel_2++;
			else display_menu_setup_sel_2 = 0;
			uart_display_screen_menu_select(display_menu_setup_sel_2);
			break;
		default:
			break;
	}
}


void uart_display_ok(void)
{
	switch ( display_process_current_status )
	{
		case DISPLAY_SOS:
			break;
		case DISPLAY_CMD:
			break;
		case DISPLAY_CMD_1:
			break;
		case DISPLAY_DEMD_1_1://无选择
			display_manage_request(DISPLAY_DEMD_1_2);
			break;
		case DISPLAY_DEMD_1_2://有选择
			if(display_menu_demand_sel_1 == 0)
			{
				display_manage_request(DISPLAY_DEMD_1_3);// 下一级菜单
			}
			else if(display_menu_demand_sel_1 == 4)
			{
				display_menu_demand_sel_1 = 0;
				display_manage_request(DISPLAY_DEMD_1_1); // 上一级菜单
			}
			break;
		case DISPLAY_DEMD_1_3://有选择
			display_manage_request(DISPLAY_DEMD_1_1); // 上一级菜单
			if(display_menu_demand_sel_2 < 4)
			{
				protocol_master_send_demand(TONG_YONG,display_menu_demand_sel_2 + 1);	// 协议通信
			}
			display_menu_demand_sel_2 = 0;
			break;
		case DISPLAY_SETUP_1_1:
			display_manage_request(DISPLAY_SETUP_1_2);// 下一级菜单
			break;
		case DISPLAY_SETUP_1_2:
			if(display_menu_setup_sel_2 < 3)
			{
				setup_weight_param = display_menu_setup_sel_2;
				protocol_master_send_setup(setup_weight_param);
			}
			display_menu_setup_sel_2 = 0;
			display_manage_request(DISPLAY_SETUP_1_1); // 上一级菜单
			break;
		default:
			break;
	}
}


void uart_display_read_input(void)
{
	
	uart_display_check_key();
//	uart_display_battery_refresh();
//	uart_display_command_refresh();

	if(display_process_status <= DISPLAY_WAIT_WECLOME) return;
	
	if(read_touch_key == KEY_NONE)	return;

	// 标签页 主页 显示操作
	switch ( display_process_current_status )
	{
	    case DISPLAY_SOS:
	    case DISPLAY_CMD:
	    case DISPLAY_CMD_1:
		case DISPLAY_DEMD_1_1:
		case DISPLAY_DEMD_1_2:
		case DISPLAY_DEMD_1_3:
		case DISPLAY_SETUP_1_1:
		case DISPLAY_SETUP_1_2:
			if(read_touch_key == MENU_KEY_SOS)
			{
				display_manage_request(DISPLAY_SOS);
			}
			else if(read_touch_key == MENU_KEY_CMD)
			{
				display_manage_request(DISPLAY_CMD);
			}
			else if(read_touch_key == MENU_KEY_DEMAND)
			{
				display_manage_request(DISPLAY_DEMD_1_1);
			}
			else if(read_touch_key == MENU_KEY_SETUP)
			{
				display_manage_request(DISPLAY_SETUP_1_1);
			}
	    default:
	        break;
	}

	// 协议操作 及 显示跳转
	switch ( display_process_current_status )
	{
	    case DISPLAY_SOS :
			if(read_touch_key == SOS_KEY_OK)
			{
				protocol_master_send_sos(E_SOS_OK);   // 协议通信
			}
			else if(read_touch_key == SOS_KEY_CANCEL)
			{
				protocol_master_send_sos(E_SOS_CANCEL);
			}
			else if(read_touch_key == SOS_KEY_DEL)
			{
				protocol_master_send_sos(E_SOS_DEL);
			}
			break;
	    case DISPLAY_CMD:
			if(read_touch_key == CMD_KEY_1 || read_touch_key == CMD_KEY_2 
				|| read_touch_key == CMD_KEY_3 || read_touch_key == CMD_KEY_4)
			{
				NBProtocol.cmd_recount = read_touch_key - CMD_KEY_1 + 1; // 1 ~ 4
				bc95_master_send_protocol_command(01,read_touch_key - CMD_KEY_1 + 1);
				display_manage_request(DISPLAY_CMD_1);
			}
	        break;
		case DISPLAY_CMD_1:
			if(read_touch_key >= CMD_REPORT_1 && read_touch_key <= CMD_REPORT_8 )
			{
				NBProtocol.cmd_report[NBProtocol.cmd_recount - 1] = 1;
				bc95_master_send_protocol_command(02,read_touch_key - CMD_REPORT_1 + 1);
				display_manage_request(DISPLAY_CMD);
			}
			break;
		case DISPLAY_DEMD_1_1:
			if(read_touch_key >= DEMAND_KEY_1 && read_touch_key <= DEMAND_KEY_5)
			{
				display_menu_demand_sel_1 = read_touch_key - DEMAND_KEY_1;
				display_manage_request(DISPLAY_DEMD_1_2);
			}
			break;
		case DISPLAY_DEMD_1_2:  //有选择
			if(read_touch_key >= DEMAND_KEY_1 && read_touch_key <= DEMAND_KEY_5)
			{
				if(display_menu_demand_sel_1 == (read_touch_key - DEMAND_KEY_1))
				{
					if(read_touch_key == DEMAND_KEY_1 ) 
						display_manage_request(DISPLAY_DEMD_1_3);
					else if(read_touch_key == DEMAND_KEY_5)	// enter pre menu
						display_manage_request(DISPLAY_DEMD_1_1);

					display_menu_demand_sel_1 = 0;
				}
				else 
				{
					display_menu_demand_sel_1 = read_touch_key - DEMAND_KEY_1;
					uart_display_screen_menu_select(display_menu_demand_sel_1);
				}
			}
			break;
		case DISPLAY_DEMD_1_3://有选择
			if(read_touch_key >= DEMAND_KEY_1 && read_touch_key <= DEMAND_KEY_5)
			{
				if(display_menu_demand_sel_2 == (read_touch_key - DEMAND_KEY_1))
				{
					display_manage_request(DISPLAY_DEMD_1_1); // enter pre menu
					if(display_menu_demand_sel_2 < 4)
					{
						protocol_master_send_demand(TONG_YONG,display_menu_demand_sel_2 + 1);   // 协议通信
					}
					display_menu_demand_sel_2 = 0;
				}
				else 
				{
					display_menu_demand_sel_2 = read_touch_key - DEMAND_KEY_1;
					uart_display_screen_menu_select(display_menu_demand_sel_2);
				}
			}
			break;
		case DISPLAY_SETUP_1_1:
			if(read_touch_key == SET_KEY_M1)
			{
				display_manage_request(DISPLAY_SETUP_1_2);
				display_menu_setup_sel_2 = 0;
			}
			break;
		case DISPLAY_SETUP_1_2:
			if(read_touch_key >= SET_KEY_1 && read_touch_key <= SET_KEY_4)
			{
				if(display_menu_setup_sel_2 == (read_touch_key - SET_KEY_1))
				{
					if(display_menu_setup_sel_2 < 3)
					{
						setup_weight_param = display_menu_setup_sel_2;
						protocol_master_send_setup(setup_weight_param);
					}
					display_manage_request(DISPLAY_SETUP_1_1); // enter pre menu
					display_menu_setup_sel_2 = 0;
				}
				else 
				{
					display_menu_setup_sel_2 = read_touch_key - SET_KEY_1;
					uart_display_screen_menu_select(display_menu_setup_sel_2);
				}
			}
			break;
	    default:
	        break;
	}	

	read_touch_key = KEY_NONE;
}





