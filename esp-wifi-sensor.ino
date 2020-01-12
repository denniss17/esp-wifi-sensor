/*
 * Wireless sensor based on an ESP8266 Module.
 *
 * Please consult the README for further instructions.
 */
#include "config.h"
#include <DHTesp.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define DHT_TYPE DHTesp::DHT22
#define DHT_PIN 2                  // D4

#define AUTHORIZATION_HEADER "Authorization"
#define CONTENT_TYPE_HEADER "Content-Type"
#define JSON_MEDIATYPE "application/json"

DHTesp dht;

float humidity, temperature, heatIndex;
String body;

void setup(void) {
  body.reserve(64);
  body = "";

  setupSerial();
  setupTemperatureSensor();
  connectWifi();

  delay(500);

  readDHTSensor();
  sendData();

  deepSleep();
}

void setupSerial() {
  Serial.begin(9600);

  // Print some information
  Serial.println();
  Serial.println("Wireless low-energy ESP8266 sensor");
}

void setupTemperatureSensor() { dht.setup(DHT_PIN, DHT_TYPE); }

void connectWifi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to '");
  Serial.print(ssid);
  Serial.print("'");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

void sendData() {
  if(!isnan(temperature) && WiFi.status()== WL_CONNECTED){

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, homeassistantUrl)) {

      http.addHeader(CONTENT_TYPE_HEADER, JSON_MEDIATYPE);
      http.addHeader(AUTHORIZATION_HEADER, homeassistantToken);
      body = "{\"state\":" + String(temperature, 1) + "}";

      Serial.printf("[HTTP] POST url: %s\n            body: ", homeassistantUrl);
      Serial.println(body);
      int httpCode = http.POST(body);

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] POST code: %d\n", httpCode);

        String payload = http.getString();  //Get the response payload
        Serial.println(payload); //Print request response payload
      } else {
        Serial.printf("[HTTP] POST failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP] Unable to connect\n");
    }
  } else {
    Serial.println("Error in WiFi connection");
  }
}

void readDHTSensor() {
  humidity = convertHumidity(dht.getHumidity());
  temperature = convertTemperature(dht.getTemperature());
  heatIndex = dht.computeHeatIndex(temperature, humidity, false); // false == celsius, true == fahrenheit

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  }
}

void deepSleep() {
  Serial.printf("Entering deepsleep for %d s\n", sleepTime/1000000);
  ESP.deepSleep(sleepTime);
}

// Functions to convert the measurements to a more sane value or to calibrate the sensors.
float convertTemperature(float temperature){
  return temperature + temperatureOffset;
}

float convertHumidity(float humidity) {
  return humidity + humidityOffset;
}

void loop() {

}
