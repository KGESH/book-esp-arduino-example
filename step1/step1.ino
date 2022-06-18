#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "board_config.h"
#include "wifi_config.h"
#include "mqtt_config.h"


SoftwareSerial board(RX_PIN, TX_PIN);
WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);
unsigned long prevTime = 0;


void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    board.begin(115200);
    board.setTimeout(1000);

    setupWiFi(WIFI_SSID, WIFI_PASSWORD);
    setupMqtt(MQTT_BROKER_URL, MQTT_BROKER_PORT);
}

void loop() {
    if (!mqtt_client.connected()) {
        reconnect();
    }

    unsigned long now = millis();
    if (now - prevTime > 7000) {
        prevTime = now;
        /**
         * 7초마다 토픽 발행 */
        mqtt_client.publish("polling/1", "sensor alive");
    }

    mqtt_client.loop();
}


void setupMqtt(const char* mqtt_server, int port) {
    mqtt_client.setServer(mqtt_server, port);
    mqtt_client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
    board.print("MQTT 브로커에게 수신한 Topic : ");
    board.println(topic);

    board.print("MQTT 브로커에게 수신한 Payload : ");
    for (int i = 0; i < length; i++) {
        board.print((char) payload[i]);
    }
    board.println();
}

void reconnect() {
    const char* client_id = "my-esp-client";

    while (!mqtt_client.connected()) {
        if (mqtt_client.connect(client_id)) {
            /**
             * MQTT 브로커에 접속 성공했을때 수행 */
            board.println("MQTT 브로커 연결 성공");
            mqtt_client.publish("esp/connect", "브로커에 연결 성공");
            return;
        }

        /**
         * MQTT 브로커에 접속 실패했을때 수행 */
        board.print("failed, rc=");
        board.print(mqtt_client.state());
        board.println(" try again in 5 seconds");
        delay(5000);
    }
}


void setupWiFi(const char* ssid, const char* password) {
    delay(10);
    board.println();
    board.print("연결할 WiFi 이름: ");
    board.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        board.print(".");
    }
    randomSeed(micros());

    board.println("");
    board.println("WiFi connected");
    board.println("IP address: ");
    board.println(WiFi.localIP());
}
