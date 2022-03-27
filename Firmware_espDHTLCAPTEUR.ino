#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#include "arduino_secrets.h"

#define DHTPIN 4     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

const char* ssid = SECRET_SSID ;
const char* password = SECRET_PASS;
const char* mqtt_server = SECRET_MQTT_SERVER ;

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

int ledPinBlue = 16;
int ledPinRed = 14;
int ledPinGreen = 5;


DHT dht(DHTPIN, DHTTYPE);
PubSubClient MQTT_CLIENT;
WiFiClient client;

void setup() {
  // Initialisation de la console
  Serial.begin(9600);
  Serial.setTimeout(2000);
  while (!Serial) { }
  Serial.println("Device Started");
  Serial.println("-------------------------------------");

  // Initialisation des led
  pinMode(ledPinBlue, OUTPUT);
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  TestLedStart();
  delay(1000);

  // Initialisation SENSOR
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");
  dht.begin();
  digitalWrite(ledPinBlue, HIGH);
  Serial.println("DHT READY!");
  Serial.println("-------------------------------------");
  delay(2000);

  // Initialisation Wifi
  Serial.println("Connection Wifi start...");
  Serial.println("-------------------------------------");
  reconnectWifi();
  digitalWrite(ledPinRed, HIGH);
  Serial.println("-------------------------------------");
  delay(2000);

  // Initialisation MQTT
  Serial.println("Connection MQTT start...");
  Serial.println("-------------------------------------");
  reconnectMqtt();
  digitalWrite(ledPinGreen, HIGH);
  Serial.println("-------------------------------------");
  delay(2000);

  digitalWrite(ledPinRed, LOW);
  digitalWrite(ledPinBlue, LOW);
  digitalWrite(ledPinGreen, LOW);

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
  digitalWrite(ledPinRed, HIGH);
  delay(500);
  digitalWrite(ledPinGreen, HIGH);
  delay(500);
  digitalWrite(ledPinGreen, LOW);
  delay(500);
  digitalWrite(ledPinRed, LOW);
  delay(500);
  digitalWrite(ledPinBlue, LOW);
}

void humMore(float value) {
  float maxHum = (float) 80;
  if (value > maxHum) {
    digitalWrite(ledPinBlue, HIGH);
  } else {
    digitalWrite(ledPinBlue, LOW);
  }
}

void tempMore(float value) {
  float maxTemp = (float) 25;
  if (value > maxTemp) {
    digitalWrite(ledPinRed, HIGH);
  } else {
    digitalWrite(ledPinRed, LOW);
  }
}

void reconnectMqtt() {
  MQTT_CLIENT.setServer(mqtt_server, 1883);
  MQTT_CLIENT.setClient(client);
  String msg = "";
  while (!MQTT_CLIENT.connected()) {
    MQTT_CLIENT.connect("cuisine", "flo", "F07101991BEME");
    msg += ".";
    if (msg.length() > 36) {
      msg = ".";
      Serial.println(msg);
    } else {
      Serial.print(msg);
    }
    digitalWrite(ledPinGreen, HIGH);
    delay(1000);
    digitalWrite(ledPinGreen, LOW);
    delay(1000);
  }
  Serial.println("MQTT connected");
}

void reconnectWifi() {
  Serial.println("WiFi Disconnected-> Connexion en cours");
  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(ledPinRed, HIGH);
    delay(1000);
    digitalWrite(ledPinRed, LOW);
    delay(1000);
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
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
      Serial.println(hum);
      Serial.println("-----------");

      Serial.print("température: ");
      Serial.println(temp);
      Serial.println("-----------");

      // conversion float->char
      char tempString[8];
      char humString[8];
      dtostrf(temp, 1, 2, tempString);
      dtostrf(hum, 1, 2, humString);

      String nameTopic = "espSensor/" ;

      String tempTopicStr = nameTopic  + "cuisine/temp";
      String humTopicStr = nameTopic + "cuisine/hum";

      char tempTopicPub[50];
      tempTopicStr.toCharArray(tempTopicPub, 50);
      char humTopicPub[50];
      humTopicStr.toCharArray(humTopicPub, 50);

      MQTT_CLIENT.publish(tempTopicPub, tempString);
      MQTT_CLIENT.publish(humTopicPub, humString);

      delay(100);
      humMore(hum);
      delay(100);
      tempMore(temp);
      delay(100);

      digitalWrite(ledPinGreen, HIGH);
      delay(200);
      digitalWrite(ledPinGreen, LOW);
      delay(200);
    } else {
      reconnectWifi();
    }

    lastTime = millis();
  }
}
