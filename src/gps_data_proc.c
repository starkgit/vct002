#include "gps_data_proc.h"
#include "algorithm.h"
#include <string.h>
#include <stdlib.h>

void UTC2BTC(DATE_TIME *);

//====================================================================//
// 语法格式：int GPS_RMC_Parse(char *line, GPS_INFO *GPS)  
// 实现功能：把gps模块的GPRMC信息解析为可识别的数据
// 参    数：存放原始信息字符数组、存储可识别数据的结构体
// 返 回 值：
//			 1: 解析GPRMC完毕
//           0: 没有进行解析，或数据无效
//====================================================================//
int GPS_RMC_Parse(char *line, GPS_INFO *GPS)
{
	uint8_t ch, status, tmp;
	float lati_cent_tmp, lati_second_tmp;
	float long_cent_tmp, long_second_tmp;
	char *buf = line;
	ch = buf[5];
	status = buf[GetComma(2, buf)];

	if (ch == 'C')  //如果第五个字符是C，($GPRMC)
	{
		if (status == 'A')  //如果数据有效，则分析
		{
			GPS -> NS       = buf[GetComma(4, buf)];
			GPS -> EW       = buf[GetComma(6, buf)];

			GPS->latitude   = Get_Double_Number(&buf[GetComma(3, buf)]);
			GPS->longitude  = Get_Double_Number(&buf[GetComma( 5, buf)]);

       	    GPS->latitude_Degree  = (int)GPS->latitude / 100;       //分离纬度
			lati_cent_tmp         = (GPS->latitude - GPS->latitude_Degree * 100);
			GPS->latitude_Cent    = (int)lati_cent_tmp;
			lati_second_tmp       = (lati_cent_tmp - GPS->latitude_Cent) * 60;
			GPS->latitude_Second  = (int)lati_second_tmp;

			GPS->longitude_Degree = (int)GPS->longitude / 100;	//分离经度
			long_cent_tmp         = (GPS->longitude - GPS->longitude_Degree * 100);
			GPS->longitude_Cent   = (int)long_cent_tmp;    
			long_second_tmp       = (long_cent_tmp - GPS->longitude_Cent) * 60;
			GPS->longitude_Second = (int)long_second_tmp;
		
			GPS->D.hour    = (buf[7] - '0') * 10 + (buf[8] - '0');		//时间
			GPS->D.minute  = (buf[9] - '0') * 10 + (buf[10] - '0');
			GPS->D.second  = (buf[11] - '0') * 10 + (buf[12] - '0');
			tmp = GetComma(9, buf);
			GPS->D.day     = (buf[tmp + 0] - '0') * 10 + (buf[tmp + 1] - '0'); //日期
			GPS->D.month   = (buf[tmp + 2] - '0') * 10 + (buf[tmp + 3] - '0');
			GPS->D.year    = (buf[tmp + 4] - '0') * 10 + (buf[tmp + 5] - '0')+2000;

			UTC2BTC(&GPS->D);
			
			return 1;
		}		
	}
	
	return 0;
}

//====================================================================//
// 语法格式：int GPS_GSV_Parse(char *line, GPS_INFO *GPS)  
// 实现功能：把gps模块的GPGSV信息解析为可识别的数据
// 参    数：存放原始信息字符数组、存储可识别数据的结构体
// 返 回 值：
//			 1: 解析GPGGA完毕
//           0: 没有进行解析，或数据无效
//====================================================================//
int GPS_GSV_Parse(char *line, GPS_INFO *GPS)
{
	unsigned char ch;
	char *buf = line;
	ch = buf[5];

	if (ch == 'V')  //$GPGSV
	{
		GPS->satellite = Get_Int_Number(&buf[GetComma(3, buf)]);
		return 1;
	}
	
	return 0;
}

//====================================================================//
// 语法格式：void UTC2BTC(DATE_TIME *GPS)
// 实现功能：转化时间为北京时区的时间
// 参    数：存放时间的结构体
// 返 回 值：无
//====================================================================//
void UTC2BTC(DATE_TIME *date_time)
{
	date_time->second ++;  
	if(date_time->second > 59)
	{
		date_time->second = 0;
		date_time->minute ++;
		if(date_time->minute > 59)
		{
			date_time->minute = 0;
			date_time->hour ++;
		}
	}	

    date_time->hour = date_time->hour + 8;
	if(date_time->hour > 23)
	{
		date_time->hour -= 24;
		date_time->day += 1;
		if(date_time->month == 2 ||
		   		date_time->month == 4 ||
		   		date_time->month == 6 ||
		   		date_time->month == 9 ||
		   		date_time->month == 11 )
		{
			if(date_time->day > 30)
			{
		   		date_time->day = 1;
				date_time->month++;
			}
		}
		else
		{
			if(date_time->day > 31)
			{	
		   		date_time->day = 1;
				date_time->month ++;
			}
		}
		if(date_time->year % 4 == 0 )
		{
	   		if(date_time->day > 29 && date_time->month == 2)
			{		
	   			date_time->day = 1;
				date_time->month ++;
			}
		}
		else
		{
	   		if(date_time->day > 28 &&date_time->month == 2)
			{
	   			date_time->day = 1;
				date_time->month ++;
			}
		}
		if(date_time->month > 12)
		{
			date_time->month -= 12;
			date_time->year ++;
		}		
	}
}


