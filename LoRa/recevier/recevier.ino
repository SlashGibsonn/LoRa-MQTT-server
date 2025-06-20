#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define ss    15  // D8 - GPIO15
#define rst   2   // D4 - GPIO2
#define dio0  4   // D2 - GPIO4

const char* ssid = "Yomel 2";
const char* password = "..";
const char* mqtt_server = "10.255.69.10"; 

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(100);
  Serial.println();
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi terhubung, IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    String clientId = "LoRaReceiver-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Terhubung ke MQTT");
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Callback MQTT jika dibutuhkan
}

void setup() {
  Serial.begin(9600);
  delay(2000);

  Serial.println("LoRa Receiver - ESP8266");

  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa gagal dimulai!");
    while (1);
  }
  Serial.println("LoRa siap");

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }

    Serial.print("Data diterima: ");
    Serial.println(receivedData);

    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, receivedData);

    if (error) {
      Serial.print(F("deserializeJson() gagal: "));
      Serial.println(error.f_str());
      return;
    }

    float suhu = doc["t"];
    float kelembaban = doc["h"];
    float kecepatan = doc["w"];

    Serial.print("Suhu (°C): ");
    Serial.println(suhu);
    Serial.print("Kelembaban (%): ");
    Serial.println(kelembaban);
    Serial.print("Kecepatan Angin (m/s): ");
    Serial.println(kecepatan);

    char output[128];
    serializeJson(doc, output);
    client.publish("/home/sensors", output);
    Serial.println("Dikirim ke MQTT broker\n");
  }
}
