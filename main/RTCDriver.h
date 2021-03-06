/*
 * RTCDriver.h
 *
 *  Created on: 2022. jún. 26.
 *      Author: zilizii
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
#define ADDRESS_RTC   			0x51
#define RTC_BIAS_YEAR			2000

// Register Addresses
#define REG_ADDR_ALL			0x00
#define REG_ADDR_CONTROL1		0x00
#define REG_ADDR_CONTROL2		0x01
#define REG_ADDR_OFFSET			0x02
#define REG_ADDR_RAM			0x03
#define REG_ADDR_TIME			0x04
#define REG_ADDR_SECS			0x04
#define REG_ADDR_MINS			0x05
#define REG_ADDR_HOURS			0x06
#define REG_ADDR_DATE			0x07
#define REG_ADDR_WEEKDAY		0x08
#define REG_ADDR_MONTH			0x09
#define REG_ADDR_YEAR			0x0A

#define REG_ADDR_TIMER_VALUE	0x10
#define REG_ADDR_TIMER_MODE		0x11

//Filter default values
#define FILTER_SECS				0x7F
#define FILTER_MINS				0x7F
#define FILTER_HOURS			0x7F
#define FILTER_DATE				0x3F
#define FILTER_WEEKDAY			0x03
#define FILTER_MONTH			0x1F
#define FILTER_YEAR				0xFF

#define FILTER_TIMER_VALUE		0xFF

#define EPOCH_YEAR				1900
#define EPOCH_BIAS_MONTH		1


/*
 * Stucture to store the RTC values in memory, Coded with BCD
 * */

typedef struct  __attribute__ ((packed)) {
	uint8_t	Control1;		// 0x00
	uint8_t	Control2;		// 0x01
	uint8_t	Offset;			// 0x02
	uint8_t RAM;			// 0x03
	uint8_t Seconds;    	// 0x04
	uint8_t Minutes;		// 0x05
	uint8_t Hours;			// 0x06
	uint8_t Date;			// 0x07
	uint8_t Weekday;		// 0x08
	uint8_t Month;			// 0x09
	uint8_t Year;			// 0x0A
	uint8_t Seconds_Alarm;	// 0x0B
	uint8_t Minutes_Alarm;	// 0x0C
	uint8_t Hours_Alarm;	// 0x0D
	uint8_t Date_Alarm;		// 0x0E
	uint8_t Weekday_Alarm;	// 0x0F
	uint8_t Timer_Value;	// 0x10
	uint8_t Timer_Mode;		// 0x11
}_ttime;

//esp_err_t i2c_master_read_slave (i2c_port_t i2c_num, uint8_t Address, uint8_t Offset, uint8_t *data_rd, size_t size)
//esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t Address, uint8_t Offset ,uint8_t *data_wr, size_t size)
typedef int32_t (*fncPntr)(int,uint8_t,uint8_t,uint8_t *, size_t );


class RTCDriver {
private:
	SemaphoreHandle_t  * smph;
	uint8_t intToBCD(uint8_t num);
	uint8_t bcdToInt(uint8_t bcd);
	fncPntr _fp_writei2c = nullptr;
	fncPntr _fp_readi2c = nullptr;
public:
	_ttime sttime;
	//RTCDriver(SemaphoreHandle_t *);
	RTCDriver(SemaphoreHandle_t *, fncPntr readI2CFnc, fncPntr writeI2CFnc);
	esp_err_t readAllRegsFromRTC(void);
	esp_err_t writeAllRegsToRTC(void);
	esp_err_t writeTimeToRTC(void);
	esp_err_t readTimeFromRTC(void);
	long getEpoch(void);
	void updateTimeFromEpoch(long);
	esp_err_t writeTimeFromEpochToRTC(long);
	bool isTimerWakeUp(bool);
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

	// Timer Functionality supporting implementation
	esp_err_t readTimerValueFromRTC(uint8_t *);
	esp_err_t writeTimerValueToRTC(uint8_t);
	esp_err_t readTimerModeFromRTC(uint8_t *);
	esp_err_t writeTimerModeToRTC(uint8_t);

	// RAM byte implementation
	esp_err_t readRAMFromRTC(uint8_t *);
	esp_err_t writeRAMToRTC(uint8_t);
	virtual ~RTCDriver();
};

#endif /* MAIN_RTCDRIVER_H_ */
