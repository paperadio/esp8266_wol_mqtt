#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <WakeOnLan.h>

WiFiUDP UDP;
WakeOnLan WOL(UDP);

WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid     = "yourwifissid";
const char* password = "yourwifipassword";
const char *MACAddress = "AA:BB:CC:DD:EE:FF";

const char* mqtt_server = "broker.mqttdashboard.com";
const int mqtt_port = 1883;
const char* topic_pc     = "tomsworkers/pc";
const char* topic_controller     = "tomsworkers/controller";

void wakeMyPC() {
    WOL.sendMagicPacket(MACAddress); // Send Wake On Lan packet with the above MAC address.
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-007";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(topic_controller, "Hello, I am pc controller");
      // ... and resubscribe
      client.subscribe(topic_pc);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  char *cstring = (char *) payload;
  cstring[length] = '\0';    // Adds a terminate terminate to end of string based on length of current payload
  //Serial.println(cstring);

  // Switch on the LED if an 1 was received as first character
  if (strcmp(cstring, "ON") == 0)   
  {
    Serial.println("received:ON");
    wakeMyPC();
    client.publish(topic_controller, "received:ON");
  }
  else
  {
    client.publish(topic_controller, "I don't know this message.");
  }
}

void setup()
{
    Serial.begin(115200);

    //mqtt client
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
  
    WOL.setRepeat(3, 100); // Optional, repeat the packet three times with 100ms between. WARNING delay() is used between send packet function.
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("CONNECTED");

    WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask()); // Optional  => To calculate the broadcast address, otherwise 255.255.255.255 is used (which is denied in some networks).
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
