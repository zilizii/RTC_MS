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
#include "driver/uart.h"
#include <freertos/portmacro.h>
#include <freertos/task.h>
#include "freertos/semphr.h"
#include <hal/gpio_types.h>
#include <nvs_flash.h>
#include <sys/_stdint.h>
#include <esp_netif.h>
#include <esp_log.h>
#include <cmath>
#include <cstring>
#include "RTCDriver.h"
#include <iostream>
#include "sdkconfig.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"

//#include "esp_event_loop.h"
#include "esp_event.h"


using std::cout;
using std::endl;
using std::runtime_error;

#define MAXIMUM_AP 20

static const char *TAG = "RT_MS";
static const int RX_BUF_SIZE = 256;
QueueHandle_t queueCommand;




/* A simple class which may throw an exception from constructor */
//class Throwing
//{
//public:
//    Throwing(int arg)
//    : m_arg(arg)
//    {
//        cout << "In constructor, arg=" << arg << endl;
//        if (arg == 0) {
//            throw runtime_error("Exception in constructor");
//        }
//    }
//
//    ~Throwing()
//    {
//        cout << "In destructor, m_arg=" << m_arg << endl;
//    }
//
//protected:
//    int m_arg;
//};


SemaphoreHandle_t i2c_mutex;
//RTCDriver ooo;
void RXtask(void * parameters);
void initUART(void);
//TaskHandle_t WifiSCANTaskt;
//void WifiSCANTask(void * parameters);

static char *auth_mode_type(wifi_auth_mode_t auth_mode)
{
  char *types[] = {"OPEN", "WEP", "WPA PSK", "WPA2 PSK", "WPA WPA2 PSK", "MAX"};
  return types[auth_mode];
}

static void scan_done_handler(void)
{
    uint16_t sta_number = 0;
    uint8_t i;
    wifi_ap_record_t *ap_list_buffer;

    esp_wifi_scan_get_ap_num(&sta_number);
    ap_list_buffer =  (wifi_ap_record_t *)(malloc(sta_number * sizeof(wifi_ap_record_t)));
    if (ap_list_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to malloc buffer to print scan results");
        return;
    }

    if (esp_wifi_scan_get_ap_records(&sta_number, (wifi_ap_record_t *)ap_list_buffer) == ESP_OK) {
        for (i = 0; i < sta_number; i++) {
            ESP_LOGI(TAG, "[%s][rssi=%d][MAC=%X:%X:%X:%X:%X:%X][%12s]", ap_list_buffer[i].ssid, ap_list_buffer[i].rssi, ap_list_buffer[i].bssid[0],ap_list_buffer[i].bssid[1],ap_list_buffer[i].bssid[2],ap_list_buffer[i].bssid[3],ap_list_buffer[i].bssid[4],ap_list_buffer[i].bssid[5], auth_mode_type(ap_list_buffer[i].authmode));
        }
    }
    free(ap_list_buffer);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data){
	//TODO: extend with events
	 switch (event_id) {
	    case WIFI_EVENT_SCAN_DONE:
	        scan_done_handler();
	        ESP_LOGI(TAG, "sta scan done");
	        break;
	    default:
	        break;
	    }
	    return;

}

/*
 * Wifi Init Function
 * Support to testing Scan functionality
 * old tcpip removed
 */

void wifiInit() {
	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	assert(sta_netif);
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
	wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());
}



