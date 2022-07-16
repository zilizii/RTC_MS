/* C++ exception handling example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <driver/gpio.h>
#include <esp_attr.h>
#include <esp_err.h>
#include <esp_event_legacy.h>
#include <esp_interface.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp32/rom/ets_sys.h>
#include <esp32/rom/gpio.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>
#include "freertos/semphr.h"
#include <hal/gpio_types.h>
#include <nvs_flash.h>
#include <sys/_stdint.h>
#include <tcpip_adapter.h>
#include <esp_log.h>
#include <cmath>
#include <cstring>
#include "RTCDriver.h"
#include <iostream>
#include "sdkconfig.h"

using std::cout;
using std::endl;
using std::runtime_error;


static const char *TAG = "RT_MS";

/* A simple class which may throw an exception from constructor */
class Throwing
{
public:
    Throwing(int arg)
    : m_arg(arg)
    {
        cout << "In constructor, arg=" << arg << endl;
        if (arg == 0) {
            throw runtime_error("Exception in constructor");
        }
    }

    ~Throwing()
    {
        cout << "In destructor, m_arg=" << m_arg << endl;
    }

protected:
    int m_arg;
};

/*static uint8_t intToBCD(uint8_t num) {
	return ((num / 10) << 4) | (num%10);
}

static uint8_t bcdToInt(uint8_t bcd) {
	// 0x10
	return ((bcd >> 4) * 10) + (bcd & 0x0f);;
}*/
SemaphoreHandle_t i2c_mutex;
//RTCDriver ooo;

/* Inside .cpp file, app_main function must be declared with C linkage */
extern "C" void app_main(void)
{
	esp_err_t ret;
	uint16_t year = 0;
	uint8_t month =0,date =0,hour =0,minute=0,second=0;
	nvs_flash_init();
	i2c_mutex = xSemaphoreCreateBinary();
	  if( i2c_mutex == NULL )
	    {
		  ESP_LOGI(TAG, "I2C Mutex creation  unsuccesfull");
	    }
	if( xSemaphoreGive( i2c_mutex ) != pdTRUE )
	{
	    // We would expect this call to fail because we cannot give
	    // a semaphore without first "taking" it!
	}
    ret = i2c_master_init();
	if(ret != ESP_OK)
		 cout<< "i2c driver install failed" << endl;
	RTCDriver * ooo = new RTCDriver(&i2c_mutex, &i2c_master_read_slave, &i2c_master_write_slave);
	ret = ooo->readAllRegsFromRTC();
	if(ret != ESP_OK){
			 cout<< "i2c Read Failed" << endl;
	}
	cout << "app_main starting" << endl;
	/*ooo->writeYearToRTC(2022);
	ooo->writeMonthToRTC(7);
	ooo->writeDateToRTC(3);*/
	//Timer Trial every 5 sec a
	ooo->writeTimerValueToRTC(5);
	ooo->writeTimerModeToRTC(0b10111);

	while(true) {
		 /*ret = ooo->updateTimeFromRTC();
		 if(ret != ESP_OK)
     	 		 cout<< "i2c Read Failed";
		 printf("Time: %.2d:%.2d:%.2d \n", bcdToInt(ooo->sttime.Hours), bcdToInt(ooo->sttime.Minutes), bcdToInt(ooo->sttime.Seconds & FILTER_SECS) );*/

		 ooo->readHoursFromRTC(&hour);
		 ooo->readMinutesFromRTC(&minute);
		 ooo->readSecondsFromRTC(&second);
		 printf("Time: %.2d:%.2d:%.2d \n", hour,minute,second );
		 ooo->readYearFromRTC(&year);
		 ooo->readMonthFromRTC(&month);
		 ooo->readDateFromRTC(&date);
		 printf("Date: %d-%.2d-%.2d \n", year, month, date);
		 //printf("EPOCH : %ld \n", ooo->getEpoch());
		 //ooo->updateTimeFromEpoch(ooo->getEpoch());
		 ooo->printAllRegs(true);
		 vTaskDelay( 1000 / portTICK_PERIOD_MS );
	 }



    try {

        /* This will succeed */
        Throwing obj1(42);

        /* This will throw an exception */
        Throwing obj2(0);

        cout << "This will not be printed" << endl;
    } catch (const runtime_error &e) {
        cout << "Exception caught: " << e.what() << endl;
    }

    cout << "app_main done" << endl;
}
