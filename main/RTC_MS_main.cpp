/* C++ exception handling example

 This example code is in the Public Domain (or CC0 licensed, at your option.)

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <driver/gpio.h>
#include <esp_attr.h>
#include <esp_err.h>
#include <esp_interface.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_now.h>
#include <esp32/rom/ets_sys.h>
#include <esp32/rom/gpio.h>
#include "esp_spiffs.h"
#include "driver/uart.h"
#include <freertos/portmacro.h>
#include <freertos/task.h>
#include "freertos/semphr.h"
#include <hal/gpio_types.h>
#include <nvs_flash.h>
#include <sys/_stdint.h>
#include <esp_netif.h>

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_netif.h>
#include <esp_eth.h>
#include <protocol_examples_common.h>
#include <esp_http_server.h>

#include <cmath>
#include <cstring>
#include <string.h>
#include <functional>
#include <map>
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
#include "IsChangedSingletone.h"
#include "DataStruct.h"
#include <freertos/event_groups.h>

using std::cout;
using std::endl;
using std::runtime_error;

#define MAXIMUM_AP 20
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

static const char *TAG = "RT_MS";
static const int RX_BUF_SIZE = 256;
static QueueHandle_t queueCommand;
static QueueHandle_t qWSCommand;
static ConfigurationHandler configHandler(CONFIG_PATH);

SemaphoreHandle_t i2c_mutex;
//RTCDriver ooo;
void RXtask(void *parameters);
void initUART(void);

static EventGroupHandle_t my_event_group;
const int WS_BIT = BIT1;
const int WIFI_BIT = BIT0;

sDataStruct Data;
static esp_err_t ws_handler(httpd_req_t *req);
void WS_handlerTask(void *parameters);
static void connect_handler(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data);
httpd_uri_t s_ws = { .uri = "/ws", .method = HTTP_GET, .handler = ws_handler, .is_websocket = true };

static std::string auth_mode_type(wifi_auth_mode_t auth_mode) {
	std::string types[] = { "OPEN", "WEP", "WPA PSK", "WPA2 PSK",
			"WPA WPA2 PSK", "MAX" };
	return types[auth_mode];
}

static void scan_done_handler(void) {
	uint16_t sta_number = 0;
	uint8_t i;
	wifi_ap_record_t *ap_list_buffer;

	esp_wifi_scan_get_ap_num(&sta_number);
	ap_list_buffer = (wifi_ap_record_t*) (malloc(
			sta_number * sizeof(wifi_ap_record_t)));
	if (ap_list_buffer == NULL) {
		ESP_LOGE(TAG, "Failed to malloc buffer to print scan results");
		return;
	}
	std::ios_base::fmtflags f(cout.flags());
	if (esp_wifi_scan_get_ap_records(&sta_number,
			(wifi_ap_record_t*) ap_list_buffer) == ESP_OK) {
		for (i = 0; i < sta_number; i++) {
			// ESP_LOGI(TAG, "[%s][rssi=%d][MAC=%X:%X:%X:%X:%X:%X][%12s]", ap_list_buffer[i].ssid, ap_list_buffer[i].rssi, ap_list_buffer[i].bssid[0],ap_list_buffer[i].bssid[1],ap_list_buffer[i].bssid[2],ap_list_buffer[i].bssid[3],ap_list_buffer[i].bssid[4],ap_list_buffer[i].bssid[5], auth_mode_type(ap_list_buffer[i].authmode));
			cout << TAG << ":" << ap_list_buffer[i].ssid << "[rssi="
					<< static_cast<int>(ap_list_buffer[i].rssi) << "][MAC="
					<< std::hex << static_cast<int>(ap_list_buffer[i].bssid[0])
					<< ":" << static_cast<int>(ap_list_buffer[i].bssid[1])
					<< ":" << static_cast<int>(ap_list_buffer[i].bssid[2])
					<< ":" << static_cast<int>(ap_list_buffer[i].bssid[3])
					<< ":" << static_cast<int>(ap_list_buffer[i].bssid[4])
					<< ":" << static_cast<int>(ap_list_buffer[i].bssid[5])
					<< ":" << "][" << auth_mode_type(ap_list_buffer[i].authmode)
					<< std::dec << "]" << endl;
		}
	}
	cout.flags(f);  // restore flags state
	free(ap_list_buffer);
}


static httpd_handle_t server = NULL;

static esp_err_t index_handler(httpd_req_t *req) {

	// Open the file from SPIFFS
	FILE *f = fopen("/spiffs/index.html", "r");
	if (f == NULL) {
		ESP_LOGE(TAG, "Failed to open file for reading");
		httpd_resp_send_404(req);
		return ESP_FAIL;
	}

	// Read the file content and send it as a response
	char line[256];
	while (fgets(line, sizeof(line), f) != NULL) {
		httpd_resp_sendstr_chunk(req, line);
	}
	// Close the file
	fclose(f);
	// Signal the end of response
	httpd_resp_sendstr_chunk(req, NULL);
	return ESP_OK;
}

static httpd_handle_t start_webserver() {

	cout << "Starting web server" << endl;
	httpd_handle_t lserver = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	if (httpd_start(&lserver, &config) == ESP_OK) {
		httpd_uri_t index_uri = { 
			.uri = "/", 
			.method = HTTP_GET, .handler = index_handler, 
			.user_ctx = NULL,
			.is_websocket = false  
		};
		httpd_register_uri_handler(lserver, &index_uri);

		// Register the WebSocket handler
		httpd_register_uri_handler(lserver, &s_ws);
		return lserver;
	}

	ESP_LOGI(TAG, "Error starting server!");
	return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server) {
	// Stop the httpd server
	return httpd_stop(server);
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
		int32_t event_id, void *event_data) {
	httpd_handle_t *lserver = (httpd_handle_t*) arg;
	if (*lserver) {
		ESP_LOGI(TAG, "Stopping webserver");
		if (stop_webserver(*lserver) == ESP_OK) {
			*lserver = NULL;
			cout << TAG << " Stopping webserver done ,,,(ovO),,,"<<endl;
			xEventGroupClearBits(my_event_group, WS_BIT);
		} else {
			ESP_LOGE(TAG, "Failed to stop http server");
		}
	}else{
		cout << TAG << " Disconnect problem, server was NULL ,,,(ovO)B"<<endl;
	}
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
		int32_t event_id, void *event_data) {
	switch (event_id) {
	case WIFI_EVENT_SCAN_DONE:
		cout << " scan ..." << endl;
		scan_done_handler();
		ESP_LOGI(TAG, "sta scan done");
		break;
	case WIFI_EVENT_AP_STACONNECTED: {
		//wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t*) event_data;
		//ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event_base->mac), event_base->aid);
		xEventGroupSetBits(my_event_group, WIFI_BIT);
		break;
	}
	case WIFI_EVENT_AP_STADISCONNECTED: {
		//wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t*) event_data;
		//ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event_base->mac), event_base->aid);
		httpd_handle_t * lserver = (httpd_handle_t*) arg;
		if (*lserver) {
			cout << TAG <<  " Stopping webserver" << endl; 
			if (stop_webserver(*lserver) == ESP_OK) {
				cout << "Done : Web Server is NULL"<< endl;
				*lserver = NULL;
			} else {
				ESP_LOGE(TAG, "Failed to stop http server");
			}
		}else{
			cout << "ERROR : Disconnect with NULL server "<<endl;
			
		}

		xEventGroupClearBits(my_event_group, WIFI_BIT | WS_BIT);
		break;
	}
	default:
		break;
	}
	return;

}


static void initialize_wifi() {
	
	
	ESP_ERROR_CHECK(esp_netif_init());
	my_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	
	esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
	assert(ap_netif);
	esp_netif_t *sta_netif =esp_netif_create_default_wifi_sta();
	assert(sta_netif);

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	//ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler, NULL) );
	//ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );

	
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
	ESP_ERROR_CHECK( esp_wifi_start() );
		
}


/*
 * Wifi Init Function
 * Support to testing Scan functionality
 * old tcpip removed
 */
