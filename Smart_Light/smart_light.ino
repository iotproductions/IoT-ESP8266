//-------------------------------------------------------------------------
// Project name: Smart Sleep Light
// Create date: 08-April-2018
// Author: Trieu Le
// IDE: Arduino IDE v1.8.4
// Hardware: ESP8266 Witty Cloud
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// Include libraries
//-------------------------------------------------------------------------
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <time.h>
#include <PubSubClient.h>
//-------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------
#define PIN_LDR_SENSOR                 0
#define PIN_STATUS_LED                 3
#define PIN_MODE_BUTTON                4
#define PIN_LED_RED                    15
#define PIN_LED_GREEN                  12
#define PIN_LED_BLUE                   13
#define DEVICE_SERIAL_LEN              15
#define LED_ON() digitalWrite(PIN_STATUS_LED, HIGH)
#define LED_OFF() digitalWrite(PIN_STATUS_LED, LOW)
#define LED_TOGGLE() digitalWrite(PIN_STATUS_LED, digitalRead(PIN_STATUS_LED) ^ 0x01)
#define TOGGLE_LED_RED() digitalWrite(PIN_LED_RED, digitalRead(PIN_LED_RED) ^ 0x01)
//-------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------
Ticker ticker;
bool in_smartconfig = false;
// MQTT Config
//const char* mqtt_server = "iot.eclipse.org";
const char* mqtt_server = "lethanhtrieu.servehttp.com";
const char* mqtt_user_name = "trieu.le";
const char* mqtt_password = "ttp@2015";
const char* device_serial = "180318SL001N001";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
char response_message[100];
int value = 0;
//-------------------------------------------------------------------------
// Setup function
//-------------------------------------------------------------------------
void setup() 
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_MODE_BUTTON, INPUT);
  pinMode(PIN_LDR_SENSOR, INPUT);
  ticker.attach(1, blink_status_led);
  client.setServer(mqtt_server, 1883);
  client.setCallback(MQTT_Callback);
  Serial.println("\r\nConfig device success !");
}
//-------------------------------------------------------------------------
// Main program
//-------------------------------------------------------------------------
void loop() 
{
  if (mode_button_press()) 
  {
    enter_smart_config();
    Serial.println("Enter smartconfig");
  }
  if (WiFi.status() == WL_CONNECTED && in_smartconfig && WiFi.smartConfigDone()) 
  {
    exit_smart_config();
    Serial.println("Connected, Exit smartconfig");
  }
  // Connected to AP
  if (WiFi.status() == WL_CONNECTED) 
  {
  if (!client.connected()) 
  {
    MQTT_Reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 2000) 
  {
    lastMsg = now;
    ++value;
    snprintf (msg, 50, "Published message: #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(device_serial, msg);
  }
  }
}
//-------------------------------------------------------------------------
// Enter Smart Config 
//-------------------------------------------------------------------------
void enter_smart_config(void)
{
  if (in_smartconfig == false) 
  {
    in_smartconfig = true;
    ticker.attach(0.1, blink_status_led);
    WiFi.beginSmartConfig();
  }
}
//-------------------------------------------------------------------------
// Exit Smart Config 
//-------------------------------------------------------------------------
void exit_smart_config(void)
{
  ticker.detach();
  LED_ON();
  in_smartconfig = false;
}
//-------------------------------------------------------------------------
// Check mode button 
//-------------------------------------------------------------------------
bool mode_button_press()
{
  static int lastPress = 0;
  if (millis() - lastPress > 3000 && digitalRead(PIN_MODE_BUTTON) == 0) 
  {
    return true;
  } 
  else if (digitalRead(PIN_MODE_BUTTON) == 1) 
  {
    lastPress = millis();
  }
  return false;
}
//-------------------------------------------------------------------------
// Check mode button 
//-------------------------------------------------------------------------
void blink_status_led()
{
  //toggle state
  int state = digitalRead(PIN_STATUS_LED);  // get the current state of GPIO1 pin
  digitalWrite(PIN_STATUS_LED, !state);     // set pin to the opposite state
}
//-------------------------------------------------------------------------
// MQTT Callback function
//-------------------------------------------------------------------------
void MQTT_Callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  // Compared topic with serial of device
  if(memcmp(topic, device_serial, DEVICE_SERIAL_LEN) == 0)
  {
	TOGGLE_LED_RED();
	if(memcmp(payload, "GET STATUS", 10) == 0)
	{	
		client.publish(device_serial,"OK:GET STATUS");
	}
	else if(memcmp(payload, "GET ADC", 7) == 0)
	{	
		memset(response_message,0x00,100);
		snprintf(response_message,100,"OK:ADC|%ld",analogRead(PIN_LDR_SENSOR));
		client.publish(device_serial,response_message);
	}
  }
}
//-------------------------------------------------------------------------
// MQTT Check connection
//-------------------------------------------------------------------------
void MQTT_Reconnect()
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = device_serial;
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqtt_user_name,mqtt_password)) 
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(device_serial);
    } 
	else 
	{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//-------------------------------------------------------------------------
// End of file
//-------------------------------------------------------------------------
