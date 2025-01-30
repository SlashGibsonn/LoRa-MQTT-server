#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN 4   
#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender");

  dht.begin();

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  } 
}

void loop() {
 
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Membuat JSON
  StaticJsonDocument<128> doc;
  doc["counter"] = counter;
  doc["t"] = t;
  doc["h"] = h;

  // Mengubah JSON menjadi string
  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  Serial.print("Sending packet: ");
  Serial.println(jsonBuffer);

  // Mengirim data melalui LoRa
  LoRa.beginPacket();
  LoRa.print(jsonBuffer);
  LoRa.endPacket();

  counter++;
  
  delay(5000); 
}
