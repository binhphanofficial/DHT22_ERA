
/*************************************************************
  Download latest ERa library here:
    https://github.com/eoh-jsc/era-lib/releases/latest
    https://www.arduino.cc/reference/en/libraries/era
    https://registry.platformio.org/libraries/eoh-ltd/ERa/installation

    ERa website:                https://e-ra.io
    ERa blog:                   https://iotasia.org
    ERa forum:                  https://forum.eoh.io
    Follow us:                  https://www.fb.com/EoHPlatform
 *************************************************************/

// Enable debug console
// Set CORE_DEBUG_LEVEL = 3 first
// #define ERA_DEBUG

#define DEFAULT_MQTT_HOST "mqtt1.eoh.io"

// You should get Auth Token in the ERa App or ERa Dashboard
#define ERA_AUTH_TOKEN "01386544-********"
#define relay 27

#include <Arduino.h>
#include <ERa.hpp>
#include <ERa/ERaTimer.hpp>
#include <EEPROM.h>
#include "DHT.h"
#include <esp_task_wdt.h>



#define DHTPIN 21
#define DHTTYPE DHT22

const char ssid[] = "BinhPhan";
const char pass[] = "12345678";
float h, t;
int address = 0;
byte slide_range;
unsigned long previousMillis = 0;
const long interval = 2000;

ERaTimer timer;
DHT dht(DHTPIN, DHTTYPE);

void on_relay(void);
void off_relay(void);
void Read_DHT22(void);

ERA_WRITE(V2) {
  byte value = param.getInt();
  slide_range = value;
  EEPROM.write(address, value);
  Serial.print("Write ");
  Serial.print(value);
  Serial.println(" to EEPROM address 0");
  EEPROM.commit();
  delay(20);
  ERa.virtualWrite(V2, slide_range);
}

/* This function print uptime every second */
void timerEvent() {
  Read_DHT22();
  ERa.virtualWrite(V0, t);
  ERa.virtualWrite(V1, h);
  Serial.println("Online");
  ERA_LOG("Timer", "Uptime: %d", ERaMillis() / 1000L);
}


void setup() {
  /* Setup debug console */
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  off_relay();
  EEPROM.begin(512);
  slide_range = EEPROM.read(address);
  Serial.print("Read ");
  Serial.print(slide_range);
  Serial.println(" from EEPROM address 0");
  ERa.begin(ssid, pass);

  /* Setup timer called function every second */
  timer.setInterval(2000L, timerEvent);
  dht.begin();
  ERa.virtualWrite(V2, slide_range);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      esp_task_wdt_reset();
      Serial.println("Offline");
      Read_DHT22();
      WiFi.begin(ssid, pass);
    }
  } else {
    ERa.run();
    timer.run();
  }
}

void Read_DHT22(void) {
  h = dht.readHumidity();
  t = dht.readTemperature();
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
  if (t >= float(slide_range)) {
    on_relay();
  } else {
    off_relay();
  }
}
void on_relay(void) {
  digitalWrite(relay, 0);
}
void off_relay(void) {
  digitalWrite(relay, 1);
}