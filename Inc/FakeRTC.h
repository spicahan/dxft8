/*
 * FakeRTC.h
 *
 *  Created on: Dec 25, 2019
 *      Author: user
 */

#ifndef FAKERTC_H_
#define FAKERTC_H_

#include <stdint.h>

typedef struct
{
	const char *Name;
	const unsigned char Minimum;
	const unsigned char Maximum;
	int8_t data;
} RTCStruct;

extern RTCStruct s_RTC_Data[];

extern int RTC_Set_Flag;

#define RTC_STRING_SIZE 13
#define FILENAME_STRING_SIZE 24

extern char log_rtc_time_string[RTC_STRING_SIZE];
extern char log_rtc_date_string[RTC_STRING_SIZE];
extern char file_name_string[FILENAME_STRING_SIZE];

void getTime(unsigned char *hour, unsigned char *minute, unsigned char *second,
		short *am_pm);
void getDate(unsigned char *day_of_week, unsigned char *date,
		unsigned char *month, unsigned char *year);

void RTC_setTime(unsigned char hSet, unsigned char mSet, unsigned char sSet);
void RTC_setDate(unsigned char daySet, unsigned char dateSet,
		unsigned char monthSet, unsigned char yearSet);

void display_RealTime(int x, int y);
void display_Real_Date(int x, int y);

void make_Real_Time(void);
void make_Real_Date(void);
void make_File_Name(void);

void load_RealTime(void);
void display_RTC_TimeEdit(int x, int y);
void display_RTC_DateEdit(int x, int y);
void set_RTC_to_TimeEdit(void);
void set_RTC_to_DateEdit(void);

void load_RealDate(void);

#endif /* FAKERTC_H_ */
