#include <Wire.h>
#include <stdio.h>
#include <ds3231.h>

#define UART_BUFF_LEN 20
String inString = "";    // string to hold input
char uart_receive_buff[UART_BUFF_LEN];
int uart_receice_count;
/*
struct ts {
    uint8_t sec;         // seconds 
    uint8_t min;         // minutes 
    uint8_t hour;        // hours 
    uint8_t mday;        // day of the month 
    uint8_t mon;         // month 
    int16_t year;        // year 
    uint8_t wday;        // day of the week 
    uint8_t yday;        // day in the year 
    uint8_t isdst;       // daylight saving time 
    uint8_t year_s;      // year in short notation
    uint32_t unixtime;   // seconds since 01.01.1970 00:00:00 UTC

};*/
struct ts rtc_setting;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // send an intro:
  Serial.println("\n\nString toInt():");
  Serial.println();
}

void loop() {
  // Read serial input:
  while (Serial.available() > 0) {
    int inChar = Serial.read();
    if(inChar == '\r')
    {
      Serial.println(uart_receive_buff);
      if((uart_receice_count == 8) && (uart_receive_buff[2] == ':') && (uart_receive_buff[5] == ':')) // received time setting cmd: 12:34:56
      {
        int time_hour =( atoi(&uart_receive_buff[0]));
        int time_min = (atoi(&uart_receive_buff[3]));
        int time_sec = (atoi(&uart_receive_buff[6]));
        
        rtc_setting.hour = time_hour;
        rtc_setting.min = time_min;
        rtc_setting.sec = time_sec;
        //DS3231_set(rtc_setting);
        Serial.print("\r\ntime_hour: ");
        Serial.print(time_hour);
        Serial.print(" time_min: ");
        Serial.print(time_min);
        Serial.print(" time_sec: ");
        Serial.print(time_sec);
        Serial.print(" SUM: ");
        Serial.print(time_sec + time_min);
      }
      uart_receice_count = 0; 
    }
    else
    {
      uart_receive_buff[uart_receice_count++] = inChar;
      if(uart_receice_count > UART_BUFF_LEN)
      {
        uart_receice_count = 0; 
      }
    }
  }
}
