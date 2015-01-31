
#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__
#include "mqtt.h"

// HTTPD USER CONFIG
#define USE_WIFI_MODE		STATIONAP_MODE
#define WIFI_CLIENTSSID		"KLP"
#define WIFI_CLIENTPASSWORD	"charly78"
#define WIFI_AP_NAME		"ESP8266"
#define WIFI_AP_PASSWORD	"00000000"
#define PLATFORM_DEBUG		true

#define CFG_HOLDER	0x00FF55A4  /* Change this value to load default configurations */
#define CFG_LOCATION	0x3C	/* Please don't change or if you know what you doing */

// MQTT USER CONFIG

#define MQTT_HOST			"192.168.254.30" //or "192.168.11.1"
#define MQTT_PORT			1883
#define MQTT_BUF_SIZE		1024
#define MQTT_KEEPALIVE		60	 /*second*/

#define MQTT_CLIENT_ID		"LR_HEATER1"
#define MQTT_USER			""
#define MQTT_PASS			""


#define STA_SSID WIFI_CLIENTSSID
#define STA_PASS WIFI_CLIENTPASSWORD
#define STA_TYPE AUTH_WPA2_PSK

#define MQTT_RECONNECT_TIMEOUT 	5	/*second*/

#define DEFAULT_SECURITY	0

#define QUEUE_BUFFER_SIZE		 		2048

// Static definitions from MQTT user_init.c


#define PIN_GPIO 2
#define PIN_GPIO2_MUX PERIPHS_IO_MUX_GPIO2_U
#define PIN_GPIO2_FUNC FUNC_GPIO2
#define TOPIC_GPIO2 "LR_HEATER1_GPIO2" // USE THIS AS INPUT TOPIC
#define TOPIC_CB "LR_HEATER1_GPIO2_CB" // USE THIS FOR CALLBACK TO OPENHAB/MQTT

MQTT_Client mqttClient;
char currGPIO2State;

#endif
