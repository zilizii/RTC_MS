/*
 * esp_now_stuff.cpp
 *
 *  Created on: 2023. jún. 27.
 *      Author: Dell
 */
#include "esp_now_stuff.h"

using std::cout;
using std::endl;
using std::runtime_error;

static void esp_now_send_cb(const uint8_t * mac_addr, esp_now_send_status_t status ) {
	// when data sent out
	if(mac_addr == NULL) {
		cout << "Send Call back argument error" << endl;
		return;
	}

	switch(status) {
	case ESP_NOW_SEND_SUCCESS :
		cout << "msg on the way to :" <<mac_addr << endl;
		break;
	case ESP_NOW_SEND_FAIL :
		cout << "Failed to send to : " << mac_addr << " msg "<< endl;
		break;
	default:
		cout << "Send Call back error" << endl;
	}
}

static void esp_now_recv_cb(const uint8_t * mac_addr, const uint8_t * data, int len) {
	// when data arrived ...

	//error handling
	if (mac_addr == NULL || data == NULL || len <= 0) {
			cout << "Receive cb arg error";
			return;
	 }
	//TODO : write the basic data handler here...
	// memcpy the data
	// check
	// send to the right queue


}

static esp_err_t InitEspNowChannel(void) {

	esp_err_t ret;
	// base esp-idf function calls
	/*ESP_ERROR_CHECK(esp_now_init());
	ESP_ERROR_CHECK(esp_now_register_send_cb(esp_now_send_cb) );
	ESP_ERROR_CHECK(esp_now_register_recv_cb(esp_now_recv_cb) );*/


	ret = ESP_OK;
	return ret;
}




