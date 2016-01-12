//------------------------------------------------------------------------------
#include <msp430.h>
#include <stdint.h>
#include "algorithm.h"
#include "uart.h"
//------------------------------------------------------------------------------
// Private  variables
//------------------------------------------------------------------------------
unsigned char Count = 0x00, First_Time = 0x01;
uint32_t FEdge,FEdge2;
uint8_t  TxArray[20];
uint8_t  RxArray[20];
//------------------------------------------------------------------------------
// Functions protocol
//------------------------------------------------------------------------------
void clock_init(void);
void gpio_init(void);
void timer_init(void);
void write_flash_Seg(uint8_t num_bytes, uint8_t *DataByte, int segment_pointer);
void read_flash_Seg(uint8_t num_bytes, uint8_t *DataByte, int segment_pointer);
void prepareToWrite_Seg (int pointer);
#define mFlashWriteEnable {while (FCTL3 & BUSY);FCTL1 = FWKEY;FCTL1 = FWKEY + WRT;}
// Set WRT bit for write operation
#define mFlashWriteDisable while (FCTL3 & BUSY);FCTL1 = FWKEY +LOCK
// disable write operation
#define	P_SEGMENT_C		0x1040	
#define	P_SEGMENT_D		0x1000	

uint8_t number2string(uint8_t *str_buff, uint32_t num);
//------------------------------------------------------------------------------
// Main function
//------------------------------------------------------------------------------
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;        // Stop watchdog timer
	clock_init();
	gpio_init();
	timer_init();
	__bis_SR_register(GIE);         // enable global interrupt
