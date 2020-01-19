#ifndef TALEGUR_CONFIG_h
#define TALEGUR_CONFIG_h

#include <Arduino.h>
#ifdef TARGET_NODE32
#include <WiFi.h>
#endif
#ifdef TARGET_ESP8285
#include <ESP8266WiFi.h>
#endif
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <time.h>
/**
 * MQQ Contants and variabvles
 */
DynamicJsonDocument jsonDoc(1024);
struct measurement_t {
  float value; // -100.00  to 100.00
} dataPoint;

const char *SSID;
const char *WIFI_PASSWORD;
const char *MQTT_SERVER;
const char *MQTT_PORT;
const char *MQTT_USER;
const char *MQTT_PW;
const char *SENSOR_NAME;
const char *SENSOR_TYPE;
const char *SENSOR_ID;
const char *O_TOPIC;
const char *C_RESP;
const char *C_TOPIC;

#endif
