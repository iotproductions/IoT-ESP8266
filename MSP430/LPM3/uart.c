/******************************************************************************
 *
 * www.payitforward.edu.vn
 *
 ******************************************************************************/

/******************************************************************************
 *
 * TI LAUNCH PAD CODING
 *
 ******************************************************************************/

/******************************************************************************
 *
 *    Module       : UART.C
 *    Description  : This file describes API functions that support you to
 *                   use UART Mode of USCI
 *
 *  Tool           : CCS 5.1
 *  Chip           : MSP430G2xx3 (MCU that haves USCI_A0 module)
 *  History        : 21-7-2012
 *  Version        : 1.1
 *
 *  Author         : Nguyen Tien Manh, CLB NCKH DDT (manhcly@ymail.com)
 *  Notes          :
 *      To apply these functions, you must include the header file uart.h and
 *      add this code file to your project.
 *
 *		To find or edit the UART configuration, check the function uart_init()
 *
******************************************************************************/

 /****************************************************************************
 * IMPORT
******************************************************************************/

#include <msp430.h>
#include "UART.h"

 /****************************************************************************
* FUNCTIONS
******************************************************************************/

 /****************************************************************************
* MODULE PUPLIC FUNCTIONS:
* void uart_init();
* void uart_putc(char c);
* void uart_puts(const char *s);
* void uart_put_num (unsigned long val, char dec, unsigned char neg);
* char uart_data_ready();
* char uart_getc();
* void uart_gets(char *s);
*
******************************************************************************/

//*****************************************************************************
// Initialization for UART module
//*****************************************************************************
void uart_init()
{	
  #if 1
    P1SEL |= BIT1 + BIT2;
    P1SEL2 |= BIT1 + BIT2;
    UCA0CTL1 |= UCSSEL_2;
    //Baud rate = 16Mhz/9600=1667(16bit);  ->131/6
    UCA0BR0 = 104;
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0;
    UCA0CTL1 &= ~UCSWRST;
    IE2 |= UCA0RXIE;
    __bis_SR_register(GIE);
  #endif
}
//**********************************************************************/

void conf_uart(void)
{
	unsigned int tempfactor;
    P1SEL |= BIT1 + BIT2 ;                     // Set P1.1 as RXD
    P1SEL2 |= BIT1 + BIT2 ;                    // Set P1.2 as TXD
	UCA0CTL0 |= UCMODE_0; // Select UART mode
    // UCA0CTL0 - USCI_A0 Control Register 0
     	// UCPEN = Parity enable
		// UCPAR = Parity select. 0: Odd parity
		// UCMSB = MSB first select. 0: LSB first
		// UC7BIT = Character length
		 	//0 : 8-bit data
			//1 : 7-bit data
		// UCSPB = Stop bit select
			//0 : One stop bit
			//1 : Two stop bits
		//UCMODEx USCI mode
			//00 : UARTmode
		// UCSYNC Synchronous mode enable
			// 0 : Asynchronous mode
			// 1 : Synchronous mode
    UCA0CTL1 |= UCSSEL_2;                     // Select clock source USCI
    // UCA0CTL1 - USCI_A0 Control Register 1
		// UCSSELx = USCI clock source select
			//00 : UCLKI
			//01 : ACLK
			//10 : SMCLK
			//11 : SMCLK
		// UCSWRST = Software reset enable
			// 0 : Disabled. USCI reset released for operation
			// 1 : Enabled. USCI logic held in reset state
    tempfactor = SMCLK_F/BAUDRATE;
    UCA0BR0 = (unsigned char) tempfactor&0x00FF;
    tempfactor >>= 8;
    UCA0BR1 = (unsigned char) (tempfactor&0x00FF);
    // UCA0BR0 - USCI_A0 BaudRate Control Register 0
    // UCA0BR0 - USCI_A0 BaudRate Control Register 1
		// The 16-bit value of (UCA0BR0+UCA0BR1×256) forms the prescaler value
        // of the Baud rate generator

    UCA0MCTL |= UCBRF_0 + UCBRS_0;                        // Configure modulator
    // UCA0MCTL - USCI_A0 Modulation Control Register
	// UCBRFx = First modulation stage select
		//UCBRF_0 equal 0x0000
	// UCBRSx = Second modulation stage select
		//UCBRS_0 equal 0x00
    // UCOS16 = Oversampling mode enabled
    UCA0CTL1 &= ~UCSWRST;                     // **Release USCI to operate**
    // UCA0CTL1 - USCI_A0 Control Register 1
		// UCSWRST    = Software reset enable
			// 0 : Disabled. USCI reset released for operation
			// 1 : Enabled. USCI logic held in reset state
    if (UART_RX_INT_EN)
    	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

    // IE2 - Interrupt Enable Register 2
		// UCA0TXIE   = USCI_A0 transmit interrupt enable
		// UCA0RXIE   = USCI_A0 receive interrupt enable
    // IFG2 - Interrupt Flag Register 2
		// UCA0TXIFG USCI_A0 transmit interrupt flag.
    		// UCA0TXIFG is set when UCA0TXBUF is empty
		// UCA0RXIFG USCI_A0 receive interrupt flag.
    		// UCA0RXIFG is set when UCA0RXBUF has received a complete character.
}

