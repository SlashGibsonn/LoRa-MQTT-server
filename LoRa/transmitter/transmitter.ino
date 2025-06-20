#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_AHTX0.h>
#include <ArduinoJson.h>

Adafruit_AHTX0 aht;

volatile byte rpmcount;
volatile unsigned long last_micros;
unsigned long timeold;
const float timemeasure = 10.0; // detik pengukuran
int GPIO_pulse = 2;      
float kecepatan_meter_per_detik;
volatile boolean flag = false;

void rpm_anemometer() {
  flag = true;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!aht.begin()) {
    Serial.println("Sensor AHT20 tidak ditemukan!");
    while (1);
  }

  pinMode(GPIO_pulse, INPUT_PULLUP);
  digitalWrite(GPIO_pulse, LOW);
  detachInterrupt(digitalPinToInterrupt(GPIO_pulse));
  attachInterrupt(digitalPinToInterrupt(GPIO_pulse), rpm_anemometer, RISING);
  rpmcount = 0;
  timeold = millis();

  Serial.println("Memulai LoRa...");
  if (!LoRa.begin(433E6)) {
    Serial.println("Gagal memulai LoRa!");
    while (1);
  }
  LoRa.setTxPower(20);
  Serial.println("LoRa siap");
}

void loop() {
  if (flag == true) {
    if ((micros() - last_micros) >= 5000) {
      rpmcount++;
      last_micros = micros();
    }
    flag = false;
  }

  if ((millis() - timeold) >= timemeasure * 1000) {
    detachInterrupt(digitalPinToInterrupt(GPIO_pulse));

    float rotasi_per_detik = float(rpmcount) / timemeasure;
    kecepatan_meter_per_detik = ((-0.0181 * pow(rotasi_per_detik, 2)) + (1.3859 * rotasi_per_detik) + 1.4055);
    if (kecepatan_meter_per_detik <= 1.5) {
      kecepatan_meter_per_detik = 0.0;
    }

    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    float suhu = temp.temperature;
    float kelembaban = humidity.relative_humidity;

    // Cetak ke Serial Monitor
    // Serial.print("Suhu: ");
    // Serial.print(suhu);
    // Serial.print(" C, Kelembaban: ");
    // Serial.print(kelembaban);
    // Serial.print(" %, Kecepatan: ");
    // Serial.print(kecepatan_meter_per_detik);
    // Serial.println(" m/s");

    StaticJsonDocument<128> doc;
    doc["t"] = suhu;
    doc["h"] = kelembaban;
    doc["w"] = kecepatan_meter_per_detik;

    char buffer[128];
    serializeJson(doc, buffer);

    LoRa.beginPacket();
    LoRa.print(buffer);
    LoRa.endPacket();

    Serial.print("Dikirim: ");
    Serial.println(buffer);

    rpmcount = 0;
    timeold = millis();
    attachInterrupt(digitalPinToInterrupt(GPIO_pulse), rpm_anemometer, RISING);
  }
}