static void wifiInit() {

	//esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	//assert(sta_netif);
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
	wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());
}

static void wifiInitAP() {
	
	
	//esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
	//assert(ap_netif);
	//esp_netif_t *sta_netif =esp_netif_create_default_wifi_sta();
	//assert(sta_netif);

	//wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, 		&connect_handler, 		&server));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, 	&disconnect_handler, 	&server));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE,			&wifi_event_handler, 	NULL));
	/*ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
	 ESP_EVENT_ANY_ID,
	 &wifi_event_handler,
	 NULL,
	 NULL));*/

	wifi_config_t ap_config = { 
		.ap = { 
			.ssid = EXAMPLE_ESP_WIFI_SSID,
			.password = EXAMPLE_ESP_WIFI_PASS,
			.ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID), 
			.channel = EXAMPLE_ESP_WIFI_CHANNEL,
			.authmode = WIFI_AUTH_WPA2_PSK, 
			.max_connection = EXAMPLE_MAX_STA_CONN,
			.pmf_cfg = { .required = true, }, 
		}, 
	};
	
	
	if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
		ap_config.ap.authmode = WIFI_AUTH_OPEN;
	}

	wifi_config_t sta_config = { 0 };
	// AP Wifi scan return ESP_FAIL --> this is a feature
	// ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
	ESP_ERROR_CHECK(esp_wifi_start());

}

