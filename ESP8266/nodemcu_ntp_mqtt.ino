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
#include <EEPROM.h>
#include <SimpleTimer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
//-------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------
#define PIN_STATUS_LED                 2
#define PIN_MODE_BUTTON                5  
#define DEVICE_SERIAL_LEN              15
#define LED_ON() digitalWrite(PIN_STATUS_LED, HIGH)
#define LED_OFF() digitalWrite(PIN_STATUS_LED, LOW)
#define LED_TOGGLE() digitalWrite(PIN_STATUS_LED, digitalRead(PIN_STATUS_LED) ^ 0x01)
#define LED_COUNT 4
#define LED_PIN   5
// MQTT Variables
#define MQTT_PAYLOAD_LEN               40
#define MQTT_PUBLISH_BUFF_LEN          100

// EEPROM OFFSET
#define OFFSET_DEVICE_IS_REGISTERED    0  // Offset of device registertration
#define OFFSET_DEVICE_CURRENT_STATUS   1  // Offset of device status
#define OFFSET_DEVICE_CURRENT_MODE     2  // Offset of device mode
#define OFFSET_DEVICE_RED_VALUE        3  // Offset of RED Value
#define OFFSET_DEVICE_GREEN_VALUE      4  // Offset of GREEN Value
#define OFFSET_DEVICE_BLUE_VALUE       5  // Offset of BLUE Value
#define OFFSET_DEVICE_AUTO_ON_MODE     6  // Offset of AUTO ON mode
#define OFFSET_DEVICE_AUTO_ON_HOUR     7  // Offset of AUTO ON hour
#define OFFSET_DEVICE_AUTO_ON_MIN      8  // Offset of AUTO ON minute
#define OFFSET_DEVICE_AUTO_OFF_MODE    9  // Offset of AUTO OFF mode
#define OFFSET_DEVICE_AUTO_OFF_HOUR    10 // Offset of AUTO OFF hour
#define OFFSET_DEVICE_AUTO_OFF_MIN     11 // Offset of AUTO OFF minute
#define OFFSET_DEVICE_EFFECT_MODE      12 // Offset of effects mode
//-------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------
// the timer object
SimpleTimer timer;
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
char msg[150];
byte temperature = 0;
byte humidity = 0;
char response_message[MQTT_PUBLISH_BUFF_LEN];
int value = 0;
char mqtt_publish_buffer[MQTT_PUBLISH_BUFF_LEN];
// Device variables  
bool device_is_registered;            // Co bao thiet bi da duoc dang ky
byte device_current_status;           // Trang thai hien tai: ON/OFF
byte device_current_mode;             // Che do hien tai: White/RGB/Random
uint8_t device_red_value;                // RED Value: 0 - 255
uint8_t device_green_value;              // GREEN Value: 0 - 255
uint8_t device_blue_value;               // BLUE Value: 0 - 255
bool device_auto_on_mode;             // Device auto ON mode
uint8_t device_auto_on_hour;             // Device auto ON hour
uint8_t device_auto_on_minute;           // Device auto ON minute
bool device_auto_off_mode;            // Device auto OFF mode
uint8_t device_auto_off_hour;            // Device auto OFF hour
uint8_t device_auto_off_minute;          // Device auto OFF minute
uint8_t device_color_effect_mode;        // Device color effect mode
DynamicJsonBuffer jsonBuffer(100);
char jsonString[200];
WiFiManager wifiManager;

// Declare UDP Object
static const char ntpServerName[] = "time.nist.gov";  		// NTP Server name
static const int 	timeZone = 7; 													// Time zone: GMT+7
WiFiUDP Udp;
uint16_t localPort; 
tmElements_t tm,last_min; 
bool got_ntp_time_ok = false;
IPAddress timeServerIP;						 	// time.nist.gov NTP server address
		
