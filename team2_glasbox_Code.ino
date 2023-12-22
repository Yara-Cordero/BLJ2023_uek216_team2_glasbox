#include <WiFi.h>
#include <Adafruit_AHTX0.h>
#include <PubSubClient.h>

//konstanten für pins
const int yellowLED = 33;
const int greenLED = 25;
const int redLED = 32;

const char* device_id = "DEFINE_DEVICE_ID_HERE";
const char* ssid = "GuestWLANPortal";
const char* mqtt_server = "noseryoung.ddns.net";

const char* topicTemp = "zuerich/glasbox/temperature/celsius";
const char* topicHum = "zuerich/glasbox/temperature/humidity";
const char* topicMax = "zuerich/glasbox/temperature/max";
const char* topicMin = "zuerich/glasbox/temperature/min";

float Maxtemp = 27;
float Mintemp = 21;

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(115200);
  setup_aht();
  setup_wifi();
  client.setServer(mqtt_server, 1983);
  client.setCallback(callback);

  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
}

void setup_aht() {
  Serial.print("Searching AHT10 / AHT20...");
  while(!aht.begin()) {
    delay(500);
    Serial.print(".");
 }
 Serial.println("done!");
}
//wifi Verbindung herstellen
void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 Serial.println("done!");
}

//Daten aus Node-RED empfangen
void callback(char* topic, byte* payload, unsigned int length) {
  
  if(strcmp(topic, topicMax) == 0) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    Serial.print("Received Message: ");
    Serial.println(message);
    Maxtemp = atof(message);

  }else if(strcmp(topic, topicMin) == 0) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    Serial.print("Received Message: ");
    Serial.println(message);
    Mintemp = atof(message);

  }
}

//Wiederherstellung der MQTT verbindung
void reconnect() {
  Serial.print("Attempting MQTT connection...");
  while(!client.connected()) {
    if(client.connect(device_id)) {
      Serial.println("done!");
      client.subscribe(topicTemp);
      client.subscribe(topicHum);
      client.subscribe(topicMin);
      client.subscribe(topicMax);
    } else {
      delay(500);
      Serial.print(".");
    }
  }
}

void loop() {
  if(!client.connected()) {
    reconnect();
  }

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  char tempBuffer[10];
  char humBuffer[10];
  float MaxMinDif = Maxtemp - Mintemp;
 
 /*
  Serial.println(temp.temperature);
  Serial.println(humidity.relative_humidity);
  Serial.println(Maxtemp);
  Serial.println(Mintemp);
  Serial.print("Max Min difference: ");
  Serial.println(MaxMinDif);
*/
  sprintf(tempBuffer, "%f", temp.temperature);
  sprintf(humBuffer, "%f", humidity.relative_humidity);

  //Temperatur und Luftfeuchtigkeit unter dem richtigen Topic veröffentlichen
  client.publish(topicTemp, tempBuffer);
  client.publish(topicHum, humBuffer);
 
  if(temp.temperature >= Mintemp && temp.temperature <= Maxtemp) {
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
  } else if (temp.temperature <= (Mintemp - MaxMinDif) || (temp.temperature >= (Maxtemp + MaxMinDif))) {
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);  
  } else {
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, LOW);
    digitalWrite(yellowLED, HIGH);
  }
  
  delay(100);
  client.loop();
}



