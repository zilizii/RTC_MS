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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_system.h"


enum BatteryType { Lithium, Lifepo4, AAx3};


class BatteryMGM {
private:
	int upperLimit;
	int lowerLimit;
    BatteryType batt;
public:
	BatteryMGM();
	virtual ~BatteryMGM();
	int readADC();
	int getBatteryVoltage();
};

#endif /* MAIN_BATTERYMGM_H_ */
