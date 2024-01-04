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
#include "hal/adc_types.h"
//

#if CONFIG_IDF_TARGET_ESP32

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

#define CONFIG_PATH "/spiffs/config.txt"

#if defined(CONFIG_SELECT_MANUAL_CONFIG)
	#define MCU_ON CONFIG_MCU_ON_GPIO
	#define WAKE_UP_GPIO CONFIG_WAKE_UP_GPIO
	#define REED_SWITCH CONFIG_REED_SWITCH
	#define ON_BOARD_LED CONFIG_ON_BOARD_LED
	#define BATTERY_ADC_CHANNEL CONFIG_BATTERY_ADC_CHANNEL;
    #define BATTERY_ADC_NUM CONFIG_BATTERY_ADC_NUM
	static const adc_channel_t channel = (adc_channel_t)BATTERY_ADC_CHANNEL;

#elif defined(CONFIG_SELECT_HW_0_1)
	static const adc_channel_t channel = ADC_CHANNEL_7;
    #define BATTERY_ADC_NUM 2
	#define MCU_ON 17
	#define WAKE_UP_GPIO 19
	#define REED_SWITCH 16
	#define ON_BOARD_LED 18
#elif defined(CONFIG_SELECT_HW_0_2)
	static const adc_channel_t channel = ADC_CHANNEL_7;
	#define BATTERY_ADC_NUM 1
    #define MCU_ON 17
	#define WAKE_UP_GPIO 19
	#define REED_SWITCH 16
	#define ON_BOARD_LED 18
#endif

#endif /* MAIN_HW_SETUP_H_ */
