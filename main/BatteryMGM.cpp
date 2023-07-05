/*
 * BatteryMGM.cpp
 *
 *  Created on: 2023. júl. 3.
 *      Author: Dell
 */

#include "BatteryMGM.h"

using std::cout;
using std::endl;
using std::runtime_error;

BatteryMGM::BatteryMGM() {
	// TODO Auto-generated constructor stub
	gpio_num_t adc_gpio_num;
	esp_err_t r;
	r = adc2_pad_get_io_num( channel, &adc_gpio_num );
	assert( r == ESP_OK );

	cout << "ADC Channel " << adc_gpio_num << endl;
	adc2_config_channel_atten( channel, ADC_ATTEN_0db );

	upperLimit = 4200;
	lowerLimit = 0;

}

BatteryMGM::~BatteryMGM() {
	// TODO Auto-generated destructor stub
}

int BatteryMGM::readADC(void) {
	esp_err_t r;
	int read_raw = 0;
	r = adc2_get_raw( channel, ADC_WIDTH_12Bit, &read_raw);
	if ( r == ESP_OK ) {
		return read_raw;
	}
	return -1;
}

int BatteryMGM::getBatteryVoltage() {
	return (this->upperLimit - this->lowerLimit) / 4095 * this->readADC();
}

