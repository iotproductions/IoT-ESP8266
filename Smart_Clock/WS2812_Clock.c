//----------------------------------------------------------------------------
// Project: RF Toolbox Receiver
// Released Date: 11-Mar-2017
// Hardware: Using Board SWM V.1.2 
// Firmware description:
//              +       edit timer 1s( from 250ms(8100)  => 500ms(16200) )every 500ms to send once
//                                          500ms(16200) => 1s(32400) every sec to send once
//              +       edit port 3 (case : 6403) and broadcast_ID
//              +       edit when magnet need CC1101 therefore P3.5 will on and otherwise
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Private includes
//----------------------------------------------------------------------------
#include <msp430fr5739.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "library\function.h"
#include "library\variable.h"
#include "library\cc1101.h"
#include "library\pins.h"
#include "library\encode_decode.h"
#include "library\XBee.h"
//----------------------------------------------------------------------------
// Global variables
//----------------------------------------------------------------------------
#define NUM_LEDS 126

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} LED;

uint16_t convertRGBData(uint8_t red, uint8_t green, uint8_t blue)
{
  uint16_t data;
  uint8_t red_t, green_t, blue_t;
  red_t = red/8;      // red   = 1111 1111 / 1000 = 0001 1111
  green_t = green/8;  // green = 0000 1111 / 1000 = 0000 0001
  blue_t = blue/8;    // blue  = 1111 0000 / 1000 = 0001 1110
  data = ((uint16_t)red_t << 5);    // red_t = 1 1111, (uint16_t)red_t = 0000 0000 0001 1111, ((uint16_t)red_t << 5) = 0111 1100 0000 0000
  data |= ((uint16_t)green_t << 5); //((uint16_t)green_t << 5) = 0000 0000 0010 0000
                                    //            data         = 0111 1100 0000 0000
                                    //             OR          = 0111 1100 0010 0000
  data |= ((uint16_t)blue_t);       //((uint16_t)green_t )     = 0000 0000 0001 1110
                                    //            data         = 0111 1100 0010 0000
                                    //      Final data (OR)    = 0111 1100 0011 1110   
  return data;
}

uint16_t convertRGBToData(LED RGB)
{
  uint16_t data;
  uint8_t red_t, green_t, blue_t;
  red_t = RGB.red/8;      // red   = 1111 1111 / 1000 = 0001 1111
  green_t = RGB.green/8;  // green = 0000 1111 / 1000 = 0000 0001
  blue_t = RGB.blue/8;    // blue  = 1111 0000 / 1000 = 0001 1110
  data = ((uint16_t)red_t << 10);    // red_t = 1 1111, (uint16_t)red_t = 0000 0000 0001 1111, ((uint16_t)red_t << 5) = 0111 1100 0000 0000
  data |= ((uint16_t)green_t << 5); //((uint16_t)green_t << 5) = 0000 0000 0010 0000
                                    //            data         = 0111 1100 0000 0000
                                    //             OR          = 0111 1100 0010 0000
  data |= ((uint16_t)blue_t);       //((uint16_t)green_t )     = 0000 0000 0001 1110
                                    //            data         = 0111 1100 0010 0000
                                    //      Final data (OR)    = 0111 1100 0011 1110   
  return data;
}
//------------------------------------------------------------------------------
// Get Blue value
//        data = 0111 1100 0011 1110
//        mask = 0000 0000 0001 1111
// data & mask = 0000 0000 0001 1110  
// Get Green value 
//        data = 0111 1100 0011 1110
//   data >> 5 = 0000 0011 1110 0001
//        mask = 0000 0000 0001 1111
// data & mask = 0000 0000 0000 0001 
// Get Red value
//        data = 0111 1100 0011 1110
//  data >> 10 = 0000 0000 0001 1111
//        mask = 0000 0000 0001 1111
// data & mask = 0000 0000 0001 1111
//------------------------------------------------------------------------------  
LED convertDataToRGB(uint16_t data) 
{
  uint16_t data_t; 
  data_t = data;
  LED RGB;
  RGB.blue   = ((uint8_t)(data_t & 0x1F))*8;
  RGB.green  = ((uint8_t)((data_t>>5) & 0x1F))*8;
  RGB.red    = ((uint8_t)((data_t>>10) & 0x1F))*8;
  return RGB;
}

