/*
 * esp_now_stuff.h
 *
 *  Created on: 2023. jún. 27.
 *      Author: Dell
 */

#ifndef MAIN_ESP_NOW_STUFF_H_
#define MAIN_ESP_NOW_STUFF_H_

#include <iostream>
#include <esp_interface.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_now.h>
#include <cstdio>
#include <sys/_stdint.h>
#include <stdio.h>

typedef struct {
	uint8_t mac_addr[ESP_NOW_ETH_ALEN];
	uint8_t *data;
	int data_len;
} esp_now_event_recv_cb_t;

void esp_now_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status);
void esp_now_recv_cb(const esp_now_recv_info_t * esp_now_info, const uint8_t *data, int data_len);
esp_err_t InitEspNowChannel(void);

#endif /* MAIN_ESP_NOW_STUFF_H_ */
