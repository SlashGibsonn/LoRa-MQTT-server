#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DHTPIN 4 
#define DHTTYPE DHT11  
#define MQPIN A0

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Yomel 2";
const char* password = "Yomel123";
const char* mqtt_server = "192.168.2.92";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

void setup_wifi() {
  delay(100);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Placeholder for handling incoming messages if needed
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("outTopic", "hello world");
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  dht.begin(); 

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    float temp = dht.readTemperature();
    float hum = dht.readHumidity(); 
    float gasSensor = analogRead(MQPIN);
    float gas = map(gasSensor, 0, 1023, 0, 255);
    
    if (isnan(temp)|| isnan(hum)){
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(hum);
    Serial.print("Gas: ");
    Serial.println(gas);
    
    StaticJsonDocument<32> doc;
    char output[55];
    doc["t"] = temp;
    doc["h"] = hum;
    doc["g"] = gas;
    serializeJson(doc, output);
    client.publish("/home/temperature", output);
    Serial.println("Sent");
  }
}
