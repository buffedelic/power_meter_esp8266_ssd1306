#include <Arduino.h>
#include <OneWire.h>
#include <DS2423.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <iostream>
#include <string>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <config.h>


bool firstRun = true;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//**********************************************************************************/
// OneWire constants
// Counters present    1D 00 FD 0C 00 00 00 9B ||  1D 6C EC 0C 00 00 00 94
/**********************************************************************************/
const uint8_t ONE_WIRE_PIN = 0; // define the Arduino digital I/O pin to be used for the 1-Wire network here
uint8_t DS2423_address1[] = { 0x1D, 0x00, 0xFD, 0x0C, 0x00, 0x00, 0x00, 0x9B }; // define the 1-Wire address of the DS2423 counter here
uint8_t DS2423_address0[] = { 0x1D, 0x6C, 0xEC, 0x0C, 0x00, 0x00, 0x00, 0x94 }; // define the 1-Wire address of the DS2423 counter here
OneWire ow(ONE_WIRE_PIN);
DS2423 cnt0(&ow, DS2423_address0); // COUNTER 0, total, empty
DS2423 cnt1(&ow, DS2423_address1); // COUNTER 1, heatpump, empty

//**********************************************************************************/
// power calculation variables
/**********************************************************************************/
unsigned long currentTime;
unsigned long lastTime = millis();
unsigned long interval;
uint32_t count0, count1, lastCount0, lastCount1;
uint8_t wattTotal, wattHeater, wattFtx, wattHousehold;
char wattText[COUNTERS_COUNT + 1];

//**********************************************************************************/
// MQTT PubSub + WiFi
/**********************************************************************************/

WiFiClient espClient;
PubSubClient client(espClient);
long lastReconnectAttempt = 0;

//**********************************************************************************/
//**********************************************************************************/

//**********************************************************************************/
// WiFi functions
//**********************************************************************************/

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, WIFI_PASSWORD);

  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(150);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(350);
  }

  randomSeed(micros());
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//**********************************************************************************/
// MQTT functions
//**********************************************************************************/

boolean reconnect() {
  if (client.connect("powerClient", MQTT_USER, MQTT_PASSWORD)) {
    // Once connected, publish device config *************************************************
    //****************************************************************************************
    client.publish("outTopic","hello world");
    // ... and resubscribe
    //client.subscribe("inTopic");
  }
  return client.connected();
  
}

void publishPower(){

  char buffer[5];
  sprintf(buffer, "%5u", wattTotal);
  client.publish(TOTAL_TOPIC, buffer);
  sprintf(buffer, "%5u", wattHeater);
  client.publish(HEATER_TOPIC, buffer);
  sprintf(buffer, "%5u", wattFtx);
  client.publish(FTX_TOPIC, buffer);
  sprintf(buffer, "%5u", wattHousehold);
  client.publish(HOUSE_HOLD_TOPIC, buffer);
  Serial.println("-----------------------------------");
  Serial.println("| Published power usage to broker |");
  Serial.println("-----------------------------------");


}

//**********************************************************************************/
// OneWire functions
//**********************************************************************************/


void readCounters(uint32_t &count0, uint32_t &count1) {

    cnt0.update();
    if (cnt0.isError()) {
        Serial.println("Error reading counter");
    } else {
        count0 = cnt0.getCount(DS2423_COUNTER_A);
    }
    cnt1.update();
    if (cnt1.isError()) {
        Serial.println("Error reading counter");
    } else {
        count1 = cnt1.getCount(DS2423_COUNTER_A);
    }
}

void readPower(){
  // # P(w) = (3600 / T(s)) / ppwh
  // # watt = (3600000 / ((interval_millis / interval_count)) / 1) or 0.8
  // 400*16*1,73 = 11072W. Dvs ca 11,1kW. 3 fas "max" belastning
  // 400*20*1,73 = 13840W. Dvs ca 13,9kW. 3 fas "max" belastning
  
  currentTime = millis();
  double _interval = (currentTime - lastTime) / 1000.0000;

  readCounters(count0, count1);

  double cn0 = _interval / (count0 - lastCount0);
  double cn1 = _interval / (count1 - lastCount1);

  Serial.println("");
  Serial.println("DEBUG");
  Serial.println("***************************************************************************");
  Serial.println("Time: Interval, Current, Last:");
  Serial.print(_interval); Serial.print("="); Serial.print(currentTime); Serial.print("-"); Serial.println(lastTime);
  Serial.println("---------------------------------------------------------------------------");
  Serial.println("Counter 0: Total, Last, Count/sec:");
  Serial.print(count0); Serial.print("-"); Serial.print(lastCount0); Serial.print("="); Serial.println(cn0);
  Serial.println("---------------------------------------------------------------------------");
  Serial.println("Counter 1: Heater, Last, Count/sec:");
  Serial.print(count1); Serial.print("-"); Serial.print(lastCount1); Serial.print("="); Serial.println(cn1);  
  lastTime = currentTime;

  wattTotal = (3600.0000 / cn0) / PPWH_1;
  wattHeater = (3600.0000 / cn1) / PPWH_08;
  wattFtx = 0;
  wattHousehold = wattTotal - wattHeater - wattFtx;

  lastCount0 = count0;
  lastCount1 = count1;
}

//**********************************************************************************/
// OLED SSD1306 functions
//**********************************************************************************/

void updateScreen(char *_wattText[]) {

  _wattText[4] = {
    "Total:        " + (String)wattTotal + "W",
    "Heater:       " + (String)wattHeater + "W",
    "FTX:          " + (String)wattFtx + "W",
    "Household:    " + (String)wattHousehold + "W"
    };
  for(int i = 0; i < 4; i++) {
    Serial.println(_wattText[i]);
    }

  display.setCursor(0,0);             // Start at top-left corner
  display.clearDisplay();
  display.setTextSize(1);

  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.println("  Power Consumption ");
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.println("");

  if(firstRun) {
    display.println("Give me a minute!");
  }
  else {
    for(int i = 0; i < 4; i++) {
      display.println(text[i]);
    }
  }
  display.display();
}

void setup() {
  Serial.begin(115200);

  // Initialize display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    Serial.println(F("Check connections and restart device"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  display.clearDisplay();

  // Initialize OneWire counters
  cnt1.begin(DS2423_COUNTER_A|DS2423_COUNTER_B);
  cnt0.begin(DS2423_COUNTER_A|DS2423_COUNTER_B);
  readCounters(lastCount0, lastCount1); // initialize base point

  //Initialize WiFi
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(LED_BUILTIN, HIGH); // Turn LED off
  setup_wifi(); // LED blinks until connected

  // Initialize MQTT Client
  client.setServer(BROKER_HOST_NAME, BROKER_PORT);
  lastReconnectAttempt = 0;

  readPower();
  //updateScreen(text);

}

void loop() {
  // Initiate MQTT Client
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    client.loop();
  }

  readPower();
  publishPower();
  //updateScreen(text);
  firstRun = false;
  delay(TIME_CONST);
}
