#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

const char *ssid = "Spidy";
const char *password = "skwgpcgr";
const char *mqtt_server = "192.168.157.182";
const char *mqtt_username = "aquaServer";
const char *mqtt_password = "raspberry";

WiFiClient espClient;
PubSubClient client(espClient);

// Define the software serial pins for communication with Arduino
SoftwareSerial swSerial(D1, D2); // Replace D2 and D3 with your desired pins
//SoftwareSerial sw1Serial(D3, D4);

void setup_wifi() {
  delay(50);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length) {
  String actuatorMessage = "";
  for (int i = 0; i < length; i++) {
    actuatorMessage += (char)payload[i];
  }
  processActuatorMessage(actuatorMessage);
}

void connect_mqttServer() {
  while (!client.connected()) {
    if (WiFi.status() != WL_CONNECTED) {
      setup_wifi();
    }

    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32_client2", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Subscribe to the actuators topic
      client.subscribe("actuators");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 2 seconds");

      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); // Set the callback function for handling messages

  // Initialize software serial for communication with Arduino
  swSerial.begin(9600);
  //sw1Serial.begin(19200);

  connect_mqttServer();
}

void loop() {
  if (!client.connected()) {
    connect_mqttServer();
  }

  client.loop();

  // Read data from Arduino via Software Serial
  while (swSerial.available() > 0) {
    String receivedData = swSerial.readStringUntil('\n');
    Serial.println(receivedData); // Print received data for debugging

    // Process the received data (replace spaces and newlines)
    receivedData.replace(":", ": ");
    receivedData.replace("  ", " ");
    receivedData.trim(); // Remove leading and trailing spaces

    // Publish the processed data to MQTT
    client.publish("sensor_data", receivedData.c_str());
  }
}

// Process the actuator message and send data to Arduino
void processActuatorMessage(String message) {
  if (message.length() > 0) {
    Serial.println(message);

    // Extract sensor data from the message
    if (message.indexOf("Filter:") != -1 && message.indexOf("Oxygen:") != -1) {
      // Send data to Arduino via Software Serial
      swSerial.println(message);
    }

    delay(1000); // Add a delay to allow Arduino to process the message
  }
}


