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
 *    Module       : UART.H
 *    Description  : This header file includes definitions and prototypes of
 *          API functions that support you to use UART Mode of USCI
 *
 *  Tool           : CCS 5.1
 *  Chip           : MSP430G2xx3 (MCU that haves USCI_A0 module)
 *  History        : 21-7-2012
 *  Version        : 1.1
 *
 *  Author         : Nguyen Tien Manh, CLB NCKH DDT (manhcly@ymail.com)
 *  Notes          :
 *      To apply these functions, you must include this header file and add
 *      the uart.c file to your project.
 *
 *      For hardware connecting, use P1.1 as RXD and P1.2 as TXD
 *
 *      You must put the jumpers so that P1.1 is connected to RXD and P1.2 is
 *      connected to TXD.

 *      You should consult the Launch Pad schematic in Launch Pad User's Guide
 *      for easier connecting.
 *    		Ex: Since Launch pad rev.1.5, Jumper Connection J3:
 *		[Half Debug Board]  ...|P1.1|...|RXD |...|RST|...|TEST|...|VCC|...
                         ---------------------------------------------------
 *		[Half MSP430 Board] ...|TXD |...|P1.2|...|RST|...|TEST|...|VCC|...
 *        (Note that this layout depends on the revision of launch pad)
 *
 *		You must define the frequency of SMCLK and BAUDRATE to ensure UART's
 *		working
 *		To find or edit the UART configuration, check the function uart_init()
 *		in uart.c
 *
******************************************************************************/

#ifndef UART_H_
#define UART_H_

 /****************************************************************************
 * USER DEFINITIONS
******************************************************************************/

#ifndef SMCLK_F
#define SMCLK_F 1000000 // frequency of Sub-System Master Clock in Hz
#endif
// This definition supports to UART delay functions. You should change it to the
// right MCLK frequency that you configure through "Config_Clocks" function.

#define	BAUDRATE  9600 // may be ... 1200, 2400, 4800, 9600, 19200, ...
// With Launch Pad, the back channel UART to the target MSP is done by
// bit-banging the otherwise unused I/O port lines of the TUSB interface chip
// (by the specific TUSB firmware), and this is limited to 9600
// So you should not try the Baud rate above 9600.

#define UART_RX_INT_EN 1
// If you enable the UART receive interrupt (define 1), you don't need to
// wait receive data by function uart_get & cuart_gets.
// Instead, you must enable GIE and write your processing code in USCI0RX_ISR
// (can be found in uart.c)

/****************************************************************************
 * HARD DEFINITIONS
******************************************************************************/


 /****************************************************************************
* FUNCTIONS 'S PROTOTYPES
******************************************************************************/
// For further description, see UART.c
void uart_init();
void uart_putc(char c);
void uart_puts(const char *s);
void uart_put_num (unsigned long val, char dec, unsigned char neg);
char uart_data_ready();
char uart_getc();
void uart_gets(char *s);
void uart_puthex(unsigned long number);
void uart_putdec(unsigned long n);
#endif /* UART_H_ */
/******************************************************************************
 * END OF standard UART.H
*******************************************************************************/
