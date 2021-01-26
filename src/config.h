#ifndef CONFIG_H
#define CONFIG_H

/*
 * This example demonstrate how to use asynchronous client & server APIs
 * in order to establish tcp socket connections in client server manner.
 * server is running (on port 7050) on one ESP, acts as AP, and other clients running on
 * remaining ESPs acts as STAs. after connection establishment between server and clients
 * there is a simple message transfer in every 2s. clients connect to server via it's host name
 * (in this case 'esp_server') with help of DNS service running on server side.
 *
 * Note: default MSS for ESPAsyncTCP is 536 byte and defualt ACK timeout is 5s.
*/

//**********************************************************************************/
// MQTT + WiFi display constants
/**********************************************************************************/
#define SSID "Nescafe"
#define WIFI_PASSWORD "nopassword"

#define BROKER_HOST_NAME "homeassistant"
#define BROKER_PORT 1883
#define MQTT_USER "buff"
#define MQTT_PASSWORD "mammas"
#define TOTAL_TOPIC "power/meter/total/current"
#define HEATER_TOPIC "power/meter/heater/current"
#define FTX_TOPIC "power/meter/ftx/current"
#define HOUSE_HOLD_TOPIC "power/meter/house_hold/current"
#define DISCOVERY_PREFIX "homeassistant"

//**********************************************************************************/
// oled display constants
/**********************************************************************************/
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3c ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define PPWH_1 1
#define PPWH_08 0.8
#define COUNTERS_COUNT 3 //Set the number of counters used
#define TIME_CONST 49350 // Adjust to get roughly 60 seconds for every main loop
#define DEBUG true

#endif // CONFIG_H
