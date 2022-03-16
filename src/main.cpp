#include <Arduino.h>

#include <string.h>

#include <stdint.h>

#include "Adafruit_Sensor.h"
#include "DHT.h"

#include "WiFi.h"
#include "WiFiClient.h"
#include "IPAddress.h"

// https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_basic/mqtt_basic.ino
#include "PubSubClient.h"

// wifi and mqtt config
#include "config.h"

#define DHTPIN 23
#define DHTTYPE DHT11

#define LEDPIN 19

DHT dht(DHTPIN, DHTTYPE);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

#define DELAY_MS(ms) vTaskDelay(ms / portTICK_PERIOD_MS)

void vTaskReadSensor(void *pvParameters)
{
    float h, t;

    char msg[50];

    for (;;)
    {
        h = dht.readHumidity();
        t = dht.readTemperature();

        if (isnan(h) || isnan(t))
        {
            Serial.println(F("Failed to read from DHT sensor!"));
        }
        else
        {
            sprintf(msg, "Temperature: %.2fC, Humidity: %.2f%%", t, h);

            Serial.println(msg);

            mqttClient.publish(MQTT_T_PUB_DHT11, msg);
        }

        DELAY_MS(1000);
    }
}

void vTaskMqttStayActive(void *pvParameters)
{
    for (;;)
    {
        mqttClient.loop();
        DELAY_MS(100);
    }
}

void ledSetCb(char *topic, byte *payload, unsigned int length)
{
    // only one topic no need to check

    if (strncmp((char *)payload, "1", length) == 0)
    {
        digitalWrite(LEDPIN, HIGH);
    }
    else if (strncmp((char *)payload, "0", length) == 0)
    {
        digitalWrite(LEDPIN, LOW);
    }
    else if (strncmp((char *)payload, "2", length) == 0)
    {
        digitalWrite(LEDPIN, !digitalRead(LEDPIN));
    }
}

void setup()
{
    /* Serial */
    Serial.begin(9600);
    Serial.println(F("DHT11-MQTT test!"));

    /* DHT11 */
    dht.begin();

    /* LED */
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);

    /* WIFI */
    // https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClient/WiFiClient.ino
    Serial.print(F("WiFi connecting to: "));
    Serial.println(WIFI_SSID);

    WiFi.hostname(WIFI_HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PSK);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(F("."));
    }

    Serial.println(F("WiFi connected!"));
    Serial.print(F("IP-address: "));
    Serial.println(WiFi.localIP());

    /* mqtt client */
    Serial.print(F("MQTT connecting to: "));
    Serial.println(MQTT_SERV);

    mqttClient.setServer(MQTT_SERV, MQTT_SERV_PORT);
    while (!mqttClient.connected())
    {
        mqttClient.connect(MQTT_ID, MQTT_USER, MQTT_PASS);
        delay(500);
        Serial.print(F("."));
    }

    Serial.println(F("MQTT connected!"));

    mqttClient.subscribe(MQTT_T_SUB_LED);
    mqttClient.setCallback(ledSetCb);

    /* start tasks*/
    xTaskCreate(
        vTaskMqttStayActive, // task func
        "mqttClient",        // name
        2000,                // stacksize
        NULL,                // params
        1,                   // priority
        NULL                 // handle
    );

    xTaskCreate(
        vTaskReadSensor, // task func
        "readsensor",    // name
        2000,            // stacksize
        NULL,            // params
        1,               // priority
        NULL             // handle
    );
}

void loop()
{
}