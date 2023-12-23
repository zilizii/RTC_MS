/*
 * BatteryMGM.h
 *
 *  Created on: 2023. júl. 3.
 *      Author: Dell
 */

#ifndef MAIN_BATTERYMGM_H_
#define MAIN_BATTERYMGM_H_

#define DEFAULT_VREF    1100
#define NO_OF_SAMPLES   64

#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <iostream>
#include "HW_setup.h"
#include "SavingInterfaceClass.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_system.h"

enum BatteryType {
	Lithium, Lifepo4, AAx3
};

const char* BatteryTypeToString(BatteryType b);
std::ostream& operator<<(std::ostream &os, BatteryType e);
static std::map<std::string, BatteryType> BatteryTypeEnumMap = { { "Lithium",
		BatteryType::Lithium }, { "Lifepo4", BatteryType::Lifepo4 }, { "AAx3",
		BatteryType::AAx3 } };

class BatteryMGM: public SavingInterfaceClass {
private:
	uint16_t upperLimit;
	uint16_t lowerLimit;
	BatteryType batt;
public:
	BatteryMGM(std::string name);
	virtual ~BatteryMGM();
	int readADC();
	int getBatteryVoltage();
	cJSON* Save();
	void Load(cJSON*);
};

#endif /* MAIN_BATTERYMGM_H_ */
