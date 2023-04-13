/** @file diagnostic.h
 *  @brief Function handling the date and time update wi-Fi module and inverter.
 *
 *  @author Duraisamy Pandurangan (V-Guard Industrial Ltd,R&D)
 *  @Created Dec 29,2017
 */

#ifndef DIAGNOSTIC_H_
#define DIAGNOSTIC_H_

#include <freertos/task.h>
#include <freertos/FreeRTOS.h>
#include <time.h>
#include <sys/time.h>
#include "sntp.h"
#include "lwip/err.h"
/*Function Declaration*/
void Get_NetTime();
static void initialize_sntp(void);
void Set_DUPSTime(int Invert_time);
void setDate(const char *dataStr, int update);
void SetSystem_time(int Time);
/************** RTC variable declaration**********/
time_t now;
struct tm timeinfo;/*!<Create the time variable for RTC global Access */

static int Hours = 0, Sleep_Min = 9, Ble_Wake; /*!< Battery power save profile switching  interval time is Set 10 Mins*/

/**
 *@brief Set the WiFi module time
 *@param datastr update time from the server ,Format is YYYYMMDDHHSSmm
 *@param update is future reference
 *@return void
 */
void setDate(const char *dataStr, int update) {
	char buf[5] = { 0 };
	/*Seperate YYYY from the dataStr*/
	strncpy(buf, dataStr + 0, 4);
	unsigned short year = atoi(buf);
	memset(buf, 0, 5);

	/*Seperate MM from the dataStr*/
	strncpy(buf, dataStr + 4, 2);
	unsigned short month = atoi(buf);
	memset(buf, 0, 5);

	/*Seperate DD from the dataStr*/
	strncpy(buf, dataStr + 6, 2);
	unsigned short day = atoi(buf);
	memset(buf, 0, 5);

	/*Seperate Hours(HH) from the dataStr*/
	strncpy(buf, dataStr + 8, 2);
	unsigned short hour = atoi(buf);
	memset(buf, 0, 5);

	/*Seperate Mins(MM) from the dataStr*/
	strncpy(buf, dataStr + 10, 2);
	unsigned short minutes = atoi(buf);
	memset(buf, 0, 5);

	/*Seperate Sec(SS) from the dataStr*/
	strncpy(buf, dataStr + 12, 2);
	unsigned short seconds = atoi(buf);
	memset(buf, 0, 5);

	time_t mytime = time(0);
	struct tm *tm_ptr = localtime(&mytime);

	/*Convert to standard time format*/
	if (tm_ptr) {
		tm_ptr->tm_sec = seconds;
		tm_ptr->tm_min = minutes;
		tm_ptr->tm_hour = hour;
		tm_ptr->tm_year = year - 1900;
		tm_ptr->tm_mon = month - 1;
		tm_ptr->tm_mday = day;

		/*Set Wi-Fi RTC*/
		const struct timeval tv = { mktime(tm_ptr), 0 };
		settimeofday(&tv, 0);
	}
	/*Get the local date and time*/
	time(&now);
	localtime_r(&now, &timeinfo);

	char strftime_buf[64];
	/*Display update date and time*/
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	ESP_LOGI("Date_Time", "The current date/time is: %s", strftime_buf);

	return;
}

/**
 * @brief Date_time Return maintenance activity time interval.
 * @return type is integer
 */

int Date_timeNet() {
	time(&now);
	localtime_r(&now, &timeinfo);
	if ((timeinfo.tm_sec > 55 && timeinfo.tm_sec <= 58)
			&& (timeinfo.tm_min == 00)) {
		return 2;
	}
		else if ((timeinfo.tm_sec > 55 && timeinfo.tm_sec <= 58)
					&& (timeinfo.tm_min == 55)) { //System maintenance Activity slot Time
		return 1;
	}
//		 else if ((timeinfo.tm_sec > 55 && timeinfo.tm_sec <= 58)
//					&& (timeinfo.tm_min%5 == 0)) { //System maintenance Activity slot Time
//
//		 }

		else {
		return 0;
	}
	return 0;
}



#endif //DIAGNOSTIC_H