/* Inside .cpp file, app_main function must be declared with C linkage */
extern "C" void app_main(void)
{
	esp_err_t ret;
	uint16_t year = 0;
	uint8_t month =0,date =0,hour =0,minute=0,second=0;
	uint8_t counter = 0;
	int qL = 0;
	bool bRTCWakeUpByTimer;

	wifiInit();

	_scommand x;
	initUART();
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
	queueCommand = ooo->getCommandQueue();
	ret = ooo->readAllRegsFromRTC();
	if(ret != ESP_OK){
			 cout<< "i2c Read Failed" << endl;
	}
	cout << "app_main starting" << endl;

	ooo->writeTimerValueToRTC(5);
	// TD 1/60Hz, TE Enabled, TIE Enabled,  TI_TP Enabled
	ooo->writeTimerModeToRTC(0b11111); // 0b11111
	xTaskCreate(RXtask, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);

	wifi_scan_config_t scan_config ;
	memset(&scan_config,0, sizeof(scan_config));
	scan_config.ssid = NULL;
	scan_config.bssid = NULL;
	scan_config.channel = 0;
	scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
	scan_config.show_hidden = true;
// Wake up reason check
	ooo->isTimerWakeUp(true,&bRTCWakeUpByTimer);
	cout << "Wake Up by Timer : " << bRTCWakeUpByTimer << endl;

	while(true) {

		 if(ooo->sttime[0x01] > 0 )
		 {
			 counter++;
			 if(counter > 1){
				 ooo->writeControl2Reg(FD_CLKOUT_LOW);
				 counter = 0;
			 }
		 }

		 qL = uxQueueMessagesWaiting(ooo->getCommandQueue());
		 for(int i = 0; i< qL; i++) {
			 xQueueReceive(ooo->getCommandQueue(),&x,portMAX_DELAY);
			 if (x.command[0] == 'S' && x.command[1] == 'E'){
				 ESP_LOGI("SET UTC EPOCH", "Read %d bytes: '%s' \n", sizeof(x.command)/sizeof(char), x.command);
				 ooo->writeTimeFromEpochToRTC(   (atol(x.command + 2)) );
			 }
			 else if (x.command[0] == 'G' && x.command[1] == 'E'){
				 ooo->readAllRegsFromRTC();
				 printf("GET UTC EPOCH : %ld \n", ooo->getEpochUTC() );
			 }
			 else if (x.command[0] == 'S' && x.command[1] == 'L'){
				 ESP_LOGI("SET Local EPOCH", "Read %d bytes: '%s' \n", sizeof(x.command)/sizeof(char), x.command);
				 ooo->writeTimeFromEpochToRTC(   (atol(x.command + 2)), false );
			 }
			 else if (x.command[0] == 'G' && x.command[1] == 'L'){
			 	 printf("GET LOCAL EPOCH : %ld \n", ooo->getEpoch() );
			 }

			 else if (x.command[0] == 'S' && x.command[1] == 'T') {
			 	 ESP_LOGI("SET TIME ZONE : ", "Read %d bytes: '%s' \n", sizeof(x.command)/sizeof(char), x.command);
			 	 ooo->setTimeZone( atoi(x.command + 2), true);
			 }
			 else if (x.command[0] == 'G' && x.command[1] == 'T') {
 				 printf("GET Time Zone : %d \n", ooo->getTimeZone());
			 }
			 else if (x.command[0] == 'W' && x.command[1] == 'S') {
				 	ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, false));
			 }
			 else if (x.command[0] == 'G' && x.command[1] == 'N') {
				 ESP_ERROR_CHECK(ooo->readHoursFromRTC(&hour));
				 ESP_ERROR_CHECK(ooo->readMinutesFromRTC(&minute));
				 ESP_ERROR_CHECK(ooo->readSecondsFromRTC(&second));
				 printf("Time: %.2d:%.2d:%.2d \n", hour,minute,second );
				 ESP_ERROR_CHECK(ooo->readYearFromRTC(&year));
				 ESP_ERROR_CHECK(ooo->readMonthFromRTC(&month));
				 ESP_ERROR_CHECK(ooo->readDateFromRTC(&date));
				 printf("Date: %d-%.2d-%.2d \n", year, month, date);
				 ESP_ERROR_CHECK(ooo->isTimerWakeUp(true,&bRTCWakeUpByTimer));
				 cout << "Wake Up by Timer : " << bRTCWakeUpByTimer << endl;
			 }
			 else if (x.command[0] == 'G' && x.command[1] == 'C') {
				 try {
				 uint8_t timerValue = 0;
				 uint8_t timerMode = 0;
				 ESP_ERROR_CHECK(ooo->readTimerValueFromRTC(&timerValue));
				 ESP_ERROR_CHECK(ooo->readTimerModeFromRTC(&timerMode));
				 //Conversion from number to enum value...
				 eTimeClockFreq val = static_cast<eTimeClockFreq>(TD_CHECK(timerMode));
				 cout << unsigned(timerValue) << " " << val << endl;
				 } catch (const std::range_error &e) {
					 cout << "Exception caught: " << e.what() << endl;
				 }
			 }
		 }

		 vTaskDelay( 10 / portTICK_PERIOD_MS );
	 }



//    try {
//
//        /* This will succeed */
//        Throwing obj1(42);
//
//        /* This will throw an exception */
//        Throwing obj2(0);
//
//        cout << "This will not be printed" << endl;
//    } catch (const runtime_error &e) {
//        cout << "Exception caught: " << e.what() << endl;
//    }

    cout << "app_main done" << endl;
}

void RXtask(void * parameters) {
	static const char *RX_TASK_TAG = "RX_TASK";
	esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
	_scommand rx;
	uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
	while (1) {
		const int rxBytes = uart_read_bytes(UART_NUM_0, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
	    if (rxBytes > 0) {
	    	data[rxBytes] = 0;
	    	strcpy(rx.command, (char *)data);
	    	xQueueSendToBack(queueCommand, &rx, portMAX_DELAY);
//	        ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
//	        ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
	    }
	}
	free(data);
}

void initUART(void) {

	const uart_config_t uart_config = {
	        .baud_rate = 115200,
	        .data_bits = UART_DATA_8_BITS,
	        .parity = UART_PARITY_DISABLE,
	        .stop_bits = UART_STOP_BITS_1,
	        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
			.rx_flow_ctrl_thresh = 122
	};
	if (uart_driver_install(UART_NUM_0, 2*1024, 0, 0, NULL, 0) != ESP_OK) {
		ESP_LOGE(TAG, "Driver installation failed");
		//vTaskDelete(NULL);
	}
	uart_param_config(UART_NUM_0, &uart_config);
}

/*void WifiSCANTask(void * parameters) {
	printf("WifiSCANTask Called\n");
	wifi_scan_config_t scan_config ;
	memset(&scan_config,0, sizeof(scan_config));



	scan_config.ssid = NULL;
	scan_config.bssid = NULL;
	scan_config.channel = 0;
	scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
	scan_config.show_hidden = true;

	ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, false));
	printf("WifiSCANTask done \n");
	wifi_ap_record_t wifi_records[MAXIMUM_AP];

	uint16_t max_records = MAXIMUM_AP;
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&max_records, wifi_records));


	printf("Number of Access Points Found: %d\n", max_records);
	printf("\n");
	printf("               SSID              | Channel | RSSI |   Authentication Mode \n");
	printf("***************************************************************\n");
	for (int i = 0; i < max_records; i++) {
	    printf("%32s | %7d | %4d | %12s\n", (char *)wifi_records[i].ssid, wifi_records[i].primary, wifi_records[i].rssi, auth_mode_type(wifi_records[i].authmode));
	}
	printf("***************************************************************\n");

	vTaskDelete(WifiSCANTaskt);
}*/


