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
#define HYGROMETER_ENABLE_PIN 5    // D1
#define PHOTORESISTOR_ENABLE_PIN 4 // D2

ESP8266WebServer server(80);
DHTesp dht;

float humidity, temperature, heatIndex;
int brightness, moisture;
String response = "";

// The timestamp of the last time the sensor was read
unsigned long previousMillis = 0;
// Interval at which to read sensor
const long interval = 2000;

void setup(void) {
  // Begin serial output
  Serial.begin(9600);
  Serial.println("\n\r \n\rWireless ESP8266 sensor");

  // Initialize pins
  pinMode(HYGROMETER_ENABLE_PIN, OUTPUT);
  pinMode(PHOTORESISTOR_ENABLE_PIN, OUTPUT);
  digitalWrite(HYGROMETER_ENABLE_PIN, LOW);
  digitalWrite(PHOTORESISTOR_ENABLE_PIN, LOW);

  // Initialize temperature sensor
  dht.setup(DHT_PIN, DHT_TYPE);

  // Connect to WiFi network
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

  // Start web server
  server.on("/", handleRequest);
  server.begin();
  Serial.println("HTTP server started");
}

void handleRequest() {
  // Read DHT
  readDHTSensor();

  // Read analog sensors, using multiplexing
  readAnalogSensors();

  // Return the response
  server.send(200, "application/json", createResponseAsJson());
  delay(100);
}

String createResponseAsJson() {
  response = "{";
  if (!isnan(temperature)) {
    response += ("\"temperature\": " + String(temperature, 1) + ", ");
  }
  if (!isnan(humidity)) {
    response += ("\"humidity\": " + String(humidity, 1) + ", ");
  }
  response += ("\"brightness\": " + String((brightness * 100) / 1024) + ", ");
  response += ("\"moisture\": " + String(((1024 - moisture) * 100) / 1024));
  response += "}";
  return response;
}

void loop(void) { server.handleClient(); }

void readDHTSensor() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow
    // sensor)
    humidity = dht.getHumidity();
    temperature = dht.getTemperature();
    heatIndex = dht.computeHeatIndex(temperature, humidity, false); // false == celcius, true == fahrenheit

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
    }
  }
}

void readAnalogSensors() {
  // Analog sensors are multiplexed, as there is only one analogue input pin.
  readHygrometer();
  readPhotoresistor();
}

void readHygrometer() {
  digitalWrite(HYGROMETER_ENABLE_PIN, HIGH);
  delay(100); // Wait for high signal
  moisture = analogRead(A0);
  digitalWrite(HYGROMETER_ENABLE_PIN, LOW);
}

void readPhotoresistor() {
  digitalWrite(PHOTORESISTOR_ENABLE_PIN, HIGH);
  delay(100); // Wait for high signal
  brightness = analogRead(A0);
  digitalWrite(PHOTORESISTOR_ENABLE_PIN, LOW);
}
