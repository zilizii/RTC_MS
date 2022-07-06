/*
 * RTCDriver.h
 *
 *  Created on: 2022. jún. 26.
 *      Author: Dell
 */
using namespace std;

#ifndef MAIN_RTCDRIVER_H_
#define MAIN_RTCDRIVER_H_
#include <cstring>
#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "i2c_cmd.h"

// i2c Address
#define ADDRESS_RTC   		0x51
#define RTC_BIAS_YEAR		2000

// Register Addresses
#define REG_ADDR_RAM		0x03
#define REG_ADDR_ALL		0x04
#define REG_ADDR_SECS		0x04
#define REG_ADDR_MINS		0x05
#define REG_ADDR_HOURS		0x06
#define REG_ADDR_DATE		0x07
#define REG_ADDR_WEEKDAY	0x08
#define REG_ADDR_MONTH		0x09
#define REG_ADDR_YEAR		0x0A

//Filter default values
#define FILTER_SECS			0x7F
#define FILTER_MINS			0x7F
#define FILTER_HOURS        0x7F
#define FILTER_DATE			0x3F
#define FILTER_WEEKDAY		0x03
#define FILTER_MONTH		0x1F
#define FILTER_YEAR			0xFF

#define EPOCH_YEAR			1900
#define EPOCH_BIAS_MONTH	1


/*
 * Stucture to store the RTC values in memory, Coded with BCD
 * */

typedef struct  __attribute__ ((packed)) {
	uint8_t Seconds;    // 0x04
	uint8_t Minutes;	// 0x05
	uint8_t Hours;		// 0x06
	uint8_t Date;		// 0x07
	uint8_t Weekday;	// 0x08
	uint8_t Month;		// 0x09
	uint8_t Year;		// 0x0A
}_ttime;

class RTCDriver {
private:
	SemaphoreHandle_t  * smph;
	uint8_t _24_12 			= 0;
	uint8_t intToBCD(uint8_t num);
	uint8_t bcdToInt(uint8_t bcd);
//	string unixTimeToHumanReadable(long int seconds)

public:
	_ttime sttime;
	RTCDriver(SemaphoreHandle_t *);
	esp_err_t readTimeFromRTC(void);
	esp_err_t writeTimeToRTC(void);
	long getEpoch(void);
	void updateTimeFromEpoch(long);
	esp_err_t writeTimeFromEpochToRTC(long);
//	esp_err_t writeTimeBasedonEpoc(long epoch);
	esp_err_t writeYearToRTC(uint16_t);
	esp_err_t readYearFromRTC(uint16_t *);
	esp_err_t writeMonthToRTC(uint8_t);
	esp_err_t readMonthFromRTC(uint8_t *);
	esp_err_t writeDateToRTC(uint8_t);
	esp_err_t readDateFromRTC(uint8_t *);
	esp_err_t writeSecondsToRTC(uint8_t);
	esp_err_t readSecondsFromRTC(uint8_t *);
	esp_err_t writeMinutesToRTC(uint8_t);
	esp_err_t readMinutesFromRTC(uint8_t *);
	esp_err_t writeHoursToRTC(uint8_t);
	esp_err_t readHoursFromRTC(uint8_t *);
	virtual ~RTCDriver();
};

#endif /* MAIN_RTCDRIVER_H_ */
