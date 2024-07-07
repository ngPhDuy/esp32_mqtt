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
Adafruit_NeoPixel rgb(4, D3, NEO_GRB + NEO_KHZ800);

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

void TaskBlink(void* params);
void TaskReadTemperatureAndHumidity(void* params);
void TaskAutoRelay(void* params);
void TaskAutoLed(void* params);
void TaskAutoFan(void* params);
void TaskSendToBroker(void* params);

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

  pinMode(D5, OUTPUT);
  xTaskCreate( TaskBlink, "Task Blink" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskReadTemperatureAndHumidity, "Task Read Temp And Humi" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskAutoRelay, "Task Auto Relay" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskAutoLed, "Task Auto Led" ,2048  ,NULL  ,2 , NULL);
  // xTaskCreate( TaskAutoFan, "Task Auto Fan" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskSendToBroker, "Task Send To Broker" ,2048  ,NULL  ,2 , NULL);
}

void loop() {
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

void messageHandler(String &topic, String &payload) {
  Serial.println("ESP32 - received from MQTT:");
  Serial.println("- topic: " + topic);
  Serial.println("- payload:");
  Serial.println(payload);
  if (payload == "On" || payload == "on") {
    analogWrite(D5, 255);
    // rgb.fill(rgb.Color(255,102,0));
    // rgb.show();
  }
  if (payload == "Off" || payload == "off") {
    analogWrite(D5, 0);
    // rgb.fill(rgb.Color(0,0,0));
    // rgb.show();
  }
  // You can process the incoming data as json object, then control something
  /*
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  */
}
void TaskBlink(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  

  while(1) {                          
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED ON
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED OFF
    delay(200);
  }
}


void TaskReadTemperatureAndHumidity(void* params){  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  while(1) {                          
    Serial.println("Task Read Temperature and Humidity");
    dht.read();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.print(dht.getTemperature());
    lcd.setCursor(0,1);
    lcd.print("Humi: ");
    lcd.print(dht.getHumidity());
    delay(5000);
  }
}

void TaskAutoRelay(void* params) {

  pinMode(D9, OUTPUT);

  while(1) {
    Serial.println("Task Auto Relay: " + analogRead(A3));
  
    if (analogRead(A3) > 500) {
      digitalWrite(D9, LOW);
    }

    if (analogRead(A3) < 50) {
      digitalWrite(D9, HIGH);
    }

    delay(1000);
  }
}

void TaskAutoLed(void* params) {
  pinMode(A0, INPUT);

  while(1) {
    Serial.println("Task Auto Led");

    int sensorValue = analogRead(A0);
    if (sensorValue > 1000) {
      rgb.setPixelColor(0, rgb.Color(0,0,0));
      rgb.setPixelColor(1, rgb.Color(0,0,0));
      rgb.setPixelColor(2, rgb.Color(0,0,0));
      rgb.setPixelColor(3, rgb.Color(0,0,0));
      rgb.show();
    }
    if (sensorValue < 999) {
      rgb.setPixelColor(0, rgb.Color(255,255,255));
      rgb.setPixelColor(1, rgb.Color(255,255,255));
      rgb.setPixelColor(2, rgb.Color(255,255,255));
      rgb.setPixelColor(3, rgb.Color(255,255,255)); 
      rgb.show();    
    }

    delay(1000);
  }
}

void TaskAutoFan(void* params) {
  pinMode(D5, OUTPUT);
  while(1) {
    Serial.println("Task Auto Fan");
    if (dht.getTemperature() >= 26.30) {
      analogWrite(D5, 0);
    } 
    if (dht.getTemperature() < 26.20) {
      analogWrite(D5, 255);
    }
    delay(1000);
  }
}

void TaskSendToBroker(void* params) {
  while(1) {
    Serial.println("Task Send To Broker");
    mqtt.loop();
    // sendToMQTT();
    delay(1000);
  }
}

void sendToMQTT() {
  StaticJsonDocument<200> message;
  dht.read();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(dht.getTemperature());
  lcd.setCursor(0,1);
  lcd.print(dht.getHumidity());
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