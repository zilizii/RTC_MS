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
#include <esp_now.h>
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
#include <bitset>
#include <iostream>
#include "sdkconfig.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <hal/gpio_types.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "HW_setup.h"
#include "ConfigurationHandler.h"
#include "BatteryMGM.h"
#include "RTCDriver.h"
#include "esp_now_stuff.h"



//#include "esp_event_loop.h"
#include "esp_event.h"


using std::cout;
using std::endl;
using std::runtime_error;

#define MAXIMUM_AP 20



static const char *TAG = "RT_MS";
static const int RX_BUF_SIZE = 256;
QueueHandle_t queueCommand;

SemaphoreHandle_t i2c_mutex;
//RTCDriver ooo;
void RXtask(void * parameters);
void initUART(void);




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
           // ESP_LOGI(TAG, "[%s][rssi=%d][MAC=%X:%X:%X:%X:%X:%X][%12s]", ap_list_buffer[i].ssid, ap_list_buffer[i].rssi, ap_list_buffer[i].bssid[0],ap_list_buffer[i].bssid[1],ap_list_buffer[i].bssid[2],ap_list_buffer[i].bssid[3],ap_list_buffer[i].bssid[4],ap_list_buffer[i].bssid[5], auth_mode_type(ap_list_buffer[i].authmode));
        	cout << TAG << ap_list_buffer[i].ssid <<"[rssi="<< ap_list_buffer[i].rssi<<"][MAC="<< std::hex << ap_list_buffer[i].bssid[0]<<":"<<std::hex<<ap_list_buffer[i].bssid[1]<<":"<<std::hex<<ap_list_buffer[i].bssid[2]<<":"<<std::hex<<ap_list_buffer[i].bssid[3]<<":"<<std::hex<<ap_list_buffer[i].bssid[4]<<":"<<std::hex<<ap_list_buffer[i].bssid[5]<<":"<<"]["<<auth_mode_type(ap_list_buffer[i].authmode)<<endl;
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



void gpioSetup(int gpioNum, int gpioMode, int gpioVal) {

    gpio_num_t gpioNumNative = (gpio_num_t)(gpioNum);
    gpio_mode_t gpioModeNative = (gpio_mode_t)(gpioMode);
    gpio_pad_select_gpio(gpioNumNative);
    gpio_set_direction(gpioNumNative, gpioModeNative);

    if(gpioMode == OUTPUT){
    	gpio_set_level(gpioNumNative, gpioVal);
    }
}


void checkHWInputs() {
	//uint8_t pins= 0x0;
	std::bitset<2> pins;
	pins[0] = gpio_get_level((gpio_num_t)WAKE_UP_GPIO);
	pins[1] =  gpio_get_level((gpio_num_t)REED_SWITCH);
	cout <<"pins: " << pins << endl;
}

void setGPIOasInput(int gpioNum) {
	gpio_config_t gpioModeNative;
	gpioModeNative.intr_type = GPIO_INTR_DISABLE;
	gpioModeNative.pin_bit_mask = (1ULL<<gpioNum);
	gpioModeNative.mode = GPIO_MODE_INPUT;
	// TODO : With the pull down the WAKE UP button okay, !INT shall be checked parallel
	// TODO : better config shall eb implemented
	gpioModeNative.pull_up_en = GPIO_PULLUP_DISABLE;
	gpioModeNative.pull_down_en = GPIO_PULLDOWN_ENABLE;
	gpio_config(&gpioModeNative);
}

void setHWInputs() {
	gpioSetup(MCU_ON, OUTPUT, HIGH);
	gpioSetup(REED_SWITCH, INPUT, LOW);
	//HW sees as always high
	//gpioSetup(WAKE_UP_GPIO, INPUT, LOW);
	setGPIOasInput(WAKE_UP_GPIO);
	gpioSetup(ON_BOARD_LED, OUTPUT, LOW);

}