//	water_value_m3 = 770;
//	water_value_lt = 78;
//	TxArray[0] = (water_value_m3>>24)&0xff;
//	TxArray[1] = (water_value_m3>>16)&0xff;
//	TxArray[2] = (water_value_m3>>8)&0xff;
//	TxArray[3] = (water_value_m3&0xff);
//	TxArray[4] = (water_value_lt>>8)&0xff;  //Tach bien dem chi so nuoc: lit
//	TxArray[5] = (water_value_lt&0xff);
//	write_flash_Seg(6, TxArray, P_SEGMENT_C);
	read_flash_Seg(6, RxArray, P_SEGMENT_C);
	water_value_m3 = ((uint32_t)(RxArray[0])<<24)|((uint32_t)(RxArray[1])<<16)|((uint32_t)(RxArray[2])<<8)|(RxArray[3]&0xFF);
	water_value_lt = ((uint16_t)(RxArray[4])<<8)|((RxArray[5])&0xFF);
    __no_operation();
	while(1)
	{
		__bis_SR_register(LPM3_bits + GIE);   // Enter LPM0
		__no_operation();                     // For debugger
		// On exiting LPM0
	}
}
//------------------------------------------------------------------------------
// TA1_A1 Interrupt vector
#pragma vector = TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR (void)
{
	switch(__even_in_range(TA1IV,0x0A))
	{
	  case TA1IV_NONE: break;           // Vector  0:  No interrupt
	  case TA1IV_TACCR1:                // Vector  2:  TACCR1 CCIFG
		
		// FEdge2 = TA1R;
		//T_cycle = TA1CCR1;              // Assign value T_cycle
		// uart_put_num(T_cycle, 0, 0);
		// uart_puts("\r\n");
		// TA1R  = 0;                      // Reset thoi gian
		//if(TA1CTL & TAIFG)              // Check for Capture Overflow
		//{
		//	__no_operation();
		//	TA1CTL &= ~TAIFG;
		//}
		//else // ignore edge first
		{
			// water_counter_algorithm();
			
			fulseCounter++;
			
			// comment_test
			if(fulseCounter >= NUMBER_PULSE_PER_LIT)
			{
				water_value_lt++;
				P1OUT |= BIT0;
				_NOP();
				uart_puts(" ");
				_NOP();
				P1OUT &= ~BIT0;
				__delay_cycles(1000000);
				uart_puts("AT+SLEEP");
				P1OUT |= BIT0;
				
				if(water_value_lt>MQTT_MAX_WATER_INDEX_LIT)
				{
					water_value_m3 ++;
					water_value_lt = 0;
					if(water_value_m3 > MQTT_MAX_WATER_INDEX_CUBIC)
					{
						water_value_m3 = 0;
						water_value_lt = 0;
						
						TxArray[0] = (water_value_m3>>24)&0xff;
						TxArray[1] = (water_value_m3>>16)&0xff;
						TxArray[2] = (water_value_m3>>8)&0xff;
						TxArray[3] = (water_value_m3&0xff);
						TxArray[4] = (water_value_lt>>8)&0xff;  //Tach bien dem chi so nuoc: lit
						TxArray[5] = (water_value_lt&0xff);
						//write_SegC(6, TxArray);
					}
				}
				if((water_value_lt % 100) == 0)
				{
					TxArray[0] = (water_value_m3>>24)&0xff;
					TxArray[1] = (water_value_m3>>16)&0xff;
					TxArray[2] = (water_value_m3>>8)&0xff;
					TxArray[3] = (water_value_m3&0xff);
					TxArray[4] = (water_value_lt>>8)&0xff;  //Tach bien dem chi so nuoc: lit
					TxArray[5] = (water_value_lt&0xff);
					//write_SegC(6, TxArray);
				}
				
				
			}
		}
		//P1OUT ^= BIT6;
		
		//            if(FEdge != FEdge2)
		//              __no_operation();
		
		break;
	  case TA1IV_TACCR2: break;             // Vector  4:  TACCR2 CCIFG
	  case TA1IV_TAIFG:
		__no_operation();
		break;              // Vector 10:  TAIFG
	  default: 	
		break;
	}
}
//------------------------------------------------------------------------------
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
}
//------------------------------------------------------------------------------
void clock_init(void)
{
	//BCSCTL3
	if (CALBC1_16MHZ==0xFF)		    // If calibration constant erased
	{											
		while(1);                               // do not load, trap CPU!!	
	}
	DCOCTL = 0;                               // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_16MHZ;                    // Set DCO to 8MHz
	DCOCTL = CALDCO_16MHZ;
}
//------------------------------------------------------------------------------
void gpio_init(void)
{
	P2DIR &= ~BIT1;                           // P2.1/TA1.1 Input Capture
	P2SEL |= BIT1;                            // TA1.1 option select
	//P2REN |= BIT1;                          // Pull_up
	//P2OUT |= BIT1;                          // Pull_up
	P1DIR |= BIT6;
	P1OUT &= ~BIT6;
	

	
    P1SEL |= BIT1 + BIT2;
    P1SEL2 |= BIT1 + BIT2;
	UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
	UCA0BR0 = 0x03;                           // 32kHz/9600 = 3.41
	UCA0BR1 = 0x00;                           //
	UCA0MCTL = UCBRS1 + UCBRS0;               // Modulation UCBRSx = 3
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
	P1DIR |= BIT0;
	
	uart_puts("UART Ready");
	
	P1OUT &= ~BIT0;
	__delay_cycles(1000000);
	uart_puts("AT+SLEEP");
	P1OUT |= BIT0;
}
//------------------------------------------------------------------------------
void timer_init(void)
{
	// Configure the TA0CCR1 to do input capture
	TA1CCTL1 = CAP + CM_2 + CCIE + SCS + CCIS_0; // TA0CCR1 Capture mode; CCI1A; Falling Edge; interrupt enable
	TA1CTL |= TASSEL_1 + ID_3 + MC_2 + TACLR;    // ACLK, Cont Mode; start timer
}
//------------------------------------------------------------------------------
void write_flash_Seg(uint8_t num_bytes, uint8_t *DataByte, int segment_pointer)
{
	char *Flash_ptr;                          // Flash pointer
	unsigned int i;
	prepareToWrite_Seg (segment_pointer);
	Flash_ptr = (char *) segment_pointer;              // Initialize Flash pointer	
	for (i=0; i < num_bytes; i++)
	{
		mFlashWriteEnable;
		*Flash_ptr++ = DataByte[i];                   // Write value to flash
		mFlashWriteDisable;
	}
}
//------------------------------------------------------------------------------
void read_flash_Seg(uint8_t num_bytes, uint8_t *DataByte, int segment_pointer)
{
	char *Flash_ptr;                          // Flash pointer
	unsigned int i;
	Flash_ptr = (char *) segment_pointer;                // Initialize flash pointer
	for (i=0; i < num_bytes; i++)
	{
		DataByte[i] = *Flash_ptr;                   // Write value to flash
		Flash_ptr++;
	}
}
//------------------------------------------------------------------------------
void prepareToWrite_Seg (int pointer)
{
	volatile char *Flash_ptr; // Flash pointer
	FCTL2 = FWKEY + FSSEL0 + FN3+ FN4; // MCLK/16 for Flash Timing Generator
	Flash_ptr = (char *) pointer; // Initialize Flash pointer
	FCTL1 = FWKEY + ERASE; // Set Erase bit
	FCTL3 = FWKEY; // Clear Lock bit
	*Flash_ptr = 0; // Dummy write to erase Flash segment
}
//------------------------------------------------------------------------------
uint8_t number2string(uint8_t *str_buff, uint32_t num)
{
    uint8_t i, rem, len = 0;
	uint32_t n;
    n = num;

    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str_buff[len - (i + 1)] = rem + '0';
    }
    str_buff[len] = '\0';
    return len;
}
