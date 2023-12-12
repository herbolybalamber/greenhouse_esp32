#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

String client_id = "sensorbox_696969";

const int lightSensorPin = 34;
int lightSensorValue = 0;
const int moistureSensorPin = 33;
int moistureSensorValue = 0;
const int ledPin = 27;


// WiFi
const char *ssid = "wifi-ssid"; // Enter your Wi-Fi name
const char *wifiPassword = "wifi-password";  // Enter Wi-Fi password



const char *mqtt_broker = "mqtt-server-adress";
const char *topicPublish = "topic-to-publish-in";
const char *topicSubscribe = "topic-to-subsribe-to";
const char *mqtt_username = "mqtt-server-username";
const char *mqtt_password = "mqtt-server-password-";
const int mqtt_port = 8883;
WiFiClientSecure espClient;
PubSubClient client(espClient);
const char *ca_cert = "-----BEGIN CERTIFICATE-----\n"
 
                      "-----END CERTIFICATE-----\n";

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_mqtt() {
  espClient.setCACert(ca_cert);
  client.setServer(mqtt_broker, 8883);
  client.setCallback(callback);
}


void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    // Connecting to a WiFi network
    setup_wifi();
    setup_mqtt();
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.println(client.state());
            delay(2000);
        }
    }
    // Publish and subscribe
    client.publish(topicPublish, "Hi, I'm ESP32 ^^");
    client.subscribe(topicSubscribe);

pinMode(ledPin, OUTPUT);
}

void callback(char *topicSubscribe, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topicSubscribe);
  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 // Convert the payload to a string
  String message = String((char*)payload);

  // Check if the payload is exactly "lights_on"
  if (message.equals("lights_on")) {
    // Turn on the LED connected to pin D25
    digitalWrite(ledPin, HIGH);
    Serial.println("Lights are on");
  }
  else if (message.equals("lights_off")){
    digitalWrite(ledPin, LOW);
    Serial.println("Lights are off");
  }

  Serial.println();
  Serial.println("-----------------------");
}



void publishSensorData() {
  // Read data from pin 34
  lightSensorValue = analogRead(lightSensorPin);
  moistureSensorValue = analogRead(moistureSensorPin);

  // Create JSON string with both light and moisture values
  String data = "{\"light\": " + String(lightSensorValue) + ", \"moisture\": " + String(moistureSensorValue) + "}";

  // Publish the data to the MQTT broker
  client.publish(topicPublish, data.c_str());
}



void loop() {
  client.loop();

  // Publish sensor data every 5 seconds
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis >= 5000) {
    publishSensorData();
    lastMillis = millis();
  }
}
