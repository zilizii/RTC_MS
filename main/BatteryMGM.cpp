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

static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;


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

#if (CONFIG_BATTERY_ADC_NUM == 1) || (BATTERY_ADC_NUM == 1)
		r = adc1_pad_get_io_num(channel, &adc_gpio_num);
#else
		r = adc2_pad_get_io_num(channel, &adc_gpio_num);
#endif
	assert( r == ESP_OK );

	cout << "ADC Channel " << adc_gpio_num << endl;

#if (CONFIG_BATTERY_ADC_NUM == 1) || (BATTERY_ADC_NUM == 1)
	    adc1_config_channel_atten( channel, ADC_ATTEN_0db );
#else
		adc2_config_channel_atten( channel, ADC_ATTEN_0db );
#endif



	upperLimit = 4200;
	lowerLimit = 0;
	batt = BatteryType::Lithium;

}

BatteryMGM::~BatteryMGM() {
}

int BatteryMGM::readADC(void) {
	esp_err_t r;
	int read_raw = 0;

#if (CONFIG_BATTERY_ADC_NUM == 1) || (BATTERY_ADC_NUM == 1)
		//r = adc1_get_raw( channel, ADC_WIDTH_12Bit, &read_raw);
	r = adc1_config_width(ADC_WIDTH_BIT_12);
	r |= adc1_config_channel_atten(channel, atten);
	read_raw = adc1_get_raw( channel);
#else
		r = adc2_get_raw( channel, ADC_WIDTH_12Bit, &read_raw);
#endif

	if ( r == ESP_OK ) {
		return read_raw;
	} else if ( r == ESP_ERR_TIMEOUT )
	{
#if (CONFIG_BATTERY_ADC_NUM == 2) || (BATTERY_ADC_NUM == 2)

		cout << "Wifi is running ";
#else
		cout << "Time OUT ERROR " ;
#endif
	}
	return -1;
}

int BatteryMGM::getBatteryVoltage() {
	uint16_t v = this->readADC();
	float batteryVoltage = ((float)v / 4095.0) * 5.7 * 1000.0;
	return (int)(batteryVoltage);
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
