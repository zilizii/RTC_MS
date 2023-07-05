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


static void esp_now_send_cb(const uint8_t * mac_addr, esp_now_send_status_t status);

static void esp_now_recv_cb(const uint8_t * mac_addr, const uint8_t * data, int len);

static esp_err_t InitEspNowChannel(void);


#endif /* MAIN_ESP_NOW_STUFF_H_ */
