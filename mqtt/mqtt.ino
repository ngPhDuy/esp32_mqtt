/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-mqtt
 */

#include <WiFi.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "DHT20.h"
#include "LiquidCrystal_I2C.h";
#include <Adafruit_NeoPixel.h>

#define CLIENT_ID "npd"  // CHANGE IT AS YOU DESIRE

DHT20 dht;
LiquidCrystal_I2C lcd(0x21,16,2);
Adafruit_NeoPixel rgb(4, D9, NEO_GRB + NEO_KHZ800);

const char WIFI_SSID[] = "ACLAB";              // CHANGE TO YOUR WIFI SSID
const char WIFI_PASSWORD[] = "ACLAB2023";           // CHANGE TO YOUR WIFI PASSWORD
const char MQTT_BROKER_ADRRESS[] = "broker.hivemq.com";  // CHANGE TO MQTT BROKER'S IP ADDRESS
const int MQTT_PORT = 1883;
const char MQTT_USERNAME[] = "";  // CHANGE IT IF REQUIRED
const char MQTT_PASSWORD[] = "";  // CHANGE IT IF REQUIRED

// The MQTT topics that this device should publish/subscribe
#define PUBLISH_TOPIC "esp32-001/send"
#define SUBSCRIBE_TOPIC "esp32-001/receive"

#define PUBLISH_INTERVAL 5000  // 4 seconds

WiFiClient network;
MQTTClient mqtt = MQTTClient(256);

unsigned long lastPublishTime = 0;

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("ESP32 - Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  connectToMQTT();

  lcd.init();
  lcd.backlight();

  rgb.begin();
}

void loop() {
  mqtt.loop();

  if (millis() - lastPublishTime > PUBLISH_INTERVAL) {
    sendToMQTT();
    lastPublishTime = millis();
  }
}

void connectToMQTT() {
  // Connect to the MQTT broker
  mqtt.begin(MQTT_BROKER_ADRRESS, MQTT_PORT, network);

  // Create a handler for incoming messages
  mqtt.onMessage(messageHandler);

  Serial.print("ESP32 - Connecting to MQTT broker");

  while (!mqtt.connect(CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  if (!mqtt.connected()) {
    Serial.println("ESP32 - MQTT broker Timeout!");
    return;
  }

  // Subscribe to a topic, the incoming messages are processed by messageHandler() function
  if (mqtt.subscribe(SUBSCRIBE_TOPIC))
    Serial.print("ESP32 - Subscribed to the topic: ");
  else
    Serial.print("ESP32 - Failed to subscribe to the topic: ");

  Serial.println(SUBSCRIBE_TOPIC);
  Serial.println("ESP32  - MQTT broker Connected!");
}

void sendToMQTT() {
  StaticJsonDocument<200> message;
  dht.read();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(dht.getTemperature());
  lcd.setCursor(0,1);
  lcd.print(dht.getHumidity());
  message["timestamp"] = millis();
  message["temp"] = dht.getTemperature();  // Or you can read data from other sensors
  message["humi"] = dht.getHumidity();
  char messageBuffer[512];
  serializeJson(message, messageBuffer);

  mqtt.publish(PUBLISH_TOPIC, messageBuffer);

  Serial.println("ESP32 - sent to MQTT:");
  Serial.print("- topic: ");
  Serial.println(PUBLISH_TOPIC);
  Serial.print("- payload:");
  Serial.println(messageBuffer);
}

void messageHandler(String &topic, String &payload) {
  Serial.println("ESP32 - received from MQTT:");
  Serial.println("- topic: " + topic);
  Serial.println("- payload:");
  Serial.println(payload);
  if (payload == "On" || payload == "on") {
    rgb.fill(rgb.Color(255,102,0));
    rgb.show();
  }
  if (payload == "Off" || payload == "off") {
    rgb.fill(rgb.Color(0,0,0));
    rgb.show();
  }
  // You can process the incoming data as json object, then control something
  /*
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  */
}
