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


const char* BatteryTypeToString(BatteryType b) {
	switch(b)
	{
	case Lithium: return "Lithium";
	case Lifepo4: return "Lifepo4";
	case AAx3: 	  return "AAx3";
	default: throw std::out_of_range("The BatteryType enumerator used with unknow value");
	}
}

std::ostream& operator<<(std::ostream& os, BatteryType e)
{
	switch(e)
	{
		case Lithium : os << "Lithium";    	break;
		case Lifepo4 : os << "Lifepo4";    	break;
		case AAx3   : os << "AAx3";       break;
		default	   : throw std::out_of_range("The BatteryType enumerator used with unknow value");
	}
	return os;
}



BatteryMGM::BatteryMGM(std::string name) :SavingInterfaceClass(name) {

	gpio_num_t adc_gpio_num;
	esp_err_t r;
	r = adc2_pad_get_io_num( channel, &adc_gpio_num );
	assert( r == ESP_OK );

	cout << "ADC Channel " << adc_gpio_num << endl;
	adc2_config_channel_atten( channel, ADC_ATTEN_0db );

	upperLimit = 4200;
	lowerLimit = 0;
	batt = BatteryType::Lithium;

}

BatteryMGM::~BatteryMGM() {
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


void BatteryMGM::Load(cJSON * p_json) {
	upperLimit = cJSON_GetObjectItem(p_json,"upperLimit")->valueint;
	lowerLimit = cJSON_GetObjectItem(p_json,"lowerLimit")->valueint;
	batt = (BatteryTypeEnumMap.find( cJSON_GetObjectItem(p_json,"BatteryType")->valuestring))->second;
}

cJSON* BatteryMGM::Save() {
	cJSON *batteryObject;
	batteryObject = cJSON_CreateObject();
	cJSON_AddNumberToObject(batteryObject, "upperLimit", upperLimit);
	cJSON_AddNumberToObject(batteryObject, "lowerLimit", lowerLimit);
	cJSON_AddStringToObject(batteryObject, "BatteryType", BatteryTypeToString(batt));
	return batteryObject;
}