static void connect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
	cout << "Connect to RTC_MS" << endl;
	httpd_handle_t *lserver = (httpd_handle_t*) arg;
	if (*lserver == NULL) {
		ESP_LOGI(TAG, "Starting webserver");
		*lserver = start_webserver();
	}
}

void gpioSetup(int gpioNum, int gpioMode, int gpioVal) {

	gpio_num_t gpioNumNative = (gpio_num_t) (gpioNum);
	gpio_mode_t gpioModeNative = (gpio_mode_t) (gpioMode);
	gpio_pad_select_gpio(gpioNumNative);
	gpio_set_direction(gpioNumNative, gpioModeNative);

	if (gpioMode == OUTPUT) {
		gpio_set_level(gpioNumNative, gpioVal);
	}
}

void checkHWInputs() {
	Data.pins[0] = gpio_get_level((gpio_num_t) WAKE_UP_GPIO);
	Data.pins[1] = gpio_get_level((gpio_num_t) REED_SWITCH);
	cout << "pins: " << Data.pins << endl;
}

void setGPIOasInput(int gpioNum) {
	gpio_config_t gpioModeNative;
	gpioModeNative.intr_type = GPIO_INTR_DISABLE;
	gpioModeNative.pin_bit_mask = (1ULL << gpioNum);
	gpioModeNative.mode = GPIO_MODE_INPUT;
	// TODO : With the pull down the WAKE UP button okay, !INT shall be checked parallel
	gpioModeNative.pull_up_en = GPIO_PULLUP_DISABLE;
	gpioModeNative.pull_down_en = GPIO_PULLDOWN_ENABLE;
	gpio_config(&gpioModeNative);
}

void setHWInputs() {
	// hold the Vcc for the MCU
	gpioSetup(MCU_ON, OUTPUT, HIGH);
	// Contact reading
	gpioSetup(REED_SWITCH, INPUT, LOW);
	// Wake up button
	setGPIOasInput(WAKE_UP_GPIO);
	// On board LED
	gpioSetup(ON_BOARD_LED, OUTPUT, LOW);
}

