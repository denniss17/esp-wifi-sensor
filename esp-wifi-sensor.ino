/*
 * Wireless sensor based on an ESP8266 Module.
 *
 * Please consult the README for further instructions.
 */
#include "config.h"
#include <DHTesp.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define DHT_TYPE DHTesp::DHT22
#define DHT_PIN 2                  // D4

// Analog sensors are multiplexed, as there is only one analogue input pin.
#define HYGROMETER_ENABLE_PIN 5    // D1
#define PHOTORESISTOR_ENABLE_PIN 4 // D2

#define JSON_MEDIATYPE "application/json"

ESP8266WebServer server(80);
DHTesp dht;

float humidity, temperature, heatIndex;
int brightness, moisture;
String response;

void setup(void) {
  response.reserve(128);
  response = "";

  setupSerial();
  setupTemperatureSensor();
  setupHygrometer();
  setupLightSensor();

  connectWifi();

  startWebserver();
}

void setupSerial() {
  Serial.begin(9600);

  // Print some information
  Serial.println();
  Serial.println("Wireless ESP8266 sensor");
  Serial.println(enableLightSensor ? "Lightsensor  enabled"
                                   : "Lightsensor  disabled");
  Serial.println(enableHygrometer ? "Hygrometer   enabled"
                                  : "Hygrometer   disabled");
}

void setupTemperatureSensor() { dht.setup(DHT_PIN, DHT_TYPE); }

void setupLightSensor() {
  if (enableLightSensor) {
    pinMode(PHOTORESISTOR_ENABLE_PIN, OUTPUT);
    digitalWrite(PHOTORESISTOR_ENABLE_PIN, LOW);
  }
}

void setupHygrometer() {
  if (enableHygrometer) {
    pinMode(HYGROMETER_ENABLE_PIN, OUTPUT);
    digitalWrite(HYGROMETER_ENABLE_PIN, LOW);
  }
}

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

void startWebserver() {
  server.on("/", handleRequest);
  server.begin();
  Serial.println("HTTP server started");
}

void handleRequest() {
  readSensors();

  // Return the response
  server.send(200, JSON_MEDIATYPE, createResponseAsJson());
  delay(100);
}

String createResponseAsJson() {
  response = "{";
  if (!isnan(temperature)) {
    response += ("\"temperature\":" + String(temperature, 1) + ",");
  }
  if (!isnan(humidity)) {
    response += ("\"humidity\":" + String(humidity, 1) + ",");
  }
  if (!isnan(heatIndex)) {
    response += ("\"heatIndex\":" + String(heatIndex, 1) + ",");
  }
  if (enableLightSensor) {
    response += ("\"brightness\":" + String(brightness) + ",");
  }
  if (enableHygrometer) {
    response += ("\"moisture\":" + String(moisture));
  }

  // If response now ends with a comma, remove the comma
  if (response.endsWith(",")) {
    response.remove(response.length() - 1);
  }

  response += "}";
  return response;
}

void loop(void) { server.handleClient(); }

void readDHTSensor() {
    humidity = convertHumidity(dht.getHumidity());
    temperature = convertTemperature(dht.getTemperature());
    heatIndex = dht.computeHeatIndex(temperature, humidity, false); // false == celsius, true == fahrenheit

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
    }
  }
}

void readSensors() {
  readDHTSensor();
  readHygrometer();
  readPhotoresistor();
}

void readHygrometer() {
  digitalWrite(HYGROMETER_ENABLE_PIN, HIGH);
  delay(100); // Wait for high signal
  moisture = convertMoisture(analogRead(A0));
  digitalWrite(HYGROMETER_ENABLE_PIN, LOW);
}

void readPhotoresistor() {
  digitalWrite(PHOTORESISTOR_ENABLE_PIN, HIGH);
  delay(100); // Wait for high signal
  brightness = convertBrightness(analogRead(A0));
  digitalWrite(PHOTORESISTOR_ENABLE_PIN, LOW);
}


// Functions to convert the measurements to a more sane value or to calibrate the sensors.
float convertTemperature(float temperature){
  return temperature + temperatureOffset;
}

float convertHumidity(float humidity) {
  return humidity + humidityOffset;
}

int convertBrightness(int brightness) {
  return (brightness * 100) / 1024;
}

int convertMoisture(int moisture) {
  return ((1024 - moisture) * 100) / 1024;
}