static uint16_t LED_DATA[NUM_LEDS];
static LED leds[NUM_LEDS] = { { 0, 0, 0 } };
//static LED result_leds[NUM_LEDS] = { { 0, 0, 0 } };
// Transmit codes
#define HIGH_CODE   (0xF0)      // b11110000
#define LOW_CODE    (0xC0)      // b11000000
//							        0     1	    2 	  3     4     5     6     7     8     9
uint8_t font7seg[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

uint8_t RTC_BUFF[6] = {1,2,3,4,5,6};             // Time 12:34:56
void updateTime(void)
{
  for(int index = 0 ; index < 6; index++)
  {
    uint8_t start = (uint8_t)(index * 21);       // Vi tri led bat dau: start = 0
    uint8_t font = font7seg[RTC_BUFF[index]];    // font7seg[1] = 0x06 = 0000 1010
    for (uint8_t i = 1; i < 8; i ++)
    {
      bool value = (font & (0x80 >> i)) != 0;    // lay gia tri cua bit thu i trong byte font
      if (value)
      {
        for (int j = 0; j < 3; j++)	             // 3 led, moi led 3 byte
        {
          uint8_t red   = leds[start + i * 3 + j].red;
          uint8_t blue  = leds[start + i * 3 + j].blue;
          uint8_t green = leds[start + i * 3 + j].green;
          uint8_t temp_buff[3] = {red, blue, green};
          uint8_t mask = 0x80;                   // b1000000
          while (mask != 0)                      // check each of the 8 bits
          {
              while(!(UCA1IFG & UCTXIFG));       // wait to transmit
              if (temp_buff[j] & mask) {         // most significant bit first
                  UCA1TXBUF = HIGH_CODE;         // send 1
              } else {
                  UCA1TXBUF = LOW_CODE;          // send 0
              }
              mask >>= 1;                        // check next bit
          }
        }
      }
      else
      {
        for (int j = 0; j < 3; j ++)             //3 led, moi led 3 byte
        {
          uint8_t temp_buff[3] = {0x00, 0x00, 0x00};
          uint8_t mask = 0x80;                   // b1000000
          while (mask != 0)                      // check each of the 8 bits
          {
              while(!(UCA1IFG & UCTXIFG));       // wait to transmit
              if (temp_buff[j] & mask) {         // most significant bit first
                  UCA1TXBUF = HIGH_CODE;         // send 1
              } else {
                  UCA1TXBUF = LOW_CODE;          // send 0
              }
              mask >>= 1;                        // check next bit
          }
        }
      }
    }
  }
}

void setLEDColor(uint16_t p, uint8_t r, uint8_t g, uint8_t b) 
{
  leds[p].red   = r;
	leds[p].green = g;
  leds[p].blue  = b;
}

void showStrip() 
{
    __bic_SR_register(GIE);  // disable interrupts
    // send RGB color for every LED
    unsigned int i, j;
    for (i = 0; i < NUM_LEDS; i++) 
    {
        LED current_led;
        current_led =  convertDataToRGB(LED_DATA[i]);
        uint8_t *rgb = (uint8_t *)&current_led; // get GRB color for this LED
        // send green, then red, then blue
        for (j = 0; j < 3; j++) 
        {
            uint8_t mask = 0x80;    // b1000000
            // check each of the 8 bits
            while (mask != 0) 
            {
                while(!(UCA1IFG & UCTXIFG));// wait to transmit
                if (rgb[j] & mask) {         // most significant bit first
                    UCA1TXBUF = HIGH_CODE;   // send 1
                } else {
                    UCA1TXBUF = LOW_CODE;    // send 0
                }
                mask >>= 1;  // check next bit
            }
        }
    }
    // send RES code for at least 50 us (800 cycles at 16 MHz)
    __delay_cycles(800);
    __bis_SR_register(GIE);    // enable interrupts
}
//----------------------------------------------------------------------------
// Main program
//----------------------------------------------------------------------------
void main(void)
{
    clock_init();                           // Clock initialization
    gpio_init();                            // GPIO initialization
    Radio.Init();
    // Set RTC time
    if(reset_variable_first_time == 0)
    {
        reset_variable_first_time    = 1;   // Set co de xoa trang thai nap lan dau
        SENSOR_SAMPLE_TIME = 15;
        SAMPLE_TIME		= 30;    			  
        NUMBER_PULSE_PER_LIT = 10000;                            // 3676
        //NUMBER_PULSE_PER_LIT_SLOW = NUMBER_PULSE_PER_LIT + ((NUMBER_PULSE_PER_LIT*6470)/VALUE_PER_LIT);
        water_value_m3 	= 0; 				  			// Water volume(m3)
        water_value_lt 	= 0;				  			// Water volume(Lit)
        pin_percent    	= 100;				  		// Battery level(%)

        RTCYEAR = 2017;                     // Year
        RTCMON 	= 11;                       // Monthy
        RTCDAY 	= 23;                       // Day
        RTCDOW 	= 3;                        // Day of week = 0x01 = Monday
        RTCHOUR = 10;                       // Hour
        RTCMIN 	= 0;                        // Minute
        RTCSEC 	= 0;                        // Seconds
    }
    RTCYEAR = 2017;                      // Year
    RTCMON 	= 11;                        // Monthy
    RTCDAY 	= 23;                        // Day
    RTCDOW 	= 3;                         // Day of week = 0x01 = Monday
    RTCHOUR = 10;                        // Hour
    RTCMIN 	= 0;                         // Minute
    RTCSEC 	= 0;                         // Seconds
    ON_TIMER_1s();                          // Enable RTC second interrupt
    RTCCTL01 &=~(RTCHOLD);                  // Start RTC
    Radio.RxOn();                           // Receive mode active
    read_battery_counter = 3900;
    solar_charge_flag = true;   
    setLEDColor(0, 0x09, 0x0A, 0x0B);  // Green
    setLEDColor(1, 0x04, 0x05, 0x06);  // Blue
    setLEDColor(2, 0x07, 0x08, 0x09);  // Magenta
    setLEDColor(3, 0x0A, 0x0B, 0x0C);  // Yellow
    setLEDColor(4, 0x0D, 0x0E, 0x0F);  // Cyan
    setLEDColor(5, 0xFF, 0x55, 0xFF);  // Red
    
    updateTime();
    _BIS_SR(GIE);                 	        // global interrupt enablelobal Interrupt
    while(1)
    {
      showStrip();
    }
}
//#pragma vector = USCI_A0_VECTOR             // USCI ISR
//__interrupt void USCI_A0_ISR(void)
//{
//  _NOP();
//} 
//----------------------------------------------------------------------------
// Timer A0 interrupt service routine
//----------------------------------------------------------------------------
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
{
  WDTCTL = WDT_ARST_1000;
    
    PW_SENSOR_DRIVE_HIGH();                                                     // on sensor
    ADC10CTL0 |= ADC10ENC + ADC10SC;                                            // Sample and conversion start (enable and start conversion)
    while (!(ADC10IFG & ADC10IFG0));                                            // Conversion complete?
    if (ADC10MEM0 < 25)                                                         // ADC10MEM0 = A2 < 20 level low
      {
        status_sensor = 0;
        //SET_LED_TO_HIGH();
       
      }
    else
      {
        status_sensor = 1;
         //SET_LED_TO_LOW(); 
        //P1OUT ^= 0x01;
      }
    
    ADC10CTL0 &= ~ADC10ENC;                                                     // ADC10 disable consersion
    if((status_sensor != status_sensor_old) && (status_sensor == 0))            // co thay doi muc logic va xung o muc thap
    {
        number_pulse_counter ++;
        fulseCounter +=  NUMBER_PULSE_PER_LIT;
        fulseCounter_flow += NUMBER_PULSE_PER_LIT;
        fulseCounter_flow_h += NUMBER_PULSE_PER_LIT;
        //P3OUT ^= BIT5;
        if(fulseCounter >= VALUE_PER_LIT)
        {
            fulseCounter = fulseCounter - VALUE_PER_LIT;
            water_value_lt++;

            if(water_value_lt > MQTT_MAX_WATER_INDEX_LIT)
            {
                water_value_m3 ++;
                water_value_lt = 0;
                if(water_value_m3 > MQTT_MAX_WATER_INDEX_CUBIC)
                {
                    water_value_m3 = 0;
                    water_value_lt = 0;
                }
            }
        }
    }
    status_sensor_old = status_sensor;
    PW_SENSOR_DRIVE_LOW();               // off sensor
}
//----------------------------------------------------------------------------
// Timer1 A0 interrupt service routine
//----------------------------------------------------------------------------
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0_ISR(void)
{
   if(TA1CCR0 != 0)
    {
      if(send_data_flag)
      {
        P3OUT |= BIT5;
        if(++count_send_data_again < 10)
        {
          Radio.Wakeup();
          Radio.Idle();
          memset(TxArray,0x00,60);
          memcpy(&TxArray[0], ">>", 2);
          TxArray[2] = CC1101_MY_ADDRESS;
          TxArray[3] = 0x02; 
          if(swm_joined_flag)
            TxArray[3] = 0x01; 
          
          memcpy(&TxArray[4], SERIAL_NUMBER, SERIAL_NUMBER_LEN);
          TxArray[PAYLOAD_OFFSET + 0] = (uint8_t)((water_value_m3>>24)&0Xff);
          TxArray[PAYLOAD_OFFSET + 1] = (uint8_t)((water_value_m3>>16)&0Xff);
          TxArray[PAYLOAD_OFFSET + 2] = (uint8_t)((water_value_m3>>8)&0Xff);
          TxArray[PAYLOAD_OFFSET + 3] = (uint8_t)((water_value_m3)&0Xff);
          TxArray[PAYLOAD_OFFSET + 4] = (uint8_t)((water_value_lt>>8)&0Xff);
          TxArray[PAYLOAD_OFFSET + 5] = (uint8_t)((water_value_lt)&0Xff);
          TxArray[PAYLOAD_OFFSET + 6] = (uint8_t)pin_percent;
          TxArray[PAYLOAD_OFFSET + 7] = (uint8_t)RTCHOUR;
          TxArray[PAYLOAD_OFFSET + 8] = (uint8_t)RTCMIN;
          TxArray[PAYLOAD_OFFSET + 9] = (uint8_t)RTCDATE;
          TxArray[PAYLOAD_OFFSET + 10] = (uint8_t)RTCMON;
          TxArray[PAYLOAD_OFFSET + 11]= (uint8_t)(RTCYEAR - 2000);
          memcpy(&TxArray[PAYLOAD_OFFSET + 12], (uint8_t*)&water_value_m3, 4);
          memcpy(&TxArray[PAYLOAD_OFFSET + 16], (uint8_t*)&water_value_m3, 4);
          TxArray[PAYLOAD_OFFSET + 20]= '1';  // Charge status
          TxArray[PAYLOAD_OFFSET + 21]= '1';  // Adapter status
          TxArray[PAYLOAD_OFFSET + 22]= 15; // RSSI
          TxArray[PAYLOAD_OFFSET + 23]= '1';  // active status
          TxArray[PAYLOAD_OFFSET + 24]= 100;// PIN 2 status
          
          Radio.SendData(TxArray, RF_PAYLOAD_LEN);
        }
        else
        {
          send_data_flag = false;
					P3OUT &= ~BIT5;
					Radio.Sleep();
					TA1CCR0 = 0;
        }
      }
    }
}
//----------------------------------------------------------------------------------
// Timer1 B0 interrupt service routine
//----------------------------------------------------------------------------
#pragma vector = TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR(void)    								  // Timer A0 hanlde every 2.5mS
{
  WDTCTL = WDT_ARST_1000;                                 // Reset Watchdog timer
  
  // Blink LED indicated connection interrupted
  if(++led_blink_counter > 200)
  {
    led_blink_counter = 0;
    if(swm_joined_flag == false)
      P3OUT ^= BIT5;
  }
}
//----------------------------------------------------------------------------------
// Port 1 interrupt service routine
//----------------------------------------------------------------------------------
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
}
//----------------------------------------------------------------------------
// RTC interrupt service routine
//----------------------------------------------------------------------------
#pragma vector=RTC_VECTOR
__interrupt void RTC_A_ISR(void)
{
  switch(__even_in_range(RTCIV,16))
  {
    case 0: break;      // No interrupts
    case 2:             // RTCRDYIFG
      //Toggle P1.0 every second
      if((RTCSEC % 2) == 0) // Cap nhat luu luong sau moi 2 giay
      {
          calculator_flow = ((fulseCounter_flow * 30)/VALUE_PER_LIT);
          if(calculator_flow !=  water_flow)
          {
              water_flow    =   (uint16_t) calculator_flow; // Luu luong lit/phut
          }
          fulseCounter_flow = 0;
      }
      if((RTCSEC % 5) == 0) // Cap nhat luu luong sau moi 5 giay
      {
          calculator_flow = ((fulseCounter_flow_h * 720)/VALUE_PER_LIT);
          
          if(calculator_flow !=  water_flow_h)
          {
              water_flow_h = (uint16_t) calculator_flow;    // Luu luong lit/gio
          }
          fulseCounter_flow_h = 0;
      }
      if((RTCSEC % SENSOR_SAMPLE_TIME) == 0)
      {
          send_data_flag = true;
          count_send_data_again = 0;
          TA1CCR0 = CYCLE_SEND_DATA;                              // Timer Offset
          __bic_SR_register_on_exit(LPM3_bits); 
      }     
      break;
    case 4:             // RTCEVIFG Interrupts every min
      break;
    case 6:             // RTCAIFG Interrupts 5:00pm on 5th day of week
      break;
    case 8: break;      // RT0PSIFG
    case 10: break;     // RT1PSIFG
    case 12: break;     // Reserved
    case 14: break;     // Reserved
    case 16: break;     // Reserved
    default: break;
  }
}
//----------------------------------------------------------------------------
// Port 3 interrupt service routine
//----------------------------------------------------------------------------