void init_spiffs() {

	esp_vfs_spiffs_conf_t config = { .base_path = "/spiffs", .partition_label =
	NULL, .max_files = 5, .format_if_mount_failed = true, };
	esp_err_t ret = esp_vfs_spiffs_register(&config);

	switch (ret) {
	case ESP_OK:
		cout << " SPIFF Init done" << std::endl;
		break;
	case ESP_ERR_NO_MEM:
		cout << " if objects could not be allocated" << std::endl;
		return;
	case ESP_ERR_INVALID_STATE:
		cout << "already mounted or partition is encrypted" << std::endl;
		return;
	case ESP_ERR_NOT_FOUND:
		cout << "partition for SPIFFS was not found" << std::endl;
		return;
	case ESP_FAIL:
		cout << "mount or format fails" << std::endl;
		return;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)",
				esp_err_to_name(ret));
	} else {
		ESP_LOGI(TAG, "SPIFFS partition size: total: %d, used: %d", total,
				used);
	}

	return;
}

static esp_err_t ws_handler(httpd_req_t *req) {
	if (req->method == HTTP_GET) {
		ESP_LOGI(TAG, "Handshake done, the new connection was opened");
		xEventGroupSetBits(my_event_group, WS_BIT);
		//TODO trigger initial data sent to the index.html
		return ESP_OK;
	}
	httpd_ws_frame_t ws_pkt;
	uint8_t *buf = NULL;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));

	// First receive the full ws message
	/* Set max_len = 0 to get the frame len */
	esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d",
				ret);
		return ret;
	}
	ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
	if (ws_pkt.len) {
		/* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
		buf = (uint8_t*) calloc(1, ws_pkt.len + 1);
		if (buf == NULL) {
			ESP_LOGE(TAG, "Failed to calloc memory for buf");
			return ESP_ERR_NO_MEM;
		}
		ws_pkt.payload = buf;
		/* Set max_len = ws_pkt.len to get the frame payload */
		ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
			free(buf);
			return ret;
		}
	}
	// If it was a PONG, update the keep-alive
	if (ws_pkt.type == HTTPD_WS_TYPE_PONG) {
		ESP_LOGD(TAG, "Received PONG message");
		free(buf);
		//return wss_keep_alive_client_is_active(httpd_get_global_user_ctx(req->handle),httpd_req_to_sockfd(req));
		return ESP_OK;
		// If it was a TEXT message, just echo it back
	} else if (ws_pkt.type == HTTPD_WS_TYPE_TEXT
			|| ws_pkt.type == HTTPD_WS_TYPE_PING
			|| ws_pkt.type == HTTPD_WS_TYPE_CLOSE) {
		if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) {
			ESP_LOGI(TAG, "Received packet with message: %s", ws_pkt.payload);
			//TODO check if the message is handed over properly.
			std::string *pStr = new std::string(ws_pkt.payload, ws_pkt.payload+ws_pkt.len);
			// needs to copy to another structure....
			
			
			
			// full message hand over to the handler task, which is responsible for the handling.
			xQueueSend(qWSCommand, &pStr, portTICK_PERIOD_MS);
			
			
		} else if (ws_pkt.type == HTTPD_WS_TYPE_PING) {
			// Response PONG packet to peer
			ESP_LOGI(TAG, "Got a WS PING frame, Replying PONG");
			ws_pkt.type = HTTPD_WS_TYPE_PONG;
		} else if (ws_pkt.type == HTTPD_WS_TYPE_CLOSE) {
			// Response CLOSE packet with no payload to peer
			ws_pkt.len = 0;
			ws_pkt.payload = NULL;
			xEventGroupClearBits(my_event_group, WS_BIT);

		}
		ret = httpd_ws_send_frame(req, &ws_pkt);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
		}
		ESP_LOGI(TAG,
				"ws_handler: httpd_handle_t=%p, sockfd=%d, client_info:%d",
				req->handle, httpd_req_to_sockfd(req),
				httpd_ws_get_fd_info(req->handle, httpd_req_to_sockfd(req)));
		free(buf);
		return ret;
	}
	free(buf);
	return ESP_OK;
}

