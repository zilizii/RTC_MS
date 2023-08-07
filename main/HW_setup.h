/*
 * HW_setup.h
 *
 *  Created on: 2023. júl. 2.
 *      Author: Dell
 */

#ifndef MAIN_HW_SETUP_H_
#define MAIN_HW_SETUP_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#if CONFIG_IDF_TARGET_ESP32
#include "esp_adc_cal.h"
#endif

/*
 * Generic settings for GPIO
 *	Change only when the esp-idf changes
 */
#define HIGH 1
#define LOW 0
#define OUTPUT GPIO_MODE_OUTPUT
#define INPUT GPIO_MODE_INPUT

/*
 * HW dependent configuration
 * Please check the necessary Schematic corresponding to the HW variants
 */

static const adc2_channel_t channel = ADC2_CHANNEL_7;
#define CONFIG_PATH "/spiffs/config.txt"


#if defined(CONFIG_SELECT_MANUAL_CONFIG)
	#define MCU_ON CONFIG_MCU_ON_GPIO
	#define WAKE_UP_GPIO CONFIG_WAKE_UP_GPIO
	#define REED_SWITCH CONFIG_REED_SWITCH
	#define ON_BOARD_LED CONFIG_ON_BOARD_LED
	#define BATTERY_ADC_PIN CONFIG_BATTERY_ADC_PIN
#elif defined(CONFIG_SELECT_HW_0_1)
	#define MCU_ON 17
	#define WAKE_UP_GPIO 19
	#define REED_SWITCH 16
	#define ON_BOARD_LED 18
	#define BATTERY_ADC_PIN 27


#elif defined(CONFIG_SELECT_HW_0_2)
	#define MCU_ON 17
	#define WAKE_UP_GPIO 19
	#define REED_SWITCH 16
	#define ON_BOARD_LED 18
	#define BATTERY_ADC_PIN 27
#endif

#endif /* MAIN_HW_SETUP_H_ */
