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
	switch(b) {
	case Lithium: return "Lithium";
	case Lifepo4: return "Lifepo4";
	case AAx3: 	  return "AAx3";
	default: throw std::out_of_range("The BatteryType enumerator used with unknow value");
	}
}

std::ostream& operator<<(std::ostream& os, BatteryType e)
{
	switch(e) {
	case Lithium : os << "Lithium";    	break;
	case Lifepo4 : os << "Lifepo4";    	break;
	case AAx3   : os << "AAx3";       break;
	default	   : throw std::out_of_range("The BatteryType enumerator used with unknow value");
	}
	return os;
}



BatteryMGM::BatteryMGM(std::string name) :SavingInterfaceClass(name) {

	esp_err_t r;

#if (BATTERY_ADC_NUM == 1)
	adc_oneshot_unit_init_cfg_t init_config = {
			.unit_id = ADC_UNIT_1,
	};

#else
	adc_oneshot_unit_init_cfg_t init_config = {
			.unit_id = ADC_UNIT_2,
	};
#endif

	adc_oneshot_chan_cfg_t config = {
			.atten = ADC_ATTEN_DB_0,
			.bitwidth = ADC_BITWIDTH_12,
	};

	ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, channel, &config));

	assert( r == ESP_OK );

	upperLimit = 4200;
	lowerLimit = 0;
	batt = BatteryType::Lithium;

}

BatteryMGM::~BatteryMGM() {
	ESP_ERROR_CHECK(adc_oneshot_del_unit(adc_handle));
}

int BatteryMGM::readADC(void) {
	esp_err_t r;
	int read_raw = 0;

	r = adc_oneshot_read(adc_handle, channel, &read_raw);

	if ( r == ESP_OK ) {
		return read_raw;
	} else if ( r == ESP_ERR_TIMEOUT )
	{
#if (BATTERY_ADC_NUM == 2)
		cout << "Wifi is running ";
#else
		cout << "Time OUT ERROR " ;
#endif
	}
	return -1;
}

int BatteryMGM::getBatteryVoltage() {
	uint16_t v = this->readADC();
	float batteryVoltage = ((float)v / 4095.0) * 5.7 * 1100.0;
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