//*****************************************************************************
// Send a  character through UART
//*****************************************************************************
void uart_putc(char c)
{
	while(!(IFG2&UCA0TXIFG));   // Wait until USCI_A0 TX buffer empty
		UCA0TXBUF = c; 			// assign character to TX buffer
}

//*****************************************************************************
// Send a  string through UART
// Note that the string is ended with '/0' (or 0x00)
//*****************************************************************************
void uart_puts(const char* s)
{
	while(*s != '\0'){
		uart_putc(*s);
		s++;
	}
}

//*****************************************************************************
// Send a constant or a number stored in 'val' through UART
// The number must be multiplied and convert in type "unsigned long"
//                                                or "unsigned int"
//   dec: amount of digits after '.'
//   neg: sign of the number, neg = 0 if the number is positive
//
// Example 1: uart_put_num(val, 3, 0); // val = 12345
//        will put "12.345"
// Example 2: uart_put_num(val, 6, 1); // val = 12345
//        will put "-0.012345"
//
// Note: This function may contain some bugs. If you can detect and fix them
//       , please contact me :d
//*****************************************************************************
void uart_put_num (unsigned long val, char dec, unsigned char neg)
{
    char i, j, digit,k;
    long total;
    long temp;
    for (i = 0, total = val; total > 0;i++) // count number
        total /= 10;
        total = i;

    if (neg !=0 )
      uart_putc ('-');

    if ((total - dec) <=0)	// if total < dec put 0.xxx
        uart_putc('0');
    else
    {

      for (i=0; i< (total-dec); i++)
      {
        temp = 1;
        for (j=1;j<(total-i);j++)
            temp *=10;
        digit = (val/temp)%10;
        uart_putc(digit + 0x30);
      }
    }
    if (dec > 0)
    {
      uart_putc('.');
      if ( (dec - total)> 0)	// ex: val = 55; dec = 3 put 0.055
      {
        for ( i = 0; i < (dec-total);i++)
          uart_putc('0');
        k = 0;
      }
      else
        k = total - dec;
      for (i=k; i< total; i++)
      {
       temp = 1;
       for (j=1;j<(total-i);j++)
           temp *=10;
       digit = (val/temp)%10;
       uart_putc(digit + 0x30);
      }
    }
}


//*****************************************************************************
// Wait to receive a character through UART
//*****************************************************************************
char uart_getc()
{
	while(!(IFG2&UCA0RXIFG));   // Wait until USCI_A0 RX receive a complete character
		return UCA0RXBUF; // assign RX buffer to function 's name
}

//*****************************************************************************
// Wait to receive a string through UART
// Note that the string is ended with '\0' (or 0x00)
//*****************************************************************************
void uart_gets(char *s)
{
	*s = uart_getc();
	while(*s!='\0')
	{
		s++;
		*s = uart_getc();
	}
}

//*****************************************************************************
// Check if USCI_A0 RX Buffer contain a complete character
//*****************************************************************************
char uart_data_ready()
{
	if(UCA0RXIFG) return 1;
	else return 0;
}
//--------------------------UART_OutUHex----------------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void uart_puthex(unsigned long number){
// This function uses recursion to convert the number of
//   unspecified length as an ASCII string
  if(number >= 0x10){
    uart_puthex(number/0x10);
    uart_puthex(number%0x10);
  }
  else{
    if(number < 0xA){
      uart_putc(number+'0');
     }
    else{
      uart_putc((number-0x0A)+'A');
    }
  }
}
//-----------------------UART_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void uart_putdec(unsigned long n)
{
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    uart_putdec(n/10);
    n = n%10;
  }
  uart_putc(n+'0'); /* n is between 0 and 9 */
}
/****************************************************************************
* USCI_A0 RECEIVE INTERRUPT SERVICE ROUTINE
******************************************************************************/
/*
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
   //uart_putc(UCA0RXBUF); // echo the received character
}
  */
/******************************************************************************
 * END OF UART.C
*******************************************************************************/
