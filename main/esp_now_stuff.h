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
#include <esp_mac.h>
#include <cstdio>
#include <sys/_stdint.h>
#include <stdio.h>
#include "SavingInterfaceClass.h"
#include <list>

#define ESP_NOW_QUEUE_SIZE           6

#define IS_BROADCAST_ADDR(addr) (memcmp(addr, s_example_broadcast_mac, ESP_NOW_ETH_ALEN) == 0)


typedef enum {
    ESP_NOW_SEND_CB,
    ESP_NOW_RECV_CB,
} esp_now_event_id_t;

typedef struct {
	uint8_t mac_addr[ESP_NOW_ETH_ALEN];
	esp_now_send_status_t status;
} esp_now_event_send_cb_t;

typedef struct {
	uint8_t mac_addr[ESP_NOW_ETH_ALEN];
	uint8_t *data;
	int data_len;
} esp_now_event_recv_cb_t;

typedef union {
    esp_now_event_send_cb_t send_cb;
    esp_now_event_recv_cb_t recv_cb;
} esp_now_event_info_t;

/* When ESPNOW sending or receiving callback function is called, post event to ESPNOW task. */
typedef struct {
    esp_now_event_id_t id;
    esp_now_event_info_t info;
} esp_now_event_t;

enum {
    ESP_NOW_DATA_BROADCAST,
    ESP_NOW_DATA_UNICAST,
    ESP_NOW_DATA_MAX,
};

typedef struct {
    uint8_t type;                         //Broadcast or unicast ESPNOW data.
    uint8_t state;                        //Indicate that if has received broadcast ESPNOW data or not.
    uint16_t seq_num;                     //Sequence number of ESPNOW data.
    uint16_t crc;                         //CRC16 value of ESPNOW data.
    uint32_t magic;                       //Magic number which is used to determine which device to send unicast ESPNOW data.
    uint8_t payload[0];                   //Real payload of ESPNOW data.
} __attribute__((packed)) esp_now_data_t;

/* Parameters of sending ESPNOW data. */
typedef struct {
    bool unicast;                         //Send unicast ESPNOW data.
    bool broadcast;                       //Send broadcast ESPNOW data.
    uint8_t state;                        //Indicate that if has received broadcast ESPNOW data or not.
    uint32_t magic;                       //Magic number which is used to determine which device to send unicast ESPNOW data.
    uint16_t count;                       //Total count of unicast ESPNOW data to be sent.
    uint16_t delay;                       //Delay between sending two ESPNOW data, unit: ms.
    int len;                              //Length of ESPNOW data to be sent, unit: byte.
    uint8_t *buffer;                      //Buffer pointing to ESPNOW data.
    uint8_t dest_mac[ESP_NOW_ETH_ALEN];   //MAC address of destination device.
} esp_now_send_param_t;

void esp_now_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status);
void esp_now_recv_cb(const esp_now_recv_info_t * esp_now_info, const uint8_t *data, int data_len);
esp_err_t InitEspNowChannel(void);




class ConnectToESPNOW : public SavingInterfaceClass{
	private:
		std::list< uint8_t[ESP_NOW_ETH_ALEN]> _llClientMacs;
		bool _isConfigured;
	public:
		ConnectToESPNOW(std::string name);
		cJSON* Save();
		void Load(cJSON*);
	
};

#endif /* MAIN_ESP_NOW_STUFF_H_ */
