#include "config.h"
/**
 * Talegur Arduino Device Software
 * Author; AIT, Ralf Hundertmark
 */

// Function Prototypes (Note Different from Arduino - Those need to be defined
// before function can be called)
void initStrings();
void initWifi(const char *ssid, const char *password);
void mqttConnect(const char *mqttHost, unsigned int mqttPort, const char *user,
                 const char *password);
void mqttReconnect();
void mqttPublishData(String sensorId, String clazz, const char *opsTopicBase,
                     String payload);
void SensorOperationsObject(String &target, String sensorId, String value);
boolean mqttGetConnected();
boolean readAndPublish();
void printConfigSummary();

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

long lastReportTime;
int reportInterval;

/**
 * setup - The standard setup routine for talegur devices
 * @method setup
 * Init hardware
 * Init WiFiClient and connect to MQTT
 */

void initStrings() {
  SSID = "YOUR WIFI SSID";
  WIFI_PASSWORD = "YOUR WIFI PASSWORD";
  MQTT_SERVER = "CONTACT US at talegur@ait.co.th for credentials";
  MQTT_PORT = "CONTACT US at talegur@ait.co.th for credentials";
  MQTT_USER = "CONTACT US at talegur@ait.co.th for credentials";
  MQTT_PW = "CONTACT US at talegur@ait.co.th for credentials";
  SENSOR_NAME = "Up tp you";
  SENSOR_TYPE = "CONTACT US at talegur@ait.co.th for credentials";
  SENSOR_TYPE_FOR_TOPIC = "CONTACT US at talegur@ait.co.th for credentials";
  SENSOR_ID = "CONTACT US at talegur@ait.co.th for credentials";
  O_TOPIC = "CONTACT US at talegur@ait.co.th for credentials";
  C_RESP = "CONTACT US at talegur@ait.co.th for credentials";
  C_TOPIC = "CONTACT US at talegur@ait.co.th for credentials";
}

/**
 * initWifi connects to the configured WiFi network and gets the time
 * @method initWifi
 * @param  ssid
 * @param  password
 */
void initWifi(const char *ssid, const char *password) {
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected");
  Serial.print("talegur: waiting for time - ");
  // 7 *3600 in the line below means the UTC offset - in Thailand that is 7 hours
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t this_second = 0;
  // wait until network has actual time
  while (!this_second) {
    delay(500);
    time(&this_second);
    Serial.print(".");
  }
  time(&this_second); // Here we get the accurate time
  Serial.print(" The time is: ");
  Serial.print(ctime(&this_second));
}

void setup() {
  // this line below is just so that the serial monitor can start and we do not loose initial messages
  delay(5000); 
  Serial.begin(9600);
  Serial.println("\n\ntalegur: starting device");
  initStrings();

  lastReportTime = 0;
  reportInterval = (1 * 60 * 1000); // 1 Minute

  delay(1000); // looks stupid ? - Thanks to slow hardware that seems required

  Serial.print("talegur: Attempting Connect: SSID=");
  Serial.print(SSID);

  initWifi(SSID, WIFI_PASSWORD);
  mqttConnect(MQTT_SERVER, atoi(MQTT_PORT), MQTT_USER, MQTT_PW);
  // printConfigSummary();

  // read and publih an initial data point
  readAndPublish();
}

/**
 * getDataPoint here is ONLY A SIMULATION.
 *              IRL this should be data reading the IO ports to
 *              which the sensor is connected
 * @method getDataPoint
 * @param  min          for SIMULATION
 * @param  max          for SIMULATION
 * @return              true if read success / fales if read failed
 */
boolean getDataPoint(int min, int max) {
  dataPoint.value = random(min * 100, max * 100) / 100.0;
  return true;
} // end of getDataPoint

/**
 * loop -- here we:
 *         SLEEP till samplingRate is reached
 *         Read a dataPoint
 *         Publish
 * @method loop
 */
void loop() {
  long now = millis();
  if ((lastReportTime + reportInterval) < now) {
    lastReportTime = now;
    readAndPublish();
    // uncomment the two lines below if you need to find out how much memory is
    // left uint32_t free = system_get_free_heap_size();
    // Serial.println("talegur: Free memory:" + String(free) + " bytes");
  } // end if
  delay(reportInterval);
} // end of Loop

