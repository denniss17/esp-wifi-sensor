# Wireless (WiFi) ESP8266 sensor

This directory contains firmware for running a wireless sensor on a ESP8266.
The sensor can be used to measure:
- Temperature and humidity (using a DHT22 sensor)
- Light (using a photoresistor)
- Dirt moisture (using a hygrometer like [this]( https://www.ebay.com/itm/Soil-Humidity-Hygrometer-Moisture-Detection-Sensor-Module-Arduino-w-Dupont-Wires/400985207745?epid=1369768833&hash=item5d5c94b3c1:g:~ogAAOxy4YdTUlFG))

## Requirements

- ESP8266
- Arduino IDE with ESP8266 board manager. Install by opening Arduino IDE and open "Tools" > "Board" > "Board Manager" from the menu. (http://arduino.esp8266.com/stable/package_esp8266com_index.json as "Additional Board Managers URLs" in the preferences of Arduino IDE)
- DHT22 sensor
- Arduino library "DHT sensor library". Install by opening Arduino IDE and open "Sketch" > "Include Library" > "Manage Libraries" from the menu.
- 1k ohm resistor
- Wires

## Wiring

### DHT22

Connect the DHT22 to the ESP as follows (when looking to front (grid) of DHT):

- 1st pin (VCC) of DHT22 -> 3.3 V
- 2nd pin (Data) of DHT22 -> D4
- 2nd pin (Data) of DTH22 -> 10k ohm -> GND (or 4th pin)
- 4th pin (GND) of DHT22 -> GND

### Photoresistor

Connect the photoresistor as follows:

- 1st pin -> D2
- 2nd pin -> Diode -> A0
- 2nd pin -> 100k ohm -> GND

### Hygrometer

Connect the hygrometer as follows:

- VCC -> D1
- GND -> GND
- AO -> Diode -> A0

## Installation

1. Copy `secrets.h.example` to `secrets.h`, and edit the settings to match you WiFi settings.
2. Flash using Arduino IDE

## Connecting with [home assistant](https://www.home-assistant.io/)

Make sure the sensor is connected and accessible via WiFi. Lookup the IP address, fix the IP address in your router (static DHCP) and access it in the browser. It should return something like:

    <example>

Now in the configuration of homeassistant, add something like:

    sensor:
    - platform: rest
      resource: http://192.168.3.1/
      value_template: '{{ value_json.temperature }}'
      name: "wifi_sensor_1_temperature"
      unit_of_measurement: "°C"
    - platform: rest
      resource: http://192.168.3.1/
      value_template: '{{ value_json.humidity }}'
      name: "wifi_sensor_1_humidity"
      unit_of_measurement: "%"

## Links

NodeMCU pin layout:

https://pradeepsinghblog.files.wordpress.com/2016/04/nodemcu_pins.png?w=616

Based on:

https://learn.adafruit.com/esp8266-temperature-slash-humidity-webserver/code

Analog port multiplexing based on:

http://www.instructables.com/id/ESP8266-with-Multiple-Analog-Sensors/
