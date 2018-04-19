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
#include <stdint.h>
#include <PubSubClient.h>
#include <RGBLED.h>
//-------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------
#define PIN_LDR_SENSOR                 0
#define PIN_STATUS_LED                 2
#define PIN_MODE_BUTTON                4
#define PIN_LED_RED                    15
#define PIN_LED_GREEN                  12
#define PIN_LED_BLUE                   13
#define DEVICE_SERIAL_LEN              15
#define LED_ON() digitalWrite(PIN_STATUS_LED, HIGH)
#define LED_OFF() digitalWrite(PIN_STATUS_LED, LOW)
#define LED_TOGGLE() digitalWrite(PIN_STATUS_LED, digitalRead(PIN_STATUS_LED) ^ 0x01)
#define TOGGLE_LED_RED() digitalWrite(PIN_LED_RED, digitalRead(PIN_LED_RED) ^ 0x01)

#define MQTT_PAYLOAD_LEN               40
#define MQTT_PUBLISH_BUFF_LEN          100
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
char response_message[MQTT_PUBLISH_BUFF_LEN];
int value = 0;
char mqtt_publish_buffer[MQTT_PUBLISH_BUFF_LEN];
RGBLED RGB_Light(PIN_LED_RED,PIN_LED_GREEN,PIN_LED_BLUE,COMMON_CATHODE);    
// Device variables  
bool device_is_registered;            // Co bao thiet bi da duoc dang ky
byte device_current_status;           // Trang thai hien tai: ON/OFF
byte device_current_mode;             // Che do hien tai: White/RGB/Random
byte device_red_value;                // RED Value: 0 - 255
byte device_green_value;              // GREEN Value: 0 - 255
byte device_blue_value;               // BLUE Value: 0 - 255
bool device_auto_on_mode;             // Device auto ON mode
byte device_auto_on_hour;             // Device auto ON hour
byte device_auto_on_minute;           // Device auto ON minute
bool device_auto_off_mode;            // Device auto OFF mode
byte device_auto_off_hour;            // Device auto OFF hour
byte device_auto_off_minute;          // Device auto OFF minute

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
	//Report the LED type and pins in use to the serial port...
  Serial.println("Welcome to the RGBLED Sample Sketch");
  String ledType = (RGB_Light.commonType==0) ? "COMMON_CATHODE" : "COMMON_ANODE";
  Serial.println("Your RGBLED instancse is a " + ledType + " LED");
  Serial.println("And the Red, Green, and Blue legs of the LEDs are connected to pins:");
  Serial.println("r,g,b = " + String(RGB_Light.redPin) + "," + String(RGB_Light.greenPin) + "," + String(RGB_Light.bluePin) );
  Serial.println("");
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
  else
  {
	Serial.print("Current WIFI Status: ");  
	Serial.println(WiFi.status()); 
	TOGGLE_LED_RED();
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
void blink_status_led(void)
{
  //toggle state
  int state = digitalRead(PIN_STATUS_LED);  // get the current state of GPIO1 pin
  digitalWrite(PIN_STATUS_LED, !state);     // set pin to the opposite state
}
//-------------------------------------------------------------------------
// Turn ON Device
//-------------------------------------------------------------------------
void turn_on_device(void)
{
  // Turn Device ON
}
//-------------------------------------------------------------------------
// Turn OFF Device
//-------------------------------------------------------------------------
void turn_off_device(void)
{
  // Turn Device OFF
}
//-------------------------------------------------------------------------
// Change color of device
//-------------------------------------------------------------------------
void change_color_of_device(byte red, byte green, byte blue)
{
  // Change device's color
}
//-------------------------------------------------------------------------
// Set auto ON
//-------------------------------------------------------------------------
void set_device_auto_on(bool auto_mode, byte auto_hour, byte auto_minute)
{
  // Set device auto turn on
}
//-------------------------------------------------------------------------
// Set auto OFF
//-------------------------------------------------------------------------
void set_device_auto_off(bool auto_mode, byte auto_hour, byte auto_minute)
{
  // Set device auto turn off
}
//-------------------------------------------------------------------------
// MQTT Callback function
//-------------------------------------------------------------------------
void MQTT_Callback(char* topic, byte* payload, unsigned int payload_length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < payload_length; i++) 
  {
	Serial.print((char)payload[i]);
  }
  Serial.println();
  // Compared topic with serial of device
  if(memcmp(topic, device_serial, DEVICE_SERIAL_LEN) == 0)
  {
	if(payload_length == MQTT_PAYLOAD_LEN)
	{
		mqtt_payload_parser(payload,payload_length);             // Xu ly lenh 
		client.publish(device_serial,mqtt_publish_buffer);       // Phan hoi lenh
	}
  }
}
//-------------------------------------------------------------------------
// Payload parser
//-------------------------------------------------------------------------
void mqtt_payload_parser(byte *mqtt_payload, unsigned int payload_len)
{
	if(mqtt_payload[0] == '>')                        // Nhan lenh tu server
	{
		if(mqtt_payload[1] == 0x67)                   // Khung lenh 0x67
		{
            memset(mqtt_publish_buffer,0x00,MQTT_PUBLISH_BUFF_LEN); // Clear publish buffer
            mqtt_publish_buffer[0] = '<';
			mqtt_publish_buffer[1] = 0x67;
			memcpy(&mqtt_publish_buffer[3],&device_serial[0],DEVICE_SERIAL_LEN);
			switch(mqtt_payload[2])                   // Xu ly lenh
			{
				case 0x01:                            // Lenh GET STATUS
					mqtt_publish_buffer[2] = 0x01;
					mqtt_publish_buffer[18] = device_current_status;
					mqtt_publish_buffer[19] = device_current_mode;
					mqtt_publish_buffer[20] = device_red_value;
					mqtt_publish_buffer[21] = device_green_value;
					mqtt_publish_buffer[22] = device_blue_value;
					mqtt_publish_buffer[23] = device_auto_on_mode;
					mqtt_publish_buffer[24] = device_auto_on_hour;
					mqtt_publish_buffer[25] = device_auto_on_minute;
					mqtt_publish_buffer[26] = device_auto_off_mode;
					mqtt_publish_buffer[27] = device_auto_off_hour;
					mqtt_publish_buffer[28] = device_auto_off_minute;
				break;
				case 0x02:                            // Lenh TURN ON
					mqtt_publish_buffer[2] = 0x02;  
					turn_on_device();	              // Bat thiet bi  
				break;
				case 0x03:                            // Lenh TURN OFF
					mqtt_publish_buffer[2] = 0x03;
					turn_off_device();                // Tat thiet bi
				break;
				case 0x04:                            // Lenh CHANGE COLOR
					mqtt_publish_buffer[2] = 0x04;
					device_red_value   = mqtt_payload[18];      // Luu RED value
					device_green_value = mqtt_payload[19];      // Luu GREEN value
					device_blue_value  = mqtt_payload[20];      // Luu BLUE value
					mqtt_publish_buffer[18] = device_red_value;
					mqtt_publish_buffer[19] = device_green_value;
					mqtt_publish_buffer[20] = device_blue_value;
					change_color_of_device(device_red_value,device_green_value,device_blue_value);
				break;
				case 0x05:                            // Lenh SET AUTO ON
					mqtt_publish_buffer[2] = 0x05;
					device_auto_on_mode    = mqtt_payload[18];  // Luu auto ON mode
					device_auto_on_hour    = mqtt_payload[19];  // Luu auto ON hour
					device_auto_on_minute  = mqtt_payload[20];  // Luu auto ON minute
					mqtt_publish_buffer[18] = device_auto_on_mode;
					mqtt_publish_buffer[19] = device_auto_on_hour;
					mqtt_publish_buffer[20] = device_auto_on_minute;
					set_device_auto_on(device_auto_on_mode,device_auto_on_hour,device_auto_on_minute);
				break;
				case 0x06:                            // Lenh SET AUTO OFF
					mqtt_publish_buffer[2] = 0x06;
					device_auto_off_mode    = mqtt_payload[18]; // Luu auto OFF mode
					device_auto_off_hour    = mqtt_payload[19]; // Luu auto OFF hour
					device_auto_off_minute  = mqtt_payload[20]; // Luu auto OFF minute
					mqtt_publish_buffer[18] = device_auto_off_mode;
					mqtt_publish_buffer[19] = device_auto_off_hour;
					mqtt_publish_buffer[20] = device_auto_off_minute;
					set_device_auto_off(device_auto_off_mode,device_auto_off_hour,device_auto_off_minute);
				break;
				case default:                         // Bo qua cac lenh khac
				break;
			};
		}
		else if(mqtt_payload[1] == 0x68)             // Phan hoi Register
		{
			if((mqtt_payload[2]) == 0x01)
			{
				if(memcmp(mqtt_payload, device_serial, DEVICE_SERIAL_LEN) == 0)
				{
					device_is_registered = true;    // Bat co bao thiet bi da duoc dang ky
				}
			}				
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
// Convert string of number to number
//-------------------------------------------------------------------------
uint32_t string2number(char *str_buff)
{
	uint16_t len = 0, i = 0, j = 0;
	uint32_t number = 0;
	uint32_t unit_10 = 0;
	len = strlen(str_buff);
	for (i = 0; i < len; i++)
	{
		unit_10 = 1;
		for (j = 0; j < i; j++)
		{
			unit_10 = unit_10*10;
		}
		number = number + ((str_buff[len - (i + 1)] - '0') * unit_10);
	}
	return number;
}
//-------------------------------------------------------------------------
// End of file
//-------------------------------------------------------------------------