/* Inside .cpp file, app_main function must be declared with C linkage */
extern "C" void app_main(void)
{
	//gpioSetup(MCU_ON, OUTPUT, HIGH);
	setHWInputs();
	checkHWInputs();
	BatteryMGM batt("BatteryManager");
	cout << "Battery Read "<< batt.readADC() << endl;
	cout << "Battery Read "<< batt.getBatteryVoltage() << " [mV] " << endl;
	esp_err_t ret;
	uint16_t year = 0;
	uint8_t month =0,date =0,hour =0,minute=0,second=0;
	//uint8_t counter = 0;
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
	RTCDriver * ooo = new RTCDriver("RTC",&i2c_mutex, &i2c_master_read_slave, &i2c_master_write_slave);
	queueCommand = ooo->getCommandQueue();
	ret = ooo->readAllRegsFromRTC();
	if(ret != ESP_OK){
			 cout<< "i2c Read Failed" << endl;
	}
	cout << "app_main starting" << endl;

	//removed for testing purpose
	//ooo->writeTimerValueToRTC(5);
	// TD 1/60Hz, TE Enabled, TIE Enabled,  TI_TP Enabled
	//ooo->writeTimerModeToRTC(0b11111); // 0b11111

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
					 cout << unsigned(timerValue) << " " << val << " Timer Enable "<< TE_CHECK(timerMode)
							 << " Timer Interrupt Enable " << TIE_CHECK(timerMode) << " Timer Interrupt Mode " << TI_TP_CHECK(timerMode) << endl;
				 } catch (const std::range_error &e) {
					 cout << "Exception caught: " << e.what() << endl;
				 }
			 }
			 else if (x.command[0] == 'S' && x.command[1] == 'C' && x.command[2] == 'V') {

				 if ( (strlen(x.command) - 3) == 0) {
					cout<<"Timer Value Out of Range : No Value added" << endl;
				 	break;
				 }
				 int value = 0;
				 value = atoi(x.command + 3);
				 if(value < 256) {
					 ESP_ERROR_CHECK(ooo->writeTimerValueToRTC( (uint8_t)value));
					 cout <<" Timer Value set to : " << value << endl;
				 }else{
					 cout<<"Timer value Out of Range [0-255] : " << value <<endl;
				 }
			 }
			 else if (x.command[0] == 'S' && x.command[1] == 'C' && x.command[2] == 'M') {
				 if(    (strlen(x.command) - 4) > 2 ){
					 cout<<"Timer Mode Out of Range : " << x.command + 3 <<endl;
					 break;
				 }
				 else if ( (strlen(x.command) - 4) == 0) {
					 cout<<"Timer Mode Out of Range : No Value added" << endl;
					 break;
				 }
				 uint8_t value = (int)strtol(x.command + 3, NULL, 16);
				// uint8_t reg;
				 if (value > 0b11111) {
					 cout<<"Timer Mode Out of Range [0-1F] : " << x.command + 3 <<endl;
					 break;
				 }
				 ESP_ERROR_CHECK(ooo->writeTimerModeToRTC(value));
				 eTimeClockFreq val = static_cast<eTimeClockFreq>(TD_CHECK(value));
				 cout << "Timer Freq set : " << val << " Timer Enable "<< TE_CHECK(value)
				 	  << " Timer Interrupt Enable " <<TIE_CHECK(value) << " Timer Interrupt Mode " << TI_TP_CHECK(value) << endl;
				 // trial;
				 //ooo->writeControl2Reg(0b111);

			 }
			 else if (x.command[0] == 'G' && x.command[1] == 'A'){
				 ESP_ERROR_CHECK(ooo->printAllRegs(true));
			 }
			 else if (x.command[0] == 'C' && x.command[1] == 'C'){
				 ESP_ERROR_CHECK(ooo->writeControl2Reg(0b1111));
			 }
			 else if (x.command[0] == 'C' && x.command[1] == 'V'){
				 ESP_ERROR_CHECK(ooo->writeControl2Reg(0b111));
			 			 }
			 else if (x.command[0] == 'R' && x.command[1] == 'R'){
				 	 	 	 ooo->resetRTC();
			 }
			 else if (x.command[0] == 'T' && x.command[1] == 'T'){
//			 1.	Set the timer configuration → Reg(0x10) = 0x0A, Reg(0x11) = 0x12, Reg(0x01) = 0x00
//			 2.	Launch the timer → Reg(0x11) = 0x16  (TE = 1)
				 cout<<"Special test routine start : 10 sec timer TP_TI = 0"<< endl;
				 ooo->writeTimerValueToRTC(0x0A);
				 ooo->writeTimerModeToRTC(0x12);
				 ooo->writeControl1Reg(0x00);
				 ooo->writeTimerModeToRTC(0x16);
				 cout<<"Start checking the oscilloscope pls..."<< endl;
			 }
			 else if (x.command[0] == 'Q' && x.command[1] == 'T'){
				 gpio_set_level((gpio_num_t)17, LOW);
				 esp_restart();

			 }
			 else if (x.command[0] == 'L' && x.command[1] == 'S' && x.command[2] == 'I'){
							 gpio_set_level((gpio_num_t)ON_BOARD_LED, HIGH);

			 }
			 else if (x.command[0] == 'L' && x.command[1] == 'S' && x.command[2] == 'O'){
			 							 gpio_set_level((gpio_num_t)ON_BOARD_LED, LOW);

			 }

		 }

		 vTaskDelay( 10 / portTICK_PERIOD_MS );
	 }

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