esp_err_t httpd_ws_send_frame_to_all_clients(httpd_ws_frame_t *ws_pkt) {
	static constexpr size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
	size_t fds = max_clients;
	int client_fds[max_clients] = { 0 };

	esp_err_t ret = httpd_get_client_list(server, &fds, client_fds);

	if (ret != ESP_OK) {
		return ret;
	}

	for (int i = 0; i < fds; i++) {
		auto client_info = httpd_ws_get_fd_info(server, client_fds[i]);
		if (client_info == HTTPD_WS_CLIENT_WEBSOCKET) {
			httpd_ws_send_frame_async(server, client_fds[i], ws_pkt);
		}
	}

	return ESP_OK;
}

esp_err_t wsSenderFnc(std::string msg) {

	httpd_ws_frame_t ws_pkt;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;
	ws_pkt.final = true;
	ws_pkt.payload = (uint8_t*) (msg.c_str());
	ws_pkt.len = msg.length();
	return httpd_ws_send_frame_to_all_clients(&ws_pkt);
}

/* Inside .cpp file, app_main function must be declared with C linkage */
extern "C" void app_main(void) {
    ESP_ERROR_CHECK( nvs_flash_init());
   
	setHWInputs();
	checkHWInputs();
	initialize_wifi();
	init_spiffs();

	setenv("HU", "Europe/Budapest", 1);
	tzset();
	//ConfigurationHandler configHandler(CONFIG_PATH);
	

	BatteryMGM batt("BatteryManager");
	SavingInterfaceClass *battI = &batt;
	configHandler.registerClass(battI);

	cout << "Battery Read " << batt.readADC() << " RAW value" << endl;

	Data.batteryVoltage = batt.getBatteryVoltage();
	cout << "Battery Read " << Data.batteryVoltage << " [mV] " << endl;

	esp_err_t ret;
	uint16_t year = 0;
	uint8_t month = 0, date = 0, hour = 0, minute = 0, second = 0;
	int qL = 0;
	bool bRTCWakeUpByTimer;

	_scommand x;
	initUART();
	i2c_mutex = xSemaphoreCreateBinary();
	if (i2c_mutex == NULL) {
		ESP_LOGI(TAG, "I2C Mutex creation  unsuccesfull");
	}
	if ( xSemaphoreGive( i2c_mutex ) != pdTRUE) {
		// We would expect this call to fail because we cannot give
		// a semaphore without first "taking" it!
	}
	ret = i2c_master_init();
	if (ret != ESP_OK)
		cout << "i2c driver install failed" << endl;
	RTCDriver *ooo = new RTCDriver("RTC", &i2c_mutex, &i2c_master_read_slave,
			&i2c_master_write_slave);
	SavingInterfaceClass *rtcI = ooo;
	configHandler.registerClass(rtcI);

// config file read and process 
	configHandler.LoadAllConfiguration();

	queueCommand = ooo->getCommandQueue();

	ret = ooo->readAllRegsFromRTC();
	if (ret != ESP_OK) {
		cout << "i2c Read Failed" << endl;
	}
	// Forced DayLight Saving calculation 
	ooo->CheckDLS();
	// Store the UTC when the event happened
	Data.epoch = ooo->getEpochUTC();

	// how to init Wifi --> depends on the reason of wake up

	//config button cause the wake up
	if (Data.pins[0] == 1) {
		unsigned int _topicSize = CONFIG_TOPIC_SIZE;
		qWSCommand =  xQueueCreate(_topicSize, sizeof(std::string *) );
		wifiInitAP();
		// wifi init after the esp init required to start
		InitEspNowChannel();
		//xTaskCreate(WS_handlerTask, "WSReqhandler", 1024 * 6, (void *)(&configHandler), tskIDLE_PRIORITY, NULL);
		xTaskCreate(WS_handlerTask, "WSReqhandler", 1024 * 6, NULL, tskIDLE_PRIORITY, NULL);
	} else {
		wifiInit();
		ESP_ERROR_CHECK(InitEspNowChannel());
	}
	
	

	// TD 1/60Hz, TE Enabled, TIE Enabled,  TI_TP Enabled
	//ooo->writeTimerModeToRTC(0b11111); // 0b11111


// Wake up reason check
	ooo->isTimerWakeUp(true, &bRTCWakeUpByTimer);
	Data.pins[2] = bRTCWakeUpByTimer;
	cout << "Wake Up by Timer : " << bRTCWakeUpByTimer << endl;

	cout << "app_main starting" << endl;

// only debug purpose
	xTaskCreate(RXtask, "uart_rx_task", 1024 * 2, NULL, tskIDLE_PRIORITY,
	NULL);



// only debug purpose
	while (true) {

		qL = uxQueueMessagesWaiting(ooo->getCommandQueue());
		for (int i = 0; i < qL; i++) {
			xQueueReceive(ooo->getCommandQueue(), &x, portMAX_DELAY);
			if (x.command[0] == 'S' && x.command[1] == 'E') {
				ESP_LOGI("SET UTC EPOCH", "Read %d bytes: '%s' \n",
						sizeof(x.command)/sizeof(char), x.command);
				ooo->writeTimeFromEpochToRTC((atol(x.command + 2)));
			} else if (x.command[0] == 'G' && x.command[1] == 'E') {
				ooo->readAllRegsFromRTC();
				printf("GET UTC EPOCH : %ld \n", ooo->getEpochUTC());

				if ( xEventGroupGetBits(my_event_group) & WS_BIT) {
					// Web socket avail
					wsSenderFnc(to_string(ooo->getEpochUTC()));
				}
			} else if (x.command[0] == 'S' && x.command[1] == 'L') {
				ESP_LOGI("SET Local EPOCH", "Read %d bytes: '%s' \n",
						sizeof(x.command)/sizeof(char), x.command);
				ooo->writeTimeFromEpochToRTC((atol(x.command + 2)), false);
			} else if (x.command[0] == 'G' && x.command[1] == 'L') {
				printf("GET LOCAL EPOCH : %ld \n", ooo->getEpoch());
			}

			else if (x.command[0] == 'S' && x.command[1] == 'T') {
				ESP_LOGI("SET TIME ZONE : ", "Read %d bytes: '%s' \n",
						sizeof(x.command)/sizeof(char), x.command);
				ooo->setTimeZone(atoi(x.command + 2), true);
			} else if (x.command[0] == 'G' && x.command[1] == 'T') {
				printf("GET Time Zone : %d \n", ooo->getTimeZone());
			} else if (x.command[0] == 'W' && x.command[1] == 'S') {
				//ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, false));
				ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
			} else if (x.command[0] == 'G' && x.command[1] == 'N') {
				//ESP_ERROR_CHECK(ooo->CheckDLS());
				ESP_ERROR_CHECK(ooo->readHoursFromRTC(&hour));
				ESP_ERROR_CHECK(ooo->readMinutesFromRTC(&minute));
				ESP_ERROR_CHECK(ooo->readSecondsFromRTC(&second));
				printf("Time: %.2d:%.2d:%.2d \n", hour, minute, second);
				ESP_ERROR_CHECK(ooo->readYearFromRTC(&year));
				ESP_ERROR_CHECK(ooo->readMonthFromRTC(&month));
				ESP_ERROR_CHECK(ooo->readDateFromRTC(&date));
				printf("Date: %d-%.2d-%.2d \n", year, month, date);
				ESP_ERROR_CHECK(ooo->isTimerWakeUp(true, &bRTCWakeUpByTimer));
				cout << "Wake Up by Timer : " << bRTCWakeUpByTimer << endl;
			} else if (x.command[0] == 'G' && x.command[1] == 'C') {
				try {
					uint8_t timerValue = 0;
					uint8_t timerMode = 0;
					ESP_ERROR_CHECK(ooo->readTimerValueFromRTC(&timerValue));
					ESP_ERROR_CHECK(ooo->readTimerModeFromRTC(&timerMode));
					//Conversion from number to enum value...
					eTimeClockFreq val = static_cast<eTimeClockFreq>(TD_CHECK(
							timerMode));
					cout << unsigned(timerValue) << " " << val
						 << " Timer Enable " << TE_CHECK(timerMode)
						 << " Timer Interrupt Enable "
						 << TIE_CHECK(timerMode) << " Timer Interrupt Mode "
						 << TI_TP_CHECK(timerMode) << endl;
				} catch (const std::range_error &e) {
					cout << "Exception caught: " << e.what() << endl;
				}
			} else if (x.command[0] == 'S' && x.command[1] == 'C'
					&& x.command[2] == 'V') {

				if ((strlen(x.command) - 3) == 0) {
					cout << "Timer Value Out of Range : No Value added" << endl;
					break;
				}
				int value = 0;
				value = atoi(x.command + 3);
				if (value < 256) {
					ESP_ERROR_CHECK(ooo->writeTimerValueToRTC((uint8_t )value));
					cout << " Timer Value set to : " << value << endl;
				} else {
					cout << "Timer value Out of Range [0-255] : " << value
							<< endl;
				}
			} else if (x.command[0] == 'S' && x.command[1] == 'C'
					&& x.command[2] == 'M') {
				if ((strlen(x.command) - 4) > 2) {
					cout << "Timer Mode Out of Range : " << x.command + 3
							<< endl;
					break;
				} else if ((strlen(x.command) - 4) == 0) {
					cout << "Timer Mode Out of Range : No Value added" << endl;
					break;
				}
				uint8_t value = (int) strtol(x.command + 3, NULL, 16);
				// uint8_t reg;
				if (value > 0b11111) {
					cout << "Timer Mode Out of Range [0-1F] : " << x.command + 3
							<< endl;
					break;
				}
				ESP_ERROR_CHECK(ooo->writeTimerModeToRTC(value));
				eTimeClockFreq val =
						static_cast<eTimeClockFreq>(TD_CHECK(value));
				cout << "Timer Freq set : " << val << " Timer Enable "
						<< TE_CHECK(value) << " Timer Interrupt Enable "
						<< TIE_CHECK(value) << " Timer Interrupt Mode "
						<< TI_TP_CHECK(value) << endl;
				// trial;
				//ooo->writeControl2Reg(0b111);

			} else if (x.command[0] == 'G' && x.command[1] == 'A') {
				ESP_ERROR_CHECK(ooo->printAllRegs(true));
			} else if (x.command[0] == 'C' && x.command[1] == 'C') {
				ESP_ERROR_CHECK(ooo->writeControl2Reg(0b1111));
			} else if (x.command[0] == 'C' && x.command[1] == 'V') {
				ESP_ERROR_CHECK(ooo->writeControl2Reg(0b111));
			} else if (x.command[0] == 'R' && x.command[1] == 'R') {
				ooo->resetRTC();
			} else if (x.command[0] == 'T' && x.command[1] == 'T') {
//			 1.	Set the timer configuration → Reg(0x10) = 0x0A, Reg(0x11) = 0x12, Reg(0x01) = 0x00
//			 2.	Launch the timer → Reg(0x11) = 0x16  (TE = 1)
				cout << "Special test routine start : 10 sec timer TP_TI = 0"
						<< endl;
				ooo->writeTimerValueToRTC(0x0A);
				ooo->writeTimerModeToRTC(0x12);
				ooo->writeControl1Reg(0x00);
				ooo->writeTimerModeToRTC(0x16);
				cout << "Start checking the oscilloscope pls..." << endl;
			} else if (x.command[0] == 'Q' && x.command[1] == 'T') {
				configHandler.SaveAllConfiguration();
				esp_now_deinit();
				esp_wifi_stop();
				i2c_master_deinit();
				esp_vfs_spiffs_unregister(NULL);
				vTaskDelay(10 / portTICK_PERIOD_MS);
				gpio_set_level((gpio_num_t) 17, LOW);
				esp_restart();

			} else if (x.command[0] == 'L' && x.command[1] == 'S'
					&& x.command[2] == 'I') {
				gpio_set_level((gpio_num_t) ON_BOARD_LED, HIGH);

			} else if (x.command[0] == 'L' && x.command[1] == 'S'
					&& x.command[2] == 'O') {
				gpio_set_level((gpio_num_t) ON_BOARD_LED, LOW);

			} else if (x.command[0] == 'G' && x.command[1] == 'J') {
				configHandler.ForcedSave();
			} else if (x.command[0] == 'F' && x.command[1] == 'M') {
				cout << esp_get_free_heap_size() << " bytes" << endl;
			} else if (x.command[0] == 'G' && x.command[1] == 'V') {
				cout << "Battery Read " << batt.readADC() << endl;
				cout << "Battery Read " << batt.getBatteryVoltage() << " [mV] "
						<< endl;
			} else if (x.command[0] == 'C' && x.command[1] == 'I') {
				checkHWInputs();
			} else if (x.command[0] == 'L' && x.command[1] == 'S') {
				ooo->CheckDLS();
			}

		}

		vTaskDelay(10 / portTICK_PERIOD_MS);
	}

	cout << "app_main done" << endl;
	esp_vfs_spiffs_unregister(NULL);
}