//-------------------------------------------------------------------------
// Function prototype
//-------------------------------------------------------------------------
void sendNTPpacket(IPAddress &address);
time_t getNtpTime();
/*-------- NTP code ----------*/
const int NTP_PACKET_SIZE = 48; 			// NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; 	// buffer to hold incoming & outgoing packets
time_t getNtpTime()
{
	while (Udp.parsePacket() > 0) ; 		// discard any previously received packets
	Serial.print(("Transmit NTP Request "));
	//get a random server from the pool
	WiFi.hostByName(ntpServerName, timeServerIP);
	Serial.println(timeServerIP);

	sendNTPpacket(timeServerIP);
	uint32_t beginWait = millis();
	while((millis() - beginWait) < 2000) 
	{
		int size = Udp.parsePacket();
		if (size >= NTP_PACKET_SIZE) {
			Serial.println(("Receive NTP Response"));
			Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			got_ntp_time_ok = true;
			return secsSince1900 - 2208988800UL + (timeZone * SECS_PER_HOUR);
		}
	}
	Serial.println(("No NTP Response :-("));
	setSyncProvider(getNtpTime);
	return 0; // return 0 if unable to get the time
}
// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     				// Stratum, or type of clock
	packetBuffer[2] = 6;     				// Polling Interval
	packetBuffer[3] = 0xEC;  				// Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;
	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	Udp.beginPacket(address, 123); //NTP requests are to port 123
	Udp.write(packetBuffer, NTP_PACKET_SIZE);
	Udp.endPacket();
}
//-------------------------------------------------------------------------
// Setup function
//-------------------------------------------------------------------------
void setup() 
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_MODE_BUTTON, INPUT);
  ticker.attach(1, blink_status_led);
  client.setServer(mqtt_server, 1883);
  client.setCallback(MQTT_Callback);
  Serial.println("\r\nConfig device success !");

  snprintf (msg, 150, "CPU Frequency: %d",ESP.getCpuFreqMHz());
  Serial.println(msg);
  // Read data from EEPROM
  EEPROM.begin(512);
  //reset saved settings
  //wifiManager.resetSettings();
	
}
//-------------------------------------------------------------------------
// Main program
//-------------------------------------------------------------------------
void loop() 
{
  if (mode_button_press()) 
  {
  Serial.printf("Wi-Fi mode set to WIFI_STA %s\n", WiFi.mode(WIFI_STA) ? "" : "Failed!");
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
    if (now - lastMsg > 60000) 
    {
		lastMsg = now;
		++value;
		
		String time;
		time.concat(day()); time.concat("-"); time.concat(month()); time.concat("-"); time.concat(year());time.concat(" ");
		time.concat(hour()); time.concat(":"); time.concat(minute()); time.concat(":"); time.concat(second()); 
		
		Serial.println(time);
		if(year() < 2000)
		{
			Serial.println(year());
			setSyncProvider(getNtpTime);
		}
		else
		{
			//snprintf (msg, 150, "Published message: #%ld, Temperature: %d, Humidity: %d", value,(int)temperature,(int)humidity);
			//snprintf (msg, 150, "{\"message_id\": %d, \"temperature\": %d ,\"humidity\": %d}",value,(int)temperature,(int)humidity);
			char jsonString[200];
			JsonObject& root = jsonBuffer.createObject();
			root["message_id"] = value;
			root["created_time"] = time;
			root["temperature"] = (int)temperature;
			root["humidity"] = (int)humidity;
			root.printTo(jsonString);

			Serial.print("Publish message: ");
			Serial.println(jsonString);
			client.publish(device_serial, jsonString);
		}
    }
  }
//  else
//  {
//  Serial.print("Current WIFI Status: ");  
//  Serial.println(WiFi.status()); 
//  }
  // Timer running
  timer.run();
  
  if(got_ntp_time_ok)
  { 
	got_ntp_time_ok = false;
	now(); // store the current time in time variable t
	Serial.println("=> Set time for system !"); 
  }
  
  /*
	if(!got_ntp_time_ok)
	{
		static time_t prevDisplay = 0; 		// when the digital clock was displayed
		
		timeStatus_t ts = timeStatus();
		switch (ts) {
		case timeNeedsSync:
		case timeSet:
			if(now() != prevDisplay) 				// update the display only if time has changed
			{ 
				prevDisplay = now();
				//digitalClockDisplay();
				if (ts == timeNeedsSync) 
				{
					Serial.println(("time needs sync"));
				}
			}
			break;
		case timeNotSet:
			//Serial.println(("Time not set"));
			//now();
			//ESP.reset();
			break;
		default:
			break;
		}
	}
	*/
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
  Serial.print("device_current_mode [");
  Serial.print(device_current_mode);
  Serial.print("]\r\n");
  // Turn Device ON
  //digitalWrite(PIN_RELAY, HIGH);
}
//-------------------------------------------------------------------------
// Turn OFF Device
//-------------------------------------------------------------------------
void turn_off_device(void)
{
  Serial.print("device_current_mode [");
  Serial.print(device_current_mode);
  Serial.print("]\r\n");
  // Turn Device OFF
  //digitalWrite(PIN_RELAY, LOW);
}
//-------------------------------------------------------------------------
// Change color of device
//-------------------------------------------------------------------------
void change_color_of_device(uint8_t red, uint8_t green, uint8_t blue)
{
  
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
// Read variables from EEPROM
//-------------------------------------------------------------------------
void read_data_from_eeprom(void)
{
  device_is_registered = EEPROM.read(OFFSET_DEVICE_IS_REGISTERED);
  device_current_status = EEPROM.read(OFFSET_DEVICE_CURRENT_STATUS);
  device_current_mode = EEPROM.read(OFFSET_DEVICE_CURRENT_MODE);
  device_red_value = EEPROM.read(OFFSET_DEVICE_RED_VALUE);
  device_green_value = EEPROM.read(OFFSET_DEVICE_GREEN_VALUE);
  device_blue_value = EEPROM.read(OFFSET_DEVICE_BLUE_VALUE);
  device_auto_on_mode = EEPROM.read(OFFSET_DEVICE_AUTO_ON_MODE);
  device_auto_on_hour = EEPROM.read(OFFSET_DEVICE_AUTO_ON_HOUR);
  device_auto_on_minute = EEPROM.read(OFFSET_DEVICE_AUTO_ON_MIN);
  device_auto_off_mode = EEPROM.read(OFFSET_DEVICE_AUTO_OFF_MODE);
  device_auto_off_hour = EEPROM.read(OFFSET_DEVICE_AUTO_OFF_HOUR);
  device_auto_off_minute = EEPROM.read(OFFSET_DEVICE_AUTO_OFF_MIN);
  device_color_effect_mode = EEPROM.read(OFFSET_DEVICE_EFFECT_MODE);
}
//-------------------------------------------------------------------------
// Write variables to EEPROM
//-------------------------------------------------------------------------
void write_data_to_eeprom(void)
{
  EEPROM.write(OFFSET_DEVICE_IS_REGISTERED, device_is_registered);
  EEPROM.write(OFFSET_DEVICE_CURRENT_STATUS, device_current_status);
  EEPROM.write(OFFSET_DEVICE_CURRENT_MODE, device_current_mode);
  EEPROM.write(OFFSET_DEVICE_RED_VALUE, device_red_value);
  EEPROM.write(OFFSET_DEVICE_GREEN_VALUE, device_green_value);
  EEPROM.write(OFFSET_DEVICE_BLUE_VALUE, device_blue_value);
  EEPROM.write(OFFSET_DEVICE_AUTO_ON_MODE, device_auto_on_mode);
  EEPROM.write(OFFSET_DEVICE_AUTO_ON_HOUR, device_auto_on_hour);
  EEPROM.write(OFFSET_DEVICE_AUTO_ON_MIN, device_auto_on_minute);
  EEPROM.write(OFFSET_DEVICE_AUTO_OFF_MODE, device_auto_off_mode);
  EEPROM.write(OFFSET_DEVICE_AUTO_OFF_HOUR, device_auto_off_hour);
  EEPROM.write(OFFSET_DEVICE_AUTO_OFF_MIN, device_auto_off_minute);
  EEPROM.write(OFFSET_DEVICE_EFFECT_MODE, device_color_effect_mode);
  EEPROM.commit();
}
//-------------------------------------------------------------------------
// MQTT Callback function
//-------------------------------------------------------------------------
void MQTT_Callback(char* topic, uint8_t* payload, unsigned int payload_length) {
  char tx_topic[DEVICE_SERIAL_LEN + 2];
  tx_topic[0] = 'W';
  tx_topic[1] = '/';
  memcpy(&tx_topic[2],device_serial,DEVICE_SERIAL_LEN);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < payload_length; i++) 
  {
    Serial.print(payload[i],HEX);
  }
  Serial.println();
  Serial.print("Message Length = ");
  Serial.print(payload_length);
  Serial.println();

  // Compared topic with serial of device
  if(memcmp(topic, device_serial, DEVICE_SERIAL_LEN) == 0)
  {
    if(payload_length == MQTT_PAYLOAD_LEN)
    {
    if(memcmp(&payload[3], device_serial, DEVICE_SERIAL_LEN) == 0)
    {
      mqtt_payload_parser(payload,payload_length);             // Xu ly lenh 
      client.publish(tx_topic,(uint8_t*)mqtt_publish_buffer,MQTT_PAYLOAD_LEN);       // Phan hoi lenh
    }
    }
  }
}
//-------------------------------------------------------------------------
// Payload parser
//-------------------------------------------------------------------------
void mqtt_payload_parser(uint8_t *mqtt_payload, unsigned int payload_len)
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
        case 0x01: 
    break;
     case 0x02:                            // Lenh TURN ON
      Serial.println("TURN ON DEVICE");
          mqtt_publish_buffer[2] = 0x02;  
          turn_on_device();               // Bat thiet bi  
        break;
        case 0x03:                            // Lenh TURN OFF
          Serial.println("TURN OFF DEVICE");
      mqtt_publish_buffer[2] = 0x03;
          turn_off_device();                // Tat thiet bi
        break;
    };
  }
    else if(mqtt_payload[1] == 0x68)                 // Phan hoi Register
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
		client.publish(device_serial, "Smart Light Connected !");
		// ... and resubscribe
		client.subscribe(device_serial);
		got_ntp_time_ok = false; // Tat co nay de gui lenh dong bo thoi gian
		if(! got_ntp_time_ok)
		{
			WiFi.hostByName(ntpServerName, timeServerIP);
			// Seed random with values unique to this device
			uint8_t macAddr[6];
			WiFi.macAddress(macAddr);
			uint32_t seed1 = (macAddr[5] << 24) | (macAddr[4] << 16) |(macAddr[3] << 8)  | macAddr[2];
			randomSeed(WiFi.localIP() + seed1 + micros());
			localPort = random(1024, 65535);
			Serial.println(("Starting UDP"));
			Udp.begin(localPort);
			Serial.print(("Local port: "));
			Serial.println(Udp.localPort());
			Serial.println(("waiting for sync"));
			setSyncProvider(getNtpTime);
			setSyncInterval(1 * 60); // Sync every 1 min
		}
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