/**
 * readAndPublish a dataPoint
 * @method readAndPublish
 * @return return code from getDataPoint (boolean)
 */
boolean readAndPublish() {
  String payload;
  boolean rc = getDataPoint(30, 150);
  SensorOperationsObject(payload, String(SENSOR_ID), String(dataPoint.value));
  Serial.println(payload);
  mqttPublishData(String(SENSOR_ID), SENSOR_TYPE_FOR_TOPIC, (const char *)O_TOPIC,
                  payload);
  return rc;
} // end of readAndPublish

/**
 * getMacAddress
 * @method getMacAddress
 * @return a hex value
 */
String getMacAddress() { return WiFi.macAddress(); }

/**
 * getIpAddress
 * @method getIpAddress
 * @return string in 4-segment . notation of IPV4
 */
String getIpAddress() { return WiFi.localIP().toString(); }

/**
 * mqttConnect connects to the MQTT server
 * @method mqttConnect
 * @param  mqttHost    The MQTT Host even if tempting do NOT use localhost
 * @param  mqttPort    the MQQT SERVER Talegur is using
 * @param  user        the MQTT User
 * @param  password    and her password
 */
void mqttConnect(const char *mqttHost, unsigned int mqttPort, const char *user,
                 const char *password) {

  mqttClient.setServer(mqttHost, mqttPort); // set in app config.h

  while (!mqttClient.connect("AMDISX", user, password)) {
    Serial.println("Attempting to connect to AMDISX MQTT broker");
    delay(10); // generally agood idea to have a short delay they say in the
               // interwebs :-(
  }
  Serial.println("talegur: Connected to AMDISX MQTT broker");
} // end of mqttConnect

/**
 * mqttReconnect
 * @method mqttReconnect
 */
void mqttReconnect() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect("AMDISX", (const char *)MQTT_USER,
                           (const char *)MQTT_PW)) {
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" - try again in a few seconds");
      delay(5000);
    }
  }
} // end of reconnect

/**
 * [mqttPublishData description]
 * @method mqttPublishData
 * @param  sensorId        [description]
 * @param  clazz           [description]
 * @param  opsTopicBase    [description]
 * @param  payload         [description]
 */
void mqttPublishData(String sensorId, String clazz, const char *opsTopicBase,
                     String payload) {
  String opsTopic;
  while (!mqttClient.connected()) {
    mqttReconnect();
  }

  if (mqttClient.connected()) {
    int rc;
    opsTopic = String(opsTopicBase);
    opsTopic.concat(sensorId);
    rc = mqttClient.publish(opsTopic.c_str(), payload.c_str());
    if (!rc) {
      Serial.print("talegur: Publish failed:");
      Serial.println(opsTopic);
    }
  } else {
    Serial.println("talegur: MQTT connect failed. Will reset and try again..");
  }
}

/**
 * [mqttGetConnected description]
 * @method mqttGetConnected
 * @return [description]
 */
boolean mqttGetConnected() { return mqttClient.connected(); }

/**
 * [printConfigSummary description]
 * @method printConfigSummary
 * @param  host               [description]
 * @param  port               [description]
 * @param  user               [description]
 */
void printConfigSummary() {
  char buf[64];
  Serial.println("talegur.: Init Summary ---------------------------------");
  sprintf(buf, "talegur.: MQTT_HOST=%s",MQTT_SERVER);
  Serial.println(buf);
  sprintf(buf, "talegur.: MQTT_PORT=%s",MQTT_PORT);
  Serial.println(buf);
  sprintf(buf, "talegur.: MQTT_USER=%s",MQTT_USER);
  Serial.println(buf);
  Serial.println("talegur.: -------------------------------------------------");
}

void SensorOperationsObject(String &target, String sensorId, String value) {
  StaticJsonDocument<1024> amdisxObject;
  time_t now = time((time_t)NULL);
  struct tm *timeinfo = gmtime(&now);
  char eventTime[32];
  sprintf(eventTime, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d", 1900 + timeinfo->tm_year,
          timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour,
          timeinfo->tm_min, timeinfo->tm_sec);
  amdisxObject["sensorId"] = sensorId;
  amdisxObject["clazz"] = "SensorValue";
  amdisxObject["measuredAt"] = eventTime;
  amdisxObject["value"] = atof(value.c_str());
  serializeJson(amdisxObject, target);
}