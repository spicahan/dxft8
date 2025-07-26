/*
 * DS3231.c
 *
 *  Created on: Dec 25, 2019
 *      Author: user
 */

#include "FakeRTC.h"
#include "Display.h"
#include "main.h"
#include "log_file.h"

char log_rtc_time_string[RTC_STRING_SIZE];
char log_rtc_date_string[RTC_STRING_SIZE];

static unsigned char rtc_hour, rtc_minute, rtc_second, rtc_dow, rtc_date, rtc_month,
		rtc_year;
static short rtc_ampm;

char file_name_string[FILENAME_STRING_SIZE];

RTCStruct s_RTC_Data[] = { {
/*Name*/"  Day ", //opt0
		/*Min */1,
		/*Max */31,
		/*Data*/0, },

{
/*Name*/"Month", //opt1
		/*Min */1,
		/*Max */12,
		/*Data*/0, },

{
/*Name*/"Year", //opt2
		/*Min */24,
		/*Max */99,
		/*Data*/0, },

{
/*Name*/"Hour", //opt3
		/*Min */0,
		/*Max */23,
		/*Data*/0, },

{
/*Name*/"Minute", //opt4

		/*Min */0,
		/*Max */59,
		/*Data*/0, },

{
/*Name*/"Second", //opt5
		/*Min */0,
		/*Max */59,
		/*Data*/0, }

};

unsigned char bcd_to_decimal(unsigned char d) {
	return ((d & 0x0F) + (((d & 0xF0) >> 4) * 10));
}

unsigned char decimal_to_bcd(unsigned char d) {
	return (((d / 10) << 4) & 0xF0) | ((d % 10) & 0x0F);
}

void getTime(unsigned char *p3, unsigned char *p2, unsigned char *p1, short *p0)
{
	(void)p0;
	uint32_t seconds = current_time / 1000;
	*p1 = seconds % 60;
	*p2 = seconds / 60 % 60;
	*p3 = seconds / 3600 % 24;
}

void getDate(unsigned char *p4, unsigned char *p3, unsigned char *p2,
		unsigned char *p1) {
	// TODO
	*p1 = 25;
	*p2 = 7;
	*p3 = 26;
	*p4 = 0;
}

void RTC_setTime(unsigned char hSet, unsigned char mSet, unsigned char sSet) {
	(void)hSet;
	(void)mSet;
	(void)sSet;
}

void RTC_setDate(unsigned char daySet, unsigned char dateSet,
		unsigned char monthSet, unsigned char yearSet) {
	// TODO
	(void)daySet;
	(void)dateSet;
	(void)monthSet;
	(void)yearSet;
}

void display_RealTime(int x, int y) {
	static int cnt = 0;
	cnt++;
	// reduce DS3231 time polling frequency
	if (cnt % 5 != 0) {
		return;
	}
	// fetch time from RTC
	getTime(&rtc_hour, &rtc_minute, &rtc_second, &rtc_ampm);
	show_UTC_time(x, y, rtc_hour, rtc_minute, rtc_second, 0);
	// further reduce date polling frequency
	if (cnt % 25 != 0) {
		return;
	}
	unsigned char old_rtc_year = rtc_year;
	unsigned char old_rtc_month = rtc_month;
	unsigned char old_rtc_date = rtc_date;
	getDate(&rtc_dow, &rtc_date, &rtc_month, &rtc_year);
	if (rtc_date != old_rtc_date || rtc_month != old_rtc_month || rtc_year != old_rtc_year) {
		display_Real_Date(0, 240);
		Init_Log_File();
	}
}

void load_RealTime(void) {
	getTime(&rtc_hour, &rtc_minute, &rtc_second, &rtc_ampm);
	s_RTC_Data[3].data = rtc_hour;
	s_RTC_Data[4].data = rtc_minute;
	s_RTC_Data[5].data = rtc_second;
}

void display_RTC_TimeEdit(int x, int y) {
	show_UTC_time(x, y, s_RTC_Data[3].data, s_RTC_Data[4].data,
			s_RTC_Data[5].data, 0);
}

void set_RTC_to_TimeEdit(void) {
	RTC_setTime(s_RTC_Data[3].data, s_RTC_Data[4].data, s_RTC_Data[5].data);
}

void load_RealDate(void) {
	getDate(&rtc_dow, &rtc_date, &rtc_month, &rtc_year);
	if (rtc_date > 0)
		s_RTC_Data[0].data = rtc_date;
	else
		s_RTC_Data[0].data = rtc_date = 1;

	if (rtc_month > 0)
		s_RTC_Data[1].data = rtc_month;
	else
		s_RTC_Data[1].data = 1;

	if (rtc_year >= 24)
		s_RTC_Data[2].data = rtc_year;
	else
		s_RTC_Data[2].data = 1;
}

void display_RTC_DateEdit(int x, int y) {
	show_Real_Date(x, y, s_RTC_Data[0].data, s_RTC_Data[1].data,
			s_RTC_Data[2].data);
}

void set_RTC_to_DateEdit(void) {
	RTC_setDate(0, s_RTC_Data[0].data, s_RTC_Data[1].data, s_RTC_Data[2].data);
}

void display_Real_Date(int x, int y) {
	getDate(&rtc_dow, &rtc_date, &rtc_month, &rtc_year);
	show_Real_Date(x, y, rtc_date, rtc_month, rtc_year);
}

void make_Real_Time(void) {

	getTime(&rtc_hour, &rtc_minute, &rtc_second, &rtc_ampm);
	sprintf(log_rtc_time_string, "%02i%02i%02i", rtc_hour, rtc_minute,
			rtc_second);
}

void make_Real_Date(void) {

	getDate(&rtc_dow, &rtc_date, &rtc_month, &rtc_year);
	sprintf(log_rtc_date_string, "20%02i%02i%02i", rtc_year,
			rtc_month, rtc_date);
}

void make_File_Name(void) {

	make_Real_Date();
	sprintf(file_name_string, "%s.adi", log_rtc_date_string);
}

void RTC_SetValue(int Idx, char newValue) {
	s_RTC_Data[Idx].data = newValue;
}
