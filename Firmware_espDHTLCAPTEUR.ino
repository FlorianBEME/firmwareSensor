#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <PubSubClient.h>


#define DHTPIN 4     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

const char* ssid = "Livebox-FLOLAU";
const char* password = "F07101991BEME";
const char* mqtt_server = "192.168.1.27";


const char* serverName = "http://api.thingspeak.com/update";
String apiKey = "ST1CBUD3NFWTOJPP";

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

int ledPinBlue = 16;
int ledPinRed = 14;


DHT dht(DHTPIN, DHTTYPE);
PubSubClient MQTT_CLIENT;
WiFiClient client;

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);
  // Wait for serial to initialize.
  while (!Serial) { }

  pinMode(ledPinBlue, OUTPUT);
  pinMode(ledPinRed, OUTPUT);
  dht.begin();

  TestLedStart();

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");

  reconnectWifi();

}

int timeSinceLastRead = 0;
String field = "temp";

void signalSend() {
  for (int i = 0; i <= 2; i++) {
    digitalWrite(ledPinBlue, HIGH);
    delay(100);
    digitalWrite(ledPinBlue, LOW);
    delay(100);
  }
}

void TestLedStart() {
  digitalWrite(ledPinBlue, HIGH);
  delay(500);
  digitalWrite(ledPinBlue, LOW);
  delay(500);
  digitalWrite(ledPinRed, HIGH);
  delay(500);
  digitalWrite(ledPinRed, LOW);
  delay(500);
}

void humMore(float value) {
  float maxHum = (float) 80;
  Serial.println(maxHum);
  if (value > maxHum) {
    Serial.println("superieur");
    digitalWrite(ledPinRed, HIGH);
  } else {
    Serial.println("inf");
    digitalWrite(ledPinRed, LOW);
  }
}

void reconnectMqtt() {
  MQTT_CLIENT.setServer(mqtt_server, 1883);
  MQTT_CLIENT.setClient(client);
  String msg = "";
  while (!MQTT_CLIENT.connected()) {
    MQTT_CLIENT.connect(mqtt_server, "flo", "F07101991BEME");
    msg += ".";
    if (msg.length() > 36) {
      msg = ".";
      Serial.println(msg);
    } else {
      Serial.print(msg);
    }

    digitalWrite(ledPinBlue, HIGH);
    delay(500);
    digitalWrite(ledPinBlue, LOW);
    delay(500);
  }
  Serial.println("MQTT connected");
}

void reconnectWifi() {
  Serial.println("WiFi Disconnected-> Connexion en cours");
  for (int i = 0; i <= 4; i++) {
    digitalWrite(ledPinRed, HIGH);
    delay(500);
    digitalWrite(ledPinRed, LOW);
    delay(500);
  }

  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(ledPinRed, HIGH);
    delay(1500);
    digitalWrite(ledPinRed, LOW);
    delay(1500);
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(ledPinRed, LOW);
  digitalWrite(ledPinBlue, LOW);
}

void errorWithSensor() {
  for (int i = 0; i <= 7; i++) {
    digitalWrite(ledPinRed, HIGH);
    delay(200);
    digitalWrite(ledPinRed, LOW);
    delay(200);
  }
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      // Check if we're connected to the MQTT broker
      if (!MQTT_CLIENT.connected()) {
        reconnectMqtt();
      }

      float temp = dht.readTemperature();
      delay(100);
      float hum = dht.readHumidity();
      delay(100);

      if (isnan(temp) | isnan(hum)) {
        errorWithSensor();
        Serial.println("Failed to read from DHT sensor!");
        lastTime = 0;
        return;
      }

      Serial.print("humidity: ");
      Serial.print(hum);
      Serial.println("-----------");
      
      Serial.print("température: ");
      Serial.print(temp);
      Serial.println("-----------");



      // conversion float->char
      char tempString[8];
      dtostrf(temp, 1, 2, tempString);

      char humString[8];
      dtostrf(hum, 1, 2, humString);


      MQTT_CLIENT.publish("espSensor/cuisine/temp", tempString);

    
      MQTT_CLIENT.publish("espSensor/cuisine/hum", humString);


      digitalWrite(ledPinBlue, HIGH);
      delay(200);
      digitalWrite(ledPinBlue, LOW);
      delay(200);


    }
    else {
      reconnectWifi();
    }
    lastTime = millis();
  }
}
