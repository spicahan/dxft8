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

static unsigned char rtc_hour, rtc_minute, rtc_second;
static unsigned char rtc_date = 26, rtc_month = 7, rtc_year = 25;
static unsigned char new_rtc_date = 26, new_rtc_month = 7, new_rtc_year = 25;

char file_name_string[FILENAME_STRING_SIZE];

static uint32_t rtc_offset = 0;

static void getTime();
static void advance_date();

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

void getTime()
{
	bool was_last_second = rtc_hour == 23 && rtc_minute == 59 && rtc_second == 59;
	uint32_t seconds = (current_time + rtc_offset) / 1000;
	rtc_second = seconds % 60;
	rtc_minute = seconds / 60 % 60;
	rtc_hour = seconds / 3600 % 24;
	bool first_second = seconds % 86400 == 0;
	if (was_last_second && first_second) {
		advance_date();
	}
}

static void advance_date()
{
	new_rtc_date++;
	
	unsigned char days_in_month = 31;
	if (rtc_month == 4 || rtc_month == 6 || rtc_month == 9 || rtc_month == 11) {
		days_in_month = 30;
	} else if (rtc_month == 2) {
		bool is_leap_year = ((rtc_year % 4) == 0) && (((rtc_year % 100) != 0) || ((rtc_year % 400) == 0));
		days_in_month = is_leap_year ? 29 : 28;
	}
	
	if (new_rtc_date > days_in_month) {
		new_rtc_date = 1;
		new_rtc_month++;
		if (new_rtc_month > 12) {
			new_rtc_month = 1;
			new_rtc_year++;
			if (new_rtc_year > 99) {
				new_rtc_year = 0;
			}
		}
	}
}

static void getDate() {
	rtc_date = new_rtc_date;
	rtc_month = new_rtc_month;
	rtc_year = new_rtc_year;
}

static void RTC_setTime(unsigned char hSet, unsigned char mSet, unsigned char sSet) {
	uint32_t cur_ms = current_time % 86400000;
	uint32_t new_ms = (hSet * 3600 + mSet * 60 + sSet) * 1000;
	rtc_offset = new_ms + 86400000 - cur_ms;
}

static void RTC_setDate(unsigned char dateSet, unsigned char monthSet, unsigned char yearSet) {
	new_rtc_date = dateSet;
	new_rtc_month = monthSet;
	new_rtc_year = yearSet;
}

void display_RealTime(int x, int y) {
	static int cnt = 0;
	cnt++;
	// reduce DS3231 time polling frequency
	if (cnt % 5 != 0) {
		return;
	}
	unsigned char old_rtc_year = rtc_year;
	unsigned char old_rtc_month = rtc_month;
	unsigned char old_rtc_date = rtc_date;
	// fetch time from RTC
	getTime();
	show_UTC_time(x, y, rtc_hour, rtc_minute, rtc_second, 0);
	getDate();
	if (rtc_date != old_rtc_date || rtc_month != old_rtc_month || rtc_year != old_rtc_year) {
		display_Real_Date(0, 240);
		Init_Log_File();
	}
}

void load_RealTime(void) {
	getTime();
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
	getDate();
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
	RTC_setDate(s_RTC_Data[0].data, s_RTC_Data[1].data, s_RTC_Data[2].data);
}

void display_Real_Date(int x, int y) {
	getDate();
	show_Real_Date(x, y, rtc_date, rtc_month, rtc_year);
}

void make_Real_Time(void) {
	getTime();
	sprintf(log_rtc_time_string, "%02i%02i%02i", rtc_hour, rtc_minute,
			rtc_second);
}

void make_Real_Date(void) {

	getDate();
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
