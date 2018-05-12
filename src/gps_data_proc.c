#include "gps_data_proc.h"
#include "algorithm.h"
#include <string.h>
#include <stdlib.h>

void UTC2BTC(DATE_TIME *);

//====================================================================//
// �﷨��ʽ��int GPS_RMC_Parse(char *line, GPS_INFO *GPS)  
// ʵ�ֹ��ܣ���gpsģ���GPRMC��Ϣ����Ϊ��ʶ�������
// ��    �������ԭʼ��Ϣ�ַ����顢�洢��ʶ�����ݵĽṹ��
// �� �� ֵ��
//			 1: ����GPRMC���
//           0: û�н��н�������������Ч
//====================================================================//
int GPS_RMC_Parse(char *line, GPS_INFO *GPS)
{
	uint8_t ch, status, tmp;
	float lati_cent_tmp, lati_second_tmp;
	float long_cent_tmp, long_second_tmp;
	char *buf = line;
	ch = buf[5];
	status = buf[GetComma(2, buf)];

	if (ch == 'C')  //���������ַ���C��($GPRMC)
	{
		if (status == 'A')  //���������Ч�������
		{
			GPS -> NS       = buf[GetComma(4, buf)];
			GPS -> EW       = buf[GetComma(6, buf)];

			GPS->latitude   = Get_Double_Number(&buf[GetComma(3, buf)]);
			GPS->longitude  = Get_Double_Number(&buf[GetComma( 5, buf)]);

       	    GPS->latitude_Degree  = (int)GPS->latitude / 100;       //����γ��
			lati_cent_tmp         = (GPS->latitude - GPS->latitude_Degree * 100);
			GPS->latitude_Cent    = (int)lati_cent_tmp;
			lati_second_tmp       = (lati_cent_tmp - GPS->latitude_Cent) * 60;
			GPS->latitude_Second  = (int)lati_second_tmp;

			GPS->longitude_Degree = (int)GPS->longitude / 100;	//���뾭��
			long_cent_tmp         = (GPS->longitude - GPS->longitude_Degree * 100);
			GPS->longitude_Cent   = (int)long_cent_tmp;    
			long_second_tmp       = (long_cent_tmp - GPS->longitude_Cent) * 60;
			GPS->longitude_Second = (int)long_second_tmp;
		
			GPS->D.hour    = (buf[7] - '0') * 10 + (buf[8] - '0');		//ʱ��
			GPS->D.minute  = (buf[9] - '0') * 10 + (buf[10] - '0');
			GPS->D.second  = (buf[11] - '0') * 10 + (buf[12] - '0');
			tmp = GetComma(9, buf);
			GPS->D.day     = (buf[tmp + 0] - '0') * 10 + (buf[tmp + 1] - '0'); //����
			GPS->D.month   = (buf[tmp + 2] - '0') * 10 + (buf[tmp + 3] - '0');
			GPS->D.year    = (buf[tmp + 4] - '0') * 10 + (buf[tmp + 5] - '0')+2000;

			UTC2BTC(&GPS->D);
			
			return 1;
		}		
	}
	
	return 0;
}

//====================================================================//
// �﷨��ʽ��int GPS_GSV_Parse(char *line, GPS_INFO *GPS)  
// ʵ�ֹ��ܣ���gpsģ���GPGSV��Ϣ����Ϊ��ʶ�������
// ��    �������ԭʼ��Ϣ�ַ����顢�洢��ʶ�����ݵĽṹ��
// �� �� ֵ��
//			 1: ����GPGGA���
//           0: û�н��н�������������Ч
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
// �﷨��ʽ��void UTC2BTC(DATE_TIME *GPS)
// ʵ�ֹ��ܣ�ת��ʱ��Ϊ����ʱ����ʱ��
// ��    �������ʱ��Ľṹ��
// �� �� ֵ����
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