#pragma vector=PORT3_VECTOR
__interrupt void Port_3(void)
{
    switch(__even_in_range(P3IV,16))
    {
      case 2:      //P3.0
        //receive data check CRC OK
        if(mrfiRadioState != RADIO_STATE_OFF)
        {
            sizerx=Radio.ReceiveData(RxArray); // put contents into RX buffer
            Radio.Idle();
            Radio.RxOn();
            if(sizerx > 0)
            {
                //if(Base64decode((char*)TxArray,(char*)RxArray) == 45)
                {
                    if((memcmp((char*)&RxArray[0], ">>",2) == 0) &&
                           (memcmp((char*)&RxArray[4], SERIAL_NUMBER,15) == 0))
                    {
                        switch((uint16_t)((RxArray[2]<<8)|RxArray[3]))
                        {
                          case 0x6401:
                            if(memcmp((char*)&RxArray[19], "OK",2) == 0)
                            {
                                send_data_flag = false;
                                count_send_data_again = 0;
                                P3OUT &=~ BIT5;
                                TA1CCR0 = 0;     
                                Radio.Sleep();
                            }
                            else if(memcmp((char*)&RxArray[19], "ST",2) == 0)
                            {
                              setting_flags = ((uint16_t)((RxArray[21]<<8)|RxArray[22]));
                              
                              if(setting_flags & 0x0004)         // Setting Water index
                              {
                                water_value_m3 = ((uint32_t)RxArray[23] << 24) | ((uint32_t)RxArray[24] << 16) | ((uint32_t)RxArray[25] << 8) | (uint32_t)RxArray[26];
                                water_value_lt = ((uint32_t)RxArray[27] << 8) | (uint32_t)RxArray[28];;
                              }
                              
                              if(setting_flags & 0x0020)         // Setting RTC
                              {
                                RTCCTL01 |= (RTCHOLD);           // Stop RTC for update time
                                RTCHOUR = RxArray[38];           // Hour
                                RTCMIN 	= RxArray[39];           // Minute
                                RTCDAY 	= RxArray[41];           // Day
                                RTCMON 	= RxArray[42];           // Month
                                RTCYEAR = (RxArray[43] + 2000);  // Year
                                RTCCTL01 &=~(RTCHOLD);           // Restart RTC
                              }
                              
                              
                              Radio.Idle();
                              memset(TxArray,0x00,60);
                              memcpy(&TxArray[0], ">>", 2);
                              TxArray[2] = CC1101_MY_ADDRESS;
                              TxArray[3] = 0x03; // Response apply setting success
                              memcpy(&TxArray[4], SERIAL_NUMBER, SERIAL_NUMBER_LEN);
                              memset(RxArray,0x00,60);
                              Radio.SendData( TxArray, RF_PAYLOAD_LEN);
                            }
                            break;
                          case 0x6402: 
                            send_data_flag = false;
                            count_send_data_again = 0;
                            P3OUT &=~ BIT5;
                            TA1CCR0 = 0;     
                            Radio.Sleep();
                            swm_joined_flag = true;
                            RTCCTL01 |= (RTCHOLD);           // Stop RTC for update time
                            RTCHOUR = RxArray[20];           // Hour
                            RTCMIN 	= RxArray[21];           // Minute
                            RTCSEC 	= RxArray[22];           // Seconds
                            RTCDAY 	= RxArray[23];           // Day
                            RTCMON 	= RxArray[24];           // Month
                            RTCYEAR = (RxArray[25] + 2000);  // Year
                            RTCCTL01 &=~(RTCHOLD);           // Restart RTC
                            
                    };
                    
                    }
                }
            }
        }
        break;
      case 4:
        break;
      case 6:
        break;
      case 8:
        break;
      case 10:
        break;
      case 12:
        break;
      case 14:
        break;
      default:
        break;
    }
    P3IFG &= ~BIT0;                           // P1.3 IFG cleared
}
//------------------------------------------------------------------------------
// End of file
//------------------------------------------------------------------------------

//P1OUT ^= 0x01;                                                            // Toggle P1.0 using exclusive-OR
