code_demo.cOpen
Trieu Le Thanh 
(lethanhtrieuk36@gmail.com)

// KHAI BAO THU VIEN ****************************************************
#include <SPI.h>
#include <Ethernet.h>

// KHOI TAO CAC DOI TUONG
	//Ethernet
	byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x19 };   
	boolean DEBUG = true;//Debug Mode
	char serverName[] = "lethanhtrieu.likesyou.org";
	boolean lastConnected = false;
	EthernetClient client;

// KHOI TAO CAC BIEN ****************************************************

String uplen;
int sensor0 = A0;
int sensor1 = A1;
int sensor2 = A2;   

// SETUP             ****************************************************
void setup()
{
	Serial.begin(9600);
	
	/*
	if (Ethernet.begin(mac) == 0)
	{
		Serial.println("Failed to configure Ethernet using DHCP");
		// no point in carrying on, so do nothing forevermore:
		digitalWrite(LED_BAO_LOI,HIGH);
		while(true);
	}
	 else
	{
		Serial.println("Ethernet ready");
		// print the Ethernet board/shield's IP address:
		Serial.print("My IP address: ");
		Serial.println(Ethernet.localIP());
	}
	*/
}

// VONG LAP          ****************************************************
void loop()
{
	delay(1000);
	uplen=laydulieu();
	up2Drive(uplen);
	
	
}

String laydulieu()
{
	String temp;
	temp +="data1=";
	temp += analogRead(sensor0);
	temp += "&data2=";
	temp += analogRead(sensor1);
	temp += "&data3=";
	temp += analogRead(sensor2);
	return temp;
}




void up2Drive(String dulieu_uplen)
{
  client.stop();
  if(DEBUG){Serial.println("connecting...");}

  if (client.connect(serverName, 80))
  {
    if(DEBUG){Serial.println("connected");}

    if(DEBUG){Serial.println("sendind request");}

	dulieu_uplen +="submit=Submit HTTP/1.1";
    if(DEBUG){ Serial.println(dulieu_uplen);}

	client.print("POST /upload.php?");
    client.println(dulieu_uplen);
    client.println("Host: lethanhtrieu.likesyou.org");
    client.println("User-Agent: Arduino");
	client.println( "Content-Type: application/x-www-form-urlencoded" );
    client.println( "Connection: close" );
    client.print( "Content-Length: " );
    client.println( dulieu_uplen.length() );
    client.println();
    client.print( dulieu_uplen );
    client.println();
  }
  else
  {
    if(DEBUG){Serial.println("connection failed");}
  }
}
Displaying code_demo.c.