void RXtask(void *parameters) {
	static const char *RX_TASK_TAG = "RX_TASK";
	esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
	_scommand rx;
	uint8_t *data = (uint8_t*) malloc(RX_BUF_SIZE + 1);
	while (1) {
		const int rxBytes = uart_read_bytes(UART_NUM_0, data, RX_BUF_SIZE,
				1000 / portTICK_PERIOD_MS);
		if (rxBytes > 0) {
			data[rxBytes] = 0;
			strcpy(rx.command, (char*) data);
			xQueueSend(queueCommand, &rx, portTICK_PERIOD_MS);
		}
	}
	free(data);
	vTaskDelete(NULL);
}

void initUART(void) {

	const uart_config_t uart_config = { .baud_rate = 115200, .data_bits =
			UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE, .stop_bits =
			UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
			.rx_flow_ctrl_thresh = 122 };
	if (uart_driver_install(UART_NUM_0, 2 * 1024, 0, 0, NULL, 0) != ESP_OK) {
		ESP_LOGE(TAG, "Driver installation failed");
		//vTaskDelete(NULL);
	}
	uart_param_config(UART_NUM_0, &uart_config);
}

void WS_handlerTask(void *parameters) {
	// task setup
	int qLen = 0;
	//ConfigurationHandler * config = (ConfigurationHandler * ) parameters;
	ConfigurationHandler * config = &configHandler;
	
	std::string * ws_msg = NULL;
	// task loop
	while (true) {
		qLen = uxQueueMessagesWaiting(qWSCommand);
		//If queue has more message
		for (int i = 0; i < qLen; i++) {
			xQueueReceive(qWSCommand, &ws_msg, portMAX_DELAY);
			// TODO here add logic	
			cout << "WS fun message : [" << *ws_msg << "]" << endl;						
			cJSON *root = cJSON_Parse((*ws_msg).c_str());
			if(root != NULL) {
				cJSON * command = cJSON_GetObjectItem(root, "epoch");
				if(command != NULL) {
					long iepoch = cJSON_GetObjectItem(root,"epoch")->valueint;
					cout << "Epoch :["<< iepoch <<"]"<<endl;
					SavingInterfaceClass * RTC = config->getClassPointer("RTC");
					((RTCDriver *) RTC)->writeTimeFromEpochToRTC(iepoch);
					((RTCDriver *) RTC)->ForcedDLSUpdate();
				}
				cJSON_Delete(root);
			}else{
				cout << "Parse error : [" << *ws_msg << "]" << endl;			
			}
			free(ws_msg);
			ws_msg = NULL;		
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
		 
}

